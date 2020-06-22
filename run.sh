#!/bin/sh
#CPP_ARGS="offset ./tests/input_001.txt ./tests/output_simple_offset.txt 0.2 arc"
#CPP_ARGS="booleans ./tests/input_001.txt ./tests/output_hard.txt ./tests/booleans_output_001.txt difference_reversed"
#CPP_ARGS="booleans ./tests/polygon_without_2_holes.txt ./tests/input_circle_offset.txt ./tests/booleans_output_001.txt union"
#CPP_ARGS="booleans ./tests/input_001.txt ./tests/input_001_small_off_10.txt ./tests/booleans_output_001.txt difference"
#CPP_ARGS="offset ./tests/polygon_with_2_holes.txt ./tests/polygon_with_2_holes_offset.txt -0.2 arc"
#CPP_ARGS="offset ./tests/polygon_with_2_holes.txt ./tests/polygon_with_2_holes_offset.txt  0.2 arc"
#CPP_ARGS="offset ./tests/input_circle_3_parts.txt ./tests/input_circle_3_parts_offset.txt 0.2 arc"
#CPP_ARGS="offset ./tests/input_hard_offset.txt ./tests/output_hard_offset.txt -0.2 arc"
#CPP_ARGS="offset ./tests/input_hard_offset.txt ./tests/output_hard_offset.txt  0.2 arc"
#CPP_ARGS="booleans ./tests/input_2_shapes.txt ./tests/output_hard.txt ./tests/forest_output.txt union"
#CPP_ARGS="offset ./tests/input_2_shapes.txt ./tests/forest_output.txt -0.5 arc"
#CPP_ARGS="offset ./tests/input_2_shapes.txt ./tests/forest_output.txt -0.43 arc"
#CPP_ARGS="offset ./tests/input_2_shapes.txt ./tests/forest_output.txt -0.15 arc"
#CPP_ARGS="offset ./tests/input_2_shapes.txt ./tests/forest_output.txt  0.15 arc"
CPP_ARGS="offset ./tests/input_2_shapes.txt ./tests/forest_output.txt  0.43 arc"
#CPP_ARGS="offset ./tests/input_2_shapes.txt ./tests/forest_output.txt  0.45 arc" # NOTE: this one produces null output for some reason!

VIS_ARGS=`echo ${CPP_ARGS} | awk '/\.txt/' RS=" " ORS=" "`

make && \
echo ./opencascade_demo $CPP_ARGS && \
./opencascade_demo $CPP_ARGS && \
echo python3 vis.py $VIS_ARGS && \
python3 vis.py $VIS_ARGS
