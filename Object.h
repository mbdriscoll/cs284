#include <vector>
#include <stack>

#include <gl/glut.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <glm/glm.hpp>

class Hedge;
class Object;

class Vertex {
public:
    glm::vec3 val;
    Hedge* edge;

    Vertex(GLfloat* v);
    Vertex(Hedge* h); // interpolate along hedge to create new v
};

class Face {
public:
    Hedge* edge;

    void render();
    void refine(Object* newo);
};

class Hedge {
public:
    Face* f;
    Hedge* next;
    Hedge* pair;
    Vertex* v;

    Hedge* cv;
    Hedge* co;

    Hedge(Face* f, Vertex* v, Hedge* next=NULL);
    Hedge* prev();
    Vertex* oppv();
    Vertex* find_or_create_midpoint(Object* newo);
    Hedge* refine(Object* newo);
    void set_pair(Hedge* o);
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
