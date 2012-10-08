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

    Face* f0 = new Face();
    Hedge* h_0_01 = new Hedge(f0, v0);
    Hedge* h_01_20 = new Hedge(f0, v01, h_0_01);
    Hedge* h_20_0 = new Hedge(f0, v20, h_01_20);
    h_0_01->next = h_20_0;
    f0->edge = h_0_01;
    newo->faces.push_back(f0);
    newo->hedges.push_back(h_0_01);
    newo->hedges.push_back(h_01_20);
    newo->hedges.push_back(h_20_0);

    Face* f1 = new Face();
    Hedge* h_1_12  = new Hedge(f1, v1);
    Hedge* h_12_01 = new Hedge(f1, v12, h_1_12);
    Hedge* h_01_1  = new Hedge(f1, v01, h_12_01);
    h_1_12->next = h_01_1;
    f1->edge = h_1_12;
    newo->faces.push_back(f1);
    newo->hedges.push_back(h_1_12);
    newo->hedges.push_back(h_12_01);
    newo->hedges.push_back(h_01_1);

    Face* f2 = new Face();
    Hedge* h_2_20  = new Hedge(f2, v2);
    Hedge* h_20_12 = new Hedge(f2, v20, h_2_20);
    Hedge* h_12_2  = new Hedge(f2, v12, h_20_12);
    h_2_20->next = h_12_2;
    f2->edge = h_2_20;
    newo->faces.push_back(f2);
    newo->hedges.push_back(h_2_20);
    newo->hedges.push_back(h_20_12);
    newo->hedges.push_back(h_12_2);

    Face* fc = new Face();
    Hedge* h_01_12 = new Hedge(fc, v01);
    Hedge* h_12_20 = new Hedge(fc, v12, h_01_12);
    Hedge* h_20_01 = new Hedge(fc, v20, h_12_20);
    h_01_12->next = h_20_01;
    fc->edge = h_01_12;
    newo->faces.push_back(fc);
    newo->hedges.push_back(h_01_12);
    newo->hedges.push_back(h_12_20);
    newo->hedges.push_back(h_20_01);
}

Vertex::Vertex(Hedge* h) : edge(NULL) {
    this->val = (h->v->val + h->oppv()->val) / vec3(2.0, 2.0, 2.0);
}
