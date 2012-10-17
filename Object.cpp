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

    newo->check();
    printf("New level has %d vertices, %d faces\n",
            (int) newo->vertices.size(), (int) newo->faces.size());
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

        /* child near and opposite vertex are reflexive */
        if (h->cv != NULL || h->co != NULL)
            assert(h->co->pair == h->pair->cv);
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
    return this->next->next->v;
}

Vertex::Vertex(GLfloat* v) : edge(NULL) {
    this->val = vec3(v[0], v[1], v[2]);
}

Vertex*
Hedge::find_or_create_midpoint(Object* newo) {
    if (this->co != NULL) {
        return this->co->v;
    } else if (this->pair != NULL && this->pair->co != NULL) {
        return this->pair->co->v;
    } else {
        Vertex* newv = new Vertex(this);
        newo->vertices.push_back(newv);
        return newv;
    }
}

Hedge*
Hedge::refine(Object* newo) {
    Face* f = new Face();
    Vertex* m1 = this->find_or_create_midpoint(newo);
    Vertex* m2 = this->next->find_or_create_midpoint(newo);

    Hedge* h2 = new Hedge(f, m1);
    Hedge* h1 = new Hedge(f, m2, h2);
    Hedge* h0 = new Hedge(f, v, h1);

    h2->next = h0;
    f->edge = h0;

    this->cv = h0;
    this->next->co = h1;

    if (this->pair != NULL)
        h0->set_pair(pair->co);
    if (this->next->pair != NULL)
        h1->set_pair(next->pair->cv);

    newo->hedges.push_back(h0);
    newo->hedges.push_back(h1);
    newo->hedges.push_back(h2);
    newo->faces.push_back(f);

    return h2;
}

void
Face::refine(Object* newo) {
    Hedge* pair_h0 = this->edge->refine(newo);
    Hedge* pair_h1 = this->edge->next->refine(newo);
    Hedge* pair_h2 = this->edge->next->next->refine(newo);

    Face* f = new Face();
    Vertex* v0 = pair_h0->v;
    Vertex* v1 = pair_h1->v;
    Vertex* v2 = pair_h2->v;

    Hedge* h2 = new Hedge(f, v0);
    Hedge* h1 = new Hedge(f, v2, h2);
    Hedge* h0 = new Hedge(f, v1, h1);

    h2->next = h0;
    f->edge = h0;

    h0->set_pair(pair_h0);
    h1->set_pair(pair_h1);
    h2->set_pair(pair_h2);

    newo->hedges.push_back(h0);
    newo->hedges.push_back(h1);
    newo->hedges.push_back(h2);
    newo->faces.push_back(f);
}

void
Hedge::set_pair(Hedge* o) {
    this->pair = o;
    if (this->pair != NULL)
        o->pair = this;
}

Vertex::Vertex(Hedge* h) : edge(NULL) {
    this->val = (h->v->val + h->oppv()->val) / vec3(2.0, 2.0, 2.0);
}
