#include <map>
#include <algorithm>

#include "msd.h"
#include "Object.h"

#define max(x,y) (((x)>(y))?(x):(y))
#define min(x,y) (((x)<(y))?(x):(y))

using namespace glm;
using namespace std;

typedef std::pair<Vertex*,Vertex*> VVpair;

void
SubDivObject::set_polygon_mode(GLenum mode) {
    polygon_mode = mode;
    render();
}

void
SubDivObject::refine() {
    Object* oldo = objs.top();
    Object* newo = new Object();

    /* reserve space */
    newo->faces.reserve(oldo->faces.size()*4+4);
    newo->hedges.reserve(oldo->hedges.size()*4+4);
    newo->vertices.reserve(oldo->vertices.size()*2+2);

    foreach( Face* f, oldo->faces )
        f->refine(newo);

    objs.push(newo);
}

void
SubDivObject::coarsen() {
    if (objs.size() <= 1) {
        printf("Cannot refine beyond original mesh.\n");
        return;
    }
    objs.pop();
}

void
SubDivObject::render() {
    glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
    objs.top()->render();
}

void
Object::render() {
    GLfloat materialShininess[] = {128.0f};
    GLfloat materialAmbDiff[] = {0.9f, 0.1f, 0.1f, 1.0f};
    GLfloat materialSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, materialAmbDiff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, materialShininess);

    glBegin(GL_TRIANGLES);
    foreach(Face* f, faces)
        f->render();
    glEnd();
}

void Object::check() {
    foreach(Hedge* h, this->hedges) {
        /* pair pointers are reflexive */
        if (h->pair != NULL)
            assert(h == h->pair->pair);

        /* next pointers are circular */
        assert(h == h->next->next->next);
    }

    printf("-- consistency tests passed --\n");
}

SubDivObject::SubDivObject(Object* base) :
    polygon_mode(GL_LINE)
{
    objs.push(base);
}

void
Face::render() {
    Hedge* ch = this->edge;

    vec3& v0 = this->edge->v->val;
    vec3& v1 = this->edge->next->v->val;
    vec3& v2 = this->edge->next->next->v->val;
    vec3 norm = normalize( cross(v2-v1, v1-v0) );

    glNormal3fv( (GLfloat*) &norm  );
    glVertex3fv( (GLfloat*) &v0 );
    glVertex3fv( (GLfloat*) &v1 );
    glVertex3fv( (GLfloat*) &v2 );
}

Hedge::Hedge(Face* f, Vertex* v, Hedge* next) :
    f(f), v(v), next(next), pair(NULL) { }

void
Object::match_pairs() {
    map<pair<Vertex*,Vertex*>,Hedge*> vtoe;
    foreach (Hedge* h, this->hedges)
        vtoe[VVpair(h->v,h->oppv())] = h;
    foreach (Hedge* h, this->hedges)
        h->pair = vtoe[VVpair(h->oppv(),h->v)];
}

inline Hedge*
Hedge::prev() {
    return this->next->next;
}

inline Vertex*
Hedge::oppv() {
    return this->next->v;
}

Vertex::Vertex(GLfloat* v) : edge(NULL) {
    this->val = vec3(v[0], v[1], v[2]);
}

#define MKHEDGES(fid, va, vb, vc) \
    Face* f ## fid = new Face(); \
    Hedge* h_ ## va ## _ ## vb = new Hedge(f ## fid, v ## va ); \
    Hedge* h_ ## vb ## _ ## vc = new Hedge(f ## fid, v ## vb , h_ ## va ## _ ## vb ); \
    Hedge* h_ ## vc ## _ ## va = new Hedge(f ## fid, v ## vc , h_ ## vb ## _ ## vc ); \
    h_ ## va ## _ ## vb ->next = h_ ## vc ## _ ## va ; \
    f ## fid->edge = h_ ## va ## _ ## vb ; \
    newo->faces.push_back(f ## fid); \
    newo->hedges.push_back(h_ ## va ## _ ## vb ); \
    newo->hedges.push_back(h_ ## vb ## _ ## vc ); \
    newo->hedges.push_back(h_ ## vc ## _ ## va );

void
Face::refine(Object* newo) {
    Hedge* h0 = this->edge;
    Hedge* h1 = this->edge->next;
    Hedge* h2 = this->edge->next->next;

    Vertex* v0 = h0->v;
    Vertex* v1 = h1->v;
    Vertex* v2 = h2->v;

    Vertex* v01 = new Vertex(h0);
    Vertex* v12 = new Vertex(h1);
    Vertex* v20 = new Vertex(h2);
    newo->vertices.push_back(v01);
    newo->vertices.push_back(v12);
    newo->vertices.push_back(v20);

    MKHEDGES(0, 0, 01, 20);
    MKHEDGES(1, 1, 12, 01);
    MKHEDGES(2, 2, 20, 12);
    MKHEDGES(3, 01, 12, 20);
}

Vertex::Vertex(Hedge* h) : edge(NULL) {
    this->val = (h->v->val + h->oppv()->val) / vec3(2.0, 2.0, 2.0);
}
