#include <cstdio>
#include <cstdlib>

#include <string>

#include "obj.h"

using namespace std;

void usage() {
    printf("Usage: ./msd [ <file.OBJ> ]\n");
    exit(1);
}

int main(int argc, char* argv[]) {
    char* objfname = NULL;
    switch (argc) {
        case 1: objfname = (char*) "cube.obj"; break;
        case 2: objfname = argv[1]; break;
        default: usage();
    }

    GLMmodel* object = glmReadOBJ(objfname);

    return 0;
}
