/*
 * prepare_graph.cpp
 *
 *  Created on: 3 Feb 2017
 *      Author: Dean De Leo
 */

#include "prepare.hpp"

#include <algorithm>
#include <cassert>

namespace gr8 {

/******************************************************************************
 *                                                                            *
 *   reorder_computations                                                     *
 *                                                                            *
 ******************************************************************************/
void reorder_computations(Query& q){
	// a lower value means a higher priority
	auto ordering_cost = [](const ShortestPath& sp){
		size_t cost = 0;

		// BFS should come first
		if(!sp.bfs()){ cost += 100; }

		// Path computation second
		if(sp.compute_path()){ cost += 10; }

		// Third, discriminate based on the weight type
		if(!sp.bfs()){
	        switch(sp.weights.get()->ttype){
	        case TYPE_void:
	            assert(0); // I thought we were done with voids
	            break;
	        case TYPE_bte:
	        	cost += 1;
	        	break;
	        case TYPE_sht:
	        	cost += 1;
	        	break;
	        case TYPE_int:
	        	cost += 3;
	        	break;
	        case TYPE_lng:
	        case TYPE_oid: // <- it should not occur
	        	cost += 4;
	            break;
	        case TYPE_flt:
	        	cost += 5;
	        	break;
	        case TYPE_dbl:
	        	cost += 6;
	        	break;
	        default: // whatever you are
	        	cost += 7;
	        	break;
	        }
		}

		return cost;
	};

	auto comparator = [&](const ShortestPath& sp1, const ShortestPath& sp2){
		auto c1 = ordering_cost(sp1);
		auto c2 = ordering_cost(sp2);
		return c1 < c2;
	};

	sort(begin(q.shortest_paths), end(q.shortest_paths), comparator);
}

/******************************************************************************
 *                                                                            *
 *   prepare_graph                                                            *
 *                                                                            *
 ******************************************************************************/

extern "C" {
// prototypes, core functions of MonetDB
str ALGsort12(bat *result, bat *norder, const bat *bid, const bit *reverse, const bit *stable);
str ALGprojection(bat *result, const bat *lid, const bat *rid);
// preprocess.c
str GRAPHprefixsum(bat* id_output, const bat* id_input, const lng* ptr_domain_cardinality);
} // extern "C"

// side effect: we need to reorder also the weights in q.shortest_paths
static GraphDescriptorCompact* to_compact_sequential(Query& q, GraphDescriptorColumns* graph){
	if(graph->edge_src.empty()){ // edge case
		return new GraphDescriptorCompact(BatHandle{}, BatHandle{}, 0);
	}

	str rc = MAL_SUCCEED;
	bat	input = -1;
	bat output = -1;
	bat perm = -1;
	bit reverse = false;
	bit stable = false;

	BatHandle edge_src, edge_dst;

	// sort the inputs
	input = graph->edge_src.id();
	rc = ALGsort12(&output, &perm, &input, &reverse, &stable);
	MAL_ASSERT_RC();
	edge_src = BatHandle(&output);

	input = graph->edge_dst.id();
	rc = ALGprojection(&output, &perm, &input);
	MAL_ASSERT_RC();
	edge_dst = BatHandle(&output);

	// find the max value
	oid* dst = edge_dst.array<oid>();
	auto max_value = *(std::max_element(dst, dst + edge_dst.size()));
	max_value = std::max(max_value, edge_src.last<oid>());

	// prefix sum on the src
	lng count = (lng) max_value +1;
	input = edge_src.id();
	rc = GRAPHprefixsum(&output, &input, &count);
	MAL_ASSERT_RC();
	edge_src = BatHandle(&output);

	// finally permute the shortest paths weights
	for(auto& sp : q.shortest_paths){
		if(!sp.bfs()) {
			input = sp.weights.id();
			rc = ALGprojection(&output, &perm, &input);
			MAL_ASSERT_RC();
			sp.weights = BatHandle(&output);
		}
	}

	// done
	return new GraphDescriptorCompact(std::move(edge_src), std::move(edge_dst), (size_t) count);
}

void prepare_graph(Query& q){
	assert(q.graph && "Graph descriptor not initialised");

	switch(q.graph->get_type()){
	case e_graph_columns: {
		q.graph.reset( to_compact_sequential(q, (GraphDescriptorColumns*) q.graph.get()) );
	} break;
	case e_graph_compact:
		/* nop */
		break;
	default:
		RAISE_ERROR("Invalid graph type: " << q.graph->get_type());
	}
}


} /* namespace gr8 */