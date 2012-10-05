CXXFLAGS=-I/opt/local/include
CFLAGS=-I/opt/local/include
LDFLAGS=-L/opt/local/lib -lgl

default: msd

msd: obj.c

.PHONY: clean

clean:
	rm -rf *.o msd
