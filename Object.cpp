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
    f(f), v(v), next(next), pair(NULL), co(NULL), cv(NULL) { }

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

Vertex*
Hedge::find_or_create_midpoint(Object* newo) {
    if (this->pair == NULL ||
        this->pair->co == NULL) {
        Vertex* newv = new Vertex(this);
        newo->vertices.push_back(newv);
        return newv;
    } else {
        assert(this->pair->co->v != NULL);
        return this->pair->co->v;
    }
}

void
Face::refine(Object* newo) {
    Hedge* h0 = this->edge;
    Hedge* h1 = this->edge->next;
    Hedge* h2 = this->edge->next->next;

    Vertex* v0 = h0->v;
    Vertex* v1 = h1->v;
    Vertex* v2 = h2->v;

    Vertex* v01 = h0->find_or_create_midpoint(newo);
    Vertex* v12 = h1->find_or_create_midpoint(newo);
    Vertex* v20 = h2->find_or_create_midpoint(newo);

    Face* fi = new Face();
    Hedge* h_012i = new Hedge(fi, v01);
    Hedge* h_120i = new Hedge(fi, v12, h_012i);
    Hedge* h_201i = new Hedge(fi, v20, h_120i);
    h_012i->next = h_201i;
    fi->edge = h_012i;
    newo->hedges.push_back(h_012i);
    newo->hedges.push_back(h_120i);
    newo->hedges.push_back(h_201i);
    newo->faces.push_back(fi);

#if 0
    Face* f0 = new Face();
    Hedge* h_001 = new Hedge(f0, v0);
    Hedge* h_201 = new Hedge(f0, v01, h_001);
    Hedge* h_200 = new Hedge(f0, v20, h_201);
    h_001->next = h_200;
    f0->edge = h_001;
    h_201->pair = h_201i;
    h_201i->pair = h_201;
    h_001->pair = (h0->pair != NULL) ? h0->pair->co : NULL;
    if (h_001->pair) h_001->pair->pair = h_001;
    h_200->pair = (h2->pair != NULL) ? h2->pair->cv : NULL;
    if (h_200->pair) h_200->pair->pair = h_200;
    newo->hedges.push_back(h_001);
    newo->hedges.push_back(h_201);
    newo->hedges.push_back(h_200);
    newo->faces.push_back(f0);
#endif

#define MKEDGES(a, b, c) \
    Face* f ## a = new Face(); \
    Hedge* h_ ## a ## a ## b = new Hedge(f ## a, v ## a); \
    Hedge* h_ ## c ## a ## b = new Hedge(f ## a, v ## a ## b, h_ ## a ## a ## b); \
    Hedge* h_ ## c ## a ## a = new Hedge(f ## a, v ## c ## a, h_ ## c ## a ## b); \
    h_ ## a ## a ## b->next = h_ ## c ## a ## a; \
    f ## a->edge = h_ ## a ## a ## b; \
    h_ ## c ## a ## b->pair = h_ ## c ## a ## b ## i; \
    h_ ## c ## a ## b ## i->pair = h_ ## c ## a ## b; \
    h_ ## a ## a ## b->pair = (h ## a->pair != NULL) ? h ## a->pair->co : NULL; \
    if (h_ ## a ## a ## b->pair) h_ ## a ## a ## b->pair->pair = h_ ## a ## a ## b; \
    h_ ## c ## a ## a->pair = (h ## c->pair != NULL) ? h ## c->pair->cv : NULL; \
    if (h_ ## c ## a ## a->pair) h_ ## c ## a ## a->pair->pair = h_ ## c ## a ## a; \
    newo->hedges.push_back(h_ ## a ## a ## b); \
    newo->hedges.push_back(h_ ## c ## a ## b); \
    newo->hedges.push_back(h_ ## c ## a ## a); \
    newo->faces.push_back(f ## a); \

    MKEDGES(0, 1, 2);
    MKEDGES(1, 2, 0);
    MKEDGES(2, 0, 1);
}

Vertex::Vertex(Hedge* h) : edge(NULL) {
    this->val = (h->v->val + h->oppv()->val) / vec3(2.0, 2.0, 2.0);
}
