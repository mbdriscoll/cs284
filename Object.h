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
    Vertex(Hedge* h);
    int valence();
};

class Face {
public:
    Hedge* edge;
    bool interior;

    void render();
    void refine(Object* newo);
};

class Hedge {
public:
    Face* f;
    Hedge* next;
    Hedge* pair;
    Vertex* v;

    Vertex* mp;
    Hedge* cv;
    Hedge* co;

    Hedge* prev();
    Vertex* oppv();
    void set_midpoint(Object* newo);
    Hedge* refine(Object* newo);
    void set_pair(Hedge* o);
};


class Object {
public:
    std::vector<Face*> faces;
    std::vector<Hedge*> hedges;
    std::vector<Vertex*> vertices;

    Hedge* new_hedge(Face* f, Vertex* v, Hedge* next=NULL);
    Face* new_face(bool interior = false);
    void render();
    void check(bool postrefine=false);
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
