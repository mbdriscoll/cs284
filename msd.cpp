#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include <string>

#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "obj.h"
#include "util.h"

#include "Face.h"
#include "Object.h"

#define MAX_VALENCE 64
#define ROTAMOUNT 5.0f /* in degrees */

using namespace glm;
using namespace std;

Object* obj;
vec3 eye(0.0, 0.0, 1.0);
vec3 center(0.0, 0.0, 0.0);
vec3 up(0.0, 1.0, 0.0);
GLMmodel* model;
bool useGlmDraw = false;

void usage() {
    printf("Usage: ./msd [ <file.OBJ> ]\n");
    exit(1);
}

Object* parseOBJ(char* path) {
    // Create object to return
    Object* obj = new Object();

    // parse the .obj file
    model = glmReadOBJ(path);

    // allocate space for my representation
    obj->faces = (Face*) malloc(model->numtriangles * sizeof(Face));
    obj->vertices = (GLfloat*) malloc(3 + model->numvertices * 3 * sizeof(GLfloat));

    // populate vertices
    obj->numvertices = model->numvertices + 1;
    memcpy(obj->vertices, model->vertices, 3 + obj->numvertices*3*sizeof(GLfloat));

    // populate faces with vertex indices
    obj->numfaces = model->numtriangles;
    for(int i = 0; i < model->numtriangles; i++)
        obj->faces[i] = Face(model->triangles[i].vindices, obj->vertices);

    // set neighbor pointers
    for(int i = 0; i < model->numtriangles; i++) {
        Face* f = &obj->faces[i];
        for(int j = 0; j < model->numtriangles; j++) {
            Face* g = &obj->faces[j];
            int shared = 0;
            for(int k = 0; k < 3; k++)
                for(int l = 0; l < 3; l++)
                    if (f->vindices[k] == g->vindices[l])
                        shared += 1;
            if (shared == 2) {
                int k;
                for(k = 0; k < 3; k++)
                    if (f->neighbors[k] == NULL)
                        break;
                if (k == 3)
                    printf("Warning, >3 neighbors for vertices (%d %d)\n", i, j);
                else
                    f->neighbors[k] = g;
            }
        }
    }

    obj->check();
    return obj;
}

void light() {
    GLfloat pos0[]={100.0, 100.0, 100.0, 0.5};
    GLfloat col0[]={1.0, 1.0, 1.0, 1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col0);
    glLightfv(GL_LIGHT0, GL_SPECULAR, col0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glEnable(GL_LIGHT0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    mat4 mv = lookAt(eye,center,up);
    glLoadMatrixf(&mv[0][0]);

    if (useGlmDraw) {
        glmFacetNormals(model);
        glmDraw(model, GLM_FLAT);
    }
    else
        obj->render();

    glutSwapBuffers();
}

float rot = 0.0;
void animate() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-10.0, -10.0, -10.0);
    glRotatef(rot, 1.0, 1.0, 0.0);
    light();
    rot += 1;
    if (rot > 360) rot = 0.0;
    glutPostRedisplay();
}

void reshape(int w, int h) {
   glViewport(0,0,w,h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(30,w/h,.01,100);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '=': eye *= 0.9f; break; /* zoom in */
        case '-': eye *= 1.1f; break; /* zoom out */
        case 'q':
        case 27:  exit(0); /* quit */
        case ']': obj->refine(); break; /* increase subdivs */
        case '[': obj->coarsen(); break; /* decrease subdivs */
        case 'p': obj->set_polygon_mode(GL_POINT); break; /* point polygon mode */
        case 'l': obj->set_polygon_mode(GL_LINE); break; /* line polygon mode */
        case 'f': obj->set_polygon_mode(GL_FILL); break; /* face polygon mode */
        case 't': useGlmDraw = !useGlmDraw; break; /* toggle my vs glmDraw rendering */
        default:  printf("Unrecognized key: %c\n", key); break;
    }
    glutPostRedisplay();
}

void special(int key, int x, int y) {
    switch (key) {
        case 101: /* up */
            eye = rotate(eye, ROTAMOUNT, cross(up,eye));
            up  = rotate( up, ROTAMOUNT, cross(up,eye));
            break;
        case 103: /* down */
            eye = rotate(eye, -ROTAMOUNT, cross(up,eye));
            up  = rotate( up, -ROTAMOUNT, cross(up,eye));
            break;
        case 100: /* left */
            eye = rotate(eye, ROTAMOUNT, up);
            up  = rotate( up, ROTAMOUNT, up);
            break;
        case 102: /* right */
            eye = rotate(eye, -ROTAMOUNT, up);
            up  = rotate( up, -ROTAMOUNT, up);
            break;
        default:
          printf("Unrecognized special key: <%d>\n", key);
          break;
    }
    glutPostRedisplay();
}

void init_scene() {
    glClearColor (0.0, 0.0, 0.0, 0.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glShadeModel(GL_FLAT);

    light();
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
    obj = parseOBJ(objpath);
    printf("-- object has %d vertices, %d faces--\n", obj->numvertices, obj->numfaces);

    // start graphics
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 1000);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Michael's SubDiv");

    init_scene();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutIdleFunc(animate);
    glutMainLoop();

    // cleanup
    delete obj;
    return 0;
}
