#pragma once

#include <string>
#include <vector>

#include <BRepBuilderAPI_MakeWire.hxx>
// #include <TopoDS_Face.hxx>

typedef std::vector<std::string> string_vector; // TODO move this

typedef std::vector<TopoDS_Wire> wire_vector;
typedef std::pair<TopoDS_Wire, wire_vector> outline_with_holes;
typedef std::vector<outline_with_holes> outline_with_holes_vector;

bool read_wires_from_file_lines(outline_with_holes_vector & outlines_with_holes, const std::vector< std::vector<std::string> > & lines);
bool load_face_from_file(TopoDS_Face & res, const char * path);
bool read_lines_from_file(std::vector<string_vector> & res, const char * path);
