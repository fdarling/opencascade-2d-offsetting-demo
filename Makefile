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
	src/collect_segments.cpp \
	src/main.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(SOURCES) -o $(TARGET) $(CFLAGS) $(LFLAGS)

all: $(TARGET)

clean:
	rm -f $(TARGET)
