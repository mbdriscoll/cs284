GLFW_CXXFLAGS=-I/Users/mbdriscoll/opt/glfw-2.7.6/include -I/usr/X11/include
GLFW_LDFLAGS=-L/Users/mbdriscoll/opt/glfw-2.7.6/lib -lglfw -lXrandr -lX11 -lGLU -lGL -L/usr/X11/lib

CXXFLAGS=-I/opt/local/include -I/Users/mbdriscoll/opt/boost-1.47.0/include -g -ggdb3 $(GLFW_CXXFLAGS)
CFLAGS=-I/opt/local/include
LDFLAGS=-L/opt/local/lib -lgl -lglu -lglut $(GLFW_LDFLAGS)

default: msd

msd: glm.o util.o Object.o

.PHONY: clean

clean:
	rm -rf *.o msd
