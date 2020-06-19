#pragma once

#include <string>
#include <vector>

#include <BRepBuilderAPI_MakeWire.hxx>

typedef std::vector<std::string> string_vector; // TODO move this

int get_num_borders(const std::vector<string_vector> & segments_arcs);
bool collect_segments_arcs_to_wires(std::vector<BRepBuilderAPI_MakeWire> & borders, const std::vector< std::vector<std::string> > & lines);
