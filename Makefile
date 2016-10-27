# To run the project, enter "./seg_image.bin"
# To compile the project, enter "make"
# Libraries to install: GL, glut, GLEW, GLFW(3), SOIL, OPENCV3 (install the -dev version too)
# sudo apt-get install libglew1.13 libglew-13 freeglut3 freeglut3-dev libglfw3 libglfw3-dev libsoil1 libsoil-dev libgl1-mesa-dev libglu1-mesa-dev

GLFLAGS=-lglut -lGL -lGLU -lGLEW -lglfw -lSOIL -lm
CVLIB=`pkg-config --libs opencv` -fopenmp
CVFLAG=`pkg-config --cflags opencv` -fopenmp
EXEC=seg_image
INCLUDE=-std=c++11 -Wall -Wextra -g

# The .cpp common/shader.cpp
# Files in common are found in the Visual Studio OpenGL tutorials
SOURCES=common/shader.cpp functions/function.cpp render_to_texture/seg_image.cpp
# The .h
HEADERS=$(wildcard *.h)
# The .o
OBJECTS=$(SOURCES:%.cpp=%.o)

all: $(EXEC)

# Linking
$(EXEC): $(OBJECTS) $(SOURCES)
	$(CXX) $(OBJECTS) -o $@ $(GLFLAGS) $(CVLIB)

# Building
%.o: %.cpp
	$(CXX) -c $^ -o $@ $(GLFLAGS) $(CVFLAG) $(INCLUDE)

# Remove the .o and .bin
# To use this command, enter "make clean"
clean:
	$(RM) $(OBJECTS)

mrproper: clean
	$(RM) $(EXEC)

.PHONY: all clean mrproper
