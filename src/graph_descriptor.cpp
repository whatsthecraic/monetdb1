/*
 * graph_descriptor.cpp
 *
 *  Created on: 3 Feb 2017
 *      Author: Dean De Leo
 */

#include <graph_descriptor.hpp>
#include <utility>

using namespace gr8;
using namespace std;

/******************************************************************************
 *                                                                            *
 *   Graph descriptor                                                         *
 *                                                                            *
 ******************************************************************************/
GraphDescriptor::GraphDescriptor() { }
GraphDescriptor::~GraphDescriptor() { }


/******************************************************************************
 *                                                                            *
 *   Graph descriptor columns                                                 *
 *                                                                            *
 ******************************************************************************/

GraphDescriptorColumns::GraphDescriptorColumns(BatHandle&& edge_src, BatHandle&& edge_dst) :
		edge_src(move(edge_src)), edge_dst(move(edge_dst)) { }

GraphDescriptorColumns::~GraphDescriptorColumns() { }

GraphDescriptorType GraphDescriptorColumns::get_type() const {
	return e_graph_columns;
}


/******************************************************************************
 *                                                                            *
 *   Graph descriptor compact                                                 *
 *                                                                            *
 ******************************************************************************/

GraphDescriptorCompact::GraphDescriptorCompact(BatHandle&& edge_src, BatHandle&& edge_dst, std::size_t vertex_count) :
		edge_src(move(edge_src)), edge_dst(move(edge_dst)), vertex_count(vertex_count) { }

GraphDescriptorCompact::~GraphDescriptorCompact() { }

GraphDescriptorType GraphDescriptorCompact::get_type() const {
	return e_graph_compact;
}
