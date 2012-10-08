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
    for(int i = 0; i < numfaces; i++)
        faces[i].render();
    glEnd();
}

void Object::check() {
    // check all verticle values < 10.0
    for(int i = 0; i < 3 * numvertices; i++) {
        if (vertices[i] > 10.0)
            printf("suspicious vertex %d: %f\n", i, vertices[i]);
    }

    for(int i = 0; i < numfaces; i++) {
        for(int j = 0; j < 3; j++) {
            int vi = faces[i].vindices[j];
            if (vi < 0 || vi > 3*numvertices)
                printf("bad vindex on face %d, entry %d\n", i, j);
        }
    }

    printf("Object check completed\n");
}
