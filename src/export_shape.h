#pragma once

#include <cstdio>

#include <TopoDS_Shape.hxx>

void append_wires_to_file(const TopoDS_Shape & res, FILE * output_file);
