CCX = g++

TARGET = opencascade_demo

CFLAGS = \
	-I/usr/local/include/opencascade

LFLAGS = \
	-std=c++11 \
	-L/usr/local/lib \
	-lTKBO \
	-lTKBRep \
	-lTKGeomBase \
	-lTKGeomAlgo \
	-lTKMath \
	-lTKOffset \
	-lTKPrim \
	-lTKService \
	-lTKTopAlgo \
	-lTKSTL \
	-lTKHLR \
	-lTKernel \
	-lTKG2d \
	-lTKG3d \
	-lTKBRep \
	-lTKShHealing

SOURCES = \
	opencascade_demo.cpp

all:
	$(CXX) $(SOURCES) -o $(TARGET) $(CFLAGS) $(LFLAGS)

clean:
	rm -f $(TARGET)
