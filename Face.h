#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

class Face {
  public:
    GLfloat* vertices;  //vertices
    Face* neighbors[4]; // neighbors + sentinel
    GLuint vindices[3]; // vertex indices
    Face(GLuint* vindices, GLfloat* vertices);
    glm::vec3 normal();
    void render();
};
