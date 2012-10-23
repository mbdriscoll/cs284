#include <cstdio>
#include <cstdlib>
#include <cstdarg>

#include <string>

#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "SOIL.h"
#include "glm.h"
#include "util.h"

#include "Object.h"

#define MAX_VALENCE 64
#define ROTAMOUNT 5.0f /* in degrees */

using namespace glm;
using namespace std;

GLuint texture[1];
SubDivObject* sdobj;
vec3 eye(0.0, 0.0, 7.0);
vec3 center(0.0, 0.0, 0.0);
vec3 up(0.0, 1.0, 0.0);
GLMmodel* model;
bool moveLights = true;

void usage() {
    printf("Usage: ./msd [ <file.OBJ> ]\n");
    exit(1);
}

SubDivObject* parseOBJ(char* path) {
    // Create object to return
    Object* obj = new Object();

    // parse the .obj file
    model = glmReadOBJ(path);
    glmUnitize(model);

    // reserve space for objects
    obj->faces.reserve(model->numtriangles);
    obj->hedges.reserve(3*model->numtriangles);
    obj->vertices.reserve(model->numvertices);

    for(int i = 1; i <= model->numvertices; i++) {
        GLfloat* v = &model->vertices[i*3];
        GLfloat* t = &model->texcoords[i*2];
        printf("read tex coords %f %f\n", t[0], t[1]);
        obj->vertices.push_back( new Vertex(v,t) );
    }

    for(int i = 0; i < model->numtriangles; i++) {
        GLuint* t = model->triangles[i].vindices;
        GLuint i0 = t[0];
        GLuint i1 = t[1];
        GLuint i2 = t[2];
        Vertex* v0 = obj->vertices[i0-1];
        Vertex* v1 = obj->vertices[i1-1];
        Vertex* v2 = obj->vertices[i2-1];

        GLfloat* tv0 = &model->texcoords[i0*2];
        GLfloat* tv1 = &model->texcoords[i1*2];
        GLfloat* tv2 = &model->texcoords[i2*2];
        vec2 t0 = vec2(tv0[0], tv0[1]);
        vec2 t1 = vec2(tv1[0], tv1[1]);
        vec2 t2 = vec2(tv2[0], tv2[1]);

        Face* f = obj->new_face(false);
        Hedge* h0 = obj->new_hedge(f, t0, v0);
        Hedge* h1 = obj->new_hedge(f, t1, v1, h0);
        Hedge* h2 = obj->new_hedge(f, t2, v2, h1);

        /* fix up circular references */
        h0->next = h2;
        f->edge = h0;
    }

    obj->match_pairs();

    return new SubDivObject(obj);
}

void light() {
    GLfloat pos0[]={100.0, 100.0, -100.0, 0.5};
    GLfloat col0[]={0.0, 0.0, 1.0, 1.0};
    //glLightfv(GL_LIGHT0, GL_DIFFUSE, col0);
    //glLightfv(GL_LIGHT0, GL_SPECULAR, col0);
    glLightfv(GL_LIGHT0, GL_POSITION, pos0);
    glEnable(GL_LIGHT0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    mat4 mv = lookAt(eye,center,up);
    glLoadMatrixf(&mv[0][0]);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture[0]);
    sdobj->render();

    glutSwapBuffers();
}

float rot = 0.0;
void animate() {
    if (moveLights == false)
        glutIdleFunc(NULL);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-10.0, -10.0, -10.0);
    glRotatef(rot, 1.0, 1.0, 0.0);
    //light();
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
        case ']': sdobj->refine(); break; /* increase subdivs */
        case '[': sdobj->coarsen(); break; /* decrease subdivs */
        case 'p': sdobj->set_polygon_mode(GL_POINT); break; /* point polygon mode */
        case 'l': sdobj->set_polygon_mode(GL_LINE); break; /* line polygon mode */
        case 'f': sdobj->set_polygon_mode(GL_FILL); break; /* face polygon mode */
        case 'a': moveLights = !moveLights; /* move lights */
                  glutIdleFunc(moveLights ? animate : NULL);
                  break;
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
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glFrontFace(GL_CW);

    //light();

    glEnable(GL_TEXTURE_2D);
    texture[0] = SOIL_load_OGL_texture
        (
         "lizard.bmp",
         SOIL_LOAD_AUTO,
         SOIL_CREATE_NEW_ID,
         SOIL_FLAG_INVERT_Y
        );
    if (texture[0] == 0)
        printf("SOIL load error: %s\n", SOIL_last_result());

    glBindTexture(GL_TEXTURE_2D, texture[0]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
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
    sdobj = parseOBJ(objpath);

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
    delete sdobj;
    return 0;
}
