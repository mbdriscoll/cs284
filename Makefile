CXXFLAGS=-I/opt/local/include -I/Users/mbdriscoll/opt/boost-1.47.0/include
CFLAGS=-I/opt/local/include
LDFLAGS=-L/opt/local/lib -lgl -lglu -lglut

default: msd

msd: glm.o util.o Object.o

.PHONY: clean

clean:
	rm -rf *.o msd
