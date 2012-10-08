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
