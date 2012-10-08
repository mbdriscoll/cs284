#include <vector>
#include <stack>

#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

class Hedge;

class Vertex {
public:
    glm::vec3 val;
    Hedge* edge;
    Vertex(GLfloat* v);
};

class Face {
public:
    Hedge* edge;
    void render();
};

class Hedge {
public:
    Face* f;
    Hedge* next;
    Hedge* pair;
    Vertex* v;

    Hedge(Face* f, Vertex* v, Hedge* next=NULL);
    Hedge* prev();
    Vertex* oppv();
};


class Object {
public:
    std::vector<Face*> faces;
    std::vector<Hedge*> hedges;
    std::vector<Vertex*> vertices;
    void render();
    void check();
    void match_pairs();
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
