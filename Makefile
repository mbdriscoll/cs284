SOIL_CXXFLAGS=-I/Users/mbdriscoll/opt/soil-2008/include -framework CoreFoundation
SOIL_LDFLAGS=-L/Users/mbdriscoll/opt/soil-2008/lib -lSOIL

CXXFLAGS=-I/opt/local/include -I/Users/mbdriscoll/opt/boost-1.47.0/include -g -ggdb3 $(SOIL_CXXFLAGS)
CFLAGS=-I/opt/local/include
LDFLAGS=-L/opt/local/lib -lgl -lglu -lglut $(SOIL_LDFLAGS)

default: msd

msd: glm.o util.o Object.o

.PHONY: clean

clean:
	rm -rf *.o msd
