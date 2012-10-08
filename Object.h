#include <vector>
#include <stack>

#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

typedef glm::vec3 Vertex;

class Hedge;

class Face {
    Hedge& edge;
public:
    void render();
};

class Hedge {
    Face& f;
    Hedge& e;
    Hedge& n;
    Vertex& v;
};


class Object {
    std::vector<Face> faces;
    std::vector<Hedge> hedges;
    std::vector<Vertex> vertices;
public:
    void render();
    void check();
};

class SubDivObject {
private:
    GLenum polygon_mode;
    std::stack<Object*> objs;
public:
    SubDivObject(Object* base);
    void set_polygon_mode(GLenum mode);
    void refine();
    void coarsen();
    void render();
};
