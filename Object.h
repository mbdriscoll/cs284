#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

class Face;

class Object {
private:
    GLenum polygon_mode;
public:
    GLuint numvertices, numfaces;
    GLfloat* vertices;
    Face* faces;

    Object();
    ~Object();
    void set_polygon_mode(GLenum mode);
    void refine();
    void coarsen();
    void render();
};
