g++ opencascad_demo.cpp -o opercascad_demo -I/usr/local/include/opencascade -L/usr/local/lib -std=c++11 -lTKBO -lTKBRep -lTKGeomBase -lTKGeomAlgo -lTKMath -lTKOffset -lTKPrim -lTKService -lTKTopAlgo -lTKSTL -lTKHLR -lTKernel -lTKG2d -lTKG3d -lTKBRep -lTKShHealing
#./opercascad_demo offset ./tests/input_001.txt ./tests/output_simple_offset.txt 0.2 arc
#./opercascad_demo booleans  ./tests/input_001.txt ./tests/output_hard.txt ./tests/booleans_output_001.txt difference_reversed
#./opercascad_demo booleans  ./tests/polygon_without_2_holes.txt ./tests/input_circle_offset.txt ./tests/booleans_output_001.txt union

#./opercascad_demo booleans  ./tests/input_001.txt ./tests/input_001_small_off_10.txt ./tests/booleans_output_001.txt difference

#./opercascad_demo offset ./tests/polygon_with_2_holes.txt ./tests/polygon_with_2_holes_offset.txt 0.2 arc
#./opercascad_demo offset ./tests/input_circle_3_parts.txt ./tests/input_circle_3_parts_offset.txt 0.2 arc

./opercascad_demo offset ./tests/input_hard_offset.txt ./tests/output_hard_offset.txt -0.2 arc



#python3 vis.py ./tests/polygon_without_2_holes.txt ./tests/input_circle_offset.txt ./tests/booleans_output_001.txt &
python3 vis.py ./tests/input_hard_offset.txt  ./tests/output_hard_offset.txt &

