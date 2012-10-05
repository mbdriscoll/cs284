#include <string>

#include <glm/gtx/normal.hpp>

#include "Face.h"

using namespace glm;

/* return j-th entry of i-th vertex */
#define V(i,j) (vertices[3*vindices[(i)]+(j)])

Face::Face(GLuint* vindices, GLfloat* vertices) :
  vertices(vertices) {
    memcpy(this->vindices, vindices, 3*sizeof(GLuint));
}

void
Face::render() {
    vec3 v0( V(0,0), V(0,1), V(0,2) );
    vec3 v1( V(1,0), V(1,1), V(1,2) );
    vec3 v2( V(2,0), V(2,1), V(2,2) );
    const vec3 n = triangleNormal(v1, v2, v0);

    glNormal3fv( &n[0]   );
    glVertex3fv( &V(0,0) );
    glVertex3fv( &V(1,0) );
    glVertex3fv( &V(2,0) );
}
