CXXFLAGS=-I/opt/local/include
CFLAGS=-I/opt/local/include
LDFLAGS=-L/opt/local/lib -lgl -lglu -lglut

default: msd

msd: obj.c util.cpp

.PHONY: clean

clean:
	rm -rf *.o msd
