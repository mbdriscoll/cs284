#include <string>
#include "Face.h"

using namespace glm;

vec3
Face::normal() {
}

Face::Face(GLuint* vindices, GLfloat* vertices) :
    vertices(vertices)
{
    memcpy(this->vindices, vindices, 3*sizeof(GLuint));
}

void
Face::render() {
    glVertex3fv( &vertices[3*vindices[0]] );
    glVertex3fv( &vertices[3*vindices[1]] );
    glVertex3fv( &vertices[3*vindices[2]] );
}
