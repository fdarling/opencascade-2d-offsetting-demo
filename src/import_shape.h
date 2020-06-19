#pragma once

#include <string>
#include <vector>

#include <BRepBuilderAPI_MakeWire.hxx>
// #include <TopoDS_Shape.hxx>

typedef std::vector<std::string> string_vector; // TODO move this

int get_num_borders(const std::vector<string_vector> & segments_arcs);
bool collect_segments_arcs_to_wires(std::vector<BRepBuilderAPI_MakeWire> & borders, const std::vector< std::vector<std::string> > & lines);
bool load_face_from(const char * path, TopoDS_Shape & res);
bool read_input_file(const char * path, std::vector<string_vector> & res);
