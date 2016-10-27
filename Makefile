GLFLAGS=-lglut -lGL -lGLU -lGLEW -lglfw -lSOIL -lm
CVLIB=`pkg-config --libs opencv` -fopenmp
CVFLAG=`pkg-config --cflags opencv` -fopenmp
EXEC=seg_image
INCLUDE=-std=c++11 -Wall -Wextra -g
SOURCES=common/shader.cpp functions/function.cpp render_to_texture/seg_image.cpp
HEADERS=$(wildcard *.h)
OBJECTS=$(SOURCES:%.cpp=%.o)

all: $(EXEC)

$(EXEC): $(OBJECTS) $(SOURCES)
	$(CXX) $(OBJECTS) -o $@ $(GLFLAGS) $(CVLIB)

%.o: %.cpp
	$(CXX) -c $^ -o $@ $(GLFLAGS) $(CVFLAG) $(INCLUDE)

clean:
	$(RM) $(OBJECTS)

mrproper: clean
	$(RM) $(EXEC)

.PHONY: all clean mrproper
