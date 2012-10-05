#include "Face.h"
#include "Object.h"

Object::Object() : polygon_mode(GL_LINE)
{ }

Object::~Object() {
    free(this->vertices);
    free(this->faces);
}

void
Object::set_polygon_mode(GLenum mode) {
    polygon_mode = mode;
    render();
}

void
Object::refine() {
}

void
Object::coarsen() {
}

void
Object::render() {
    GLfloat materialShininess[] = {128.0f};
    GLfloat materialAmbDiff[] = {0.9f, 0.1f, 0.1f, 1.0f};
    GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialAmbDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
    glBegin(GL_TRIANGLES);
    for(int i = 0; i < numfaces; i++) {
        faces[i].render();
    }
    glEnd();
}
