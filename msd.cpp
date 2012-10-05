#include <cstdio>
#include <cstdlib>

#include <string>

#include "obj.h"

#include "glm/glm.hpp"

using namespace std;
using namespace glm;

typedef struct _face {
    int v[3]; // vertices
    struct _face* n[3]; // neighbors
} face;

typedef struct _Object {
    GLuint numvertices, numfaces;
    GLfloat* vertices;
    face* faces;

    ~_Object() {
        free(this->vertices);
        free(this->faces);
    }
} Object;

void usage() {
    printf("Usage: ./msd [ <file.OBJ> ]\n");
    exit(1);
}

Object* parseOBJ(char* path) {
    // Create object to return
    Object* obj = new Object();

    // parse the .obj file
    GLMmodel* model = glmReadOBJ(path);

    // allocate space for my representation
    obj->faces = (face*) malloc(model->numtriangles * sizeof(face));
    obj->vertices = (GLfloat*) malloc(model->numvertices * 3 * sizeof(GLfloat));

    // populate vertices
    obj->numvertices = model->numvertices;
    memcpy(obj->vertices, model->vertices, obj->numvertices*3*sizeof(GLfloat));

    // populate faces with vertex indices
    obj->numfaces = model->numtriangles;
    for(int i = 0; i < model->numtriangles; i++) {
        face& f = obj->faces[i];
        memcpy(f.v, model->triangles[i].vindices, 3*sizeof(GLuint));
    }

    // populate faces with neighbor indices
    for(int i = 0; i < obj->numfaces; i++) {
        face& f = obj->faces[i];
        //printf("triangle %d has vertices %d,%d,%d\n", i, f.v[0], f.v[1], f.v[2]);
    }

    // discard parsing data structure
    glmDelete(model);

    return obj;
}

int main(int argc, char* argv[]) {
    // select the .obj filename to load
    char* objpath = NULL;
    switch (argc) {
        case 1: objpath = (char*) "cube.obj"; break;
        case 2: objpath = argv[1]; break;
        default: usage();
    }
    printf("-- parsing %s --\n", objpath);

    // parse OBJ file
    Object* obj = parseOBJ(objpath);
    printf("-- object has %d vertices, %d faces--\n", obj->numvertices, obj->numfaces);

    // cleanup
    delete obj;
    return 0;
}
