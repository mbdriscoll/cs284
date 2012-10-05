#include <cstdio>
#include <cstdlib>

#include <string>

#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include "obj.h"


#define MAX_VALENCE 64

using namespace std;

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
        memcpy(obj->faces[i].v, model->triangles[i].vindices, 3*sizeof(GLuint));
    }

    // TODO populate faces with neighbor indices
    for(int i = 0; i < obj->numfaces; i++)
        memset(obj->faces[i].n, NULL, 3*sizeof(face*));

    // discard parsing data structure and return
    glmDelete(model);
    return obj;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0f,-6.0f,6.0f, 0.0f,0.0f,0.0f, 0.0f,1.0f,1.0f);

    glutWireTeapot(1.0);
    glutSolidTeapot(1.0);

    glutSwapBuffers();
    glFlush();
}

void reshape(int w, int h) {
   glViewport(0,0,w,h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(30,w/h,1,40);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '=':
          /* zoom in */;
          break;
        case '-':
          /* zoom out */;
          break;
        case 'q':
          exit(0);
        default:
          printf("Unrecognized key: %c\n", key);
          break;
    }
}

void special(int key, int x, int y) {
    switch (key) {
        case 101: printf("up\n"); break;
        case 103: printf("down\n"); break;
        case 100: printf("left\n"); break;
        case 102: printf("right\n"); break;
        default:
          printf("Unrecognized special key: <%d>\n", key);
          break;
    }
}

void init_scene() {
    glClearColor (0.0, 0.0, 0.0, 0.0);

    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);

    float pos0[] = {-0.0f, 16.0f, -0.0f, 0.0f };
    float pos1[] = {-16.0f, -16.0f, 0.0f, 0.0f };
    float pos2[] = {16.0f, -16.0f, -0.0f, 0.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glLightfv(GL_LIGHT1, GL_POSITION, pos1);
    glLightfv(GL_LIGHT2, GL_POSITION, pos2);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_LIGHT2);
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

    // start graphics
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Michael's SubDiv");

    init_scene();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMainLoop();

    // cleanup
    delete obj;
    return 0;
}
