#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

class Face {
  private:
    GLfloat* vertices;  //vertices
    GLuint vindices[3]; // vertex indices
    Face* neighbors[3]; // neighbors
  public:
    Face(GLuint* vindices, GLfloat* vertices);
    glm::vec3 normal();
    void render();
};
