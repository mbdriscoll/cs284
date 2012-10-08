#include <string>

#include <glm/gtx/normal.hpp>

#include "Face.h"

using namespace glm;

/* return j-th entry of i-th vertex */
#define V(i,j) (vertices[3*(vindices[(i)])+(j)])

Face::Face(GLuint* vi, GLfloat* vertices) :
  vertices(vertices) {
      this->vindices[0] = vi[0];
      this->vindices[1] = vi[1];
      this->vindices[2] = vi[2];
    memset(neighbors, NULL, 4*sizeof(Face*));
}

void
Face::render() {
    vec3 v0( V(0,0), V(0,1), V(0,2) );
    vec3 v1( V(1,0), V(1,1), V(1,2) );
    vec3 v2( V(2,0), V(2,1), V(2,2) );
    const vec3 n = triangleNormal(v0, v1, v2);

    glNormal3fv( &n[0]   );
    glVertex3fv( &V(0,0) );
    glVertex3fv( &V(1,0) );
    glVertex3fv( &V(2,0) );
}
