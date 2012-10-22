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

    printf("Requested refinement of obj with %d vertices, %d faces, %d hedges\n",
            (int) oldo->vertices.size(),
            (int) oldo->faces.size(),
            (int) oldo->hedges.size());

    /* reserve space */
    newo->faces.reserve(oldo->faces.size()*4);
    newo->hedges.reserve(oldo->hedges.size()*4);
    newo->vertices.reserve(oldo->vertices.size()*2);

    foreach( Hedge* h, oldo->hedges )
        assert(h->pair != NULL && "can only handle closed meshes atm");

    // link up edge ptrs
    foreach( Hedge* h, oldo->hedges )
        h->v->edge = h;

    // set child vertices
    foreach( Vertex* h, oldo->vertices)
        h->refine(newo);

    // set midpoints
    foreach( Hedge* h, oldo->hedges )
        h->set_midpoint(newo);

    foreach( Face* f, oldo->faces )
        f->refine(newo);

    oldo->check(true);
    objs.push(newo);

    printf("Refined obj has %d vertices, %d faces, %d hedges\n",
            (int) newo->vertices.size(),
            (int) newo->faces.size(),
            (int) newo->hedges.size());
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

void Object::check(bool postrefine) {
    int num_boundaries = 0;
    int hno = 0;
    foreach(Hedge* h, this->hedges) {
        /* pair pointers are reflexive */
        if (h->pair != NULL)
            assert(h == h->pair->pair);
        else
            num_boundaries++;

        /* interior faces have pairs */
        if (h->f->interior == true)
            assert(h->pair != NULL);

        /* next pointers are circular */
        assert(h != h->next);
        assert(h != h->next->next);
        assert(h == h->next->next->next);

        /* vertex and next pointers are in the same direction */
        assert(h->v == h->next->oppv());
        assert(h->oppv() == h->prev()->v);

        /* child near and opposite vertex are reflexive */
        if (h->co != NULL && h->pair != NULL)
            assert(h->co->pair == h->pair->cv);

        /* v is not oppv */
        if (h->pair != NULL)
            assert(h->v != h->oppv());

        /* vertex.edge ptr is set */
        assert(h->v->edge != NULL);

        /* edges in opp direction */
        if (h->pair != NULL) {
            assert(h->v->val == h->pair->oppv()->val);
            assert(h->pair->v->val == h->oppv()->val);
            assert(h->v == h->pair->oppv());
            assert(h->pair->v == h->oppv());
        }

        /* checks for objects after being refined */
        if (postrefine) {
            /* child ptrs are set */
            assert(h->co != NULL);
            assert(h->cv != NULL);

            if (h->pair != NULL) {
                /* child pairs are set */
                assert(h->co->pair != NULL);
                assert(h->cv->pair != NULL);

                /* children are in right direction */
                assert(h->v->child == h->cv->v);
                assert(h->oppv()->child == h->co->oppv());

                /* children are reflexive */
                assert(h->cv->pair == h->pair->co);
                assert(h->co->pair == h->pair->cv);
            }
        }
    }

    printf("-- consistency tests passed (%d vertices, %d faces, %d hedges, %d boundary hedges) --\n",
            (int) vertices.size(),
            (int) faces.size(),
            (int) hedges.size(),
            num_boundaries);
}

SubDivObject::SubDivObject(Object* base) :
    polygon_mode(GL_LINE)
{
    objs.push(base);
}

void
Face::render() {
    vec3& v0 = this->edge->v->val;
    vec3& v1 = this->edge->next->v->val;
    vec3& v2 = this->edge->next->next->v->val;
    vec3 norm = normalize( cross(v2-v1, v1-v0) );

    glNormal3fv( (GLfloat*) &norm  );
    glVertex3fv( (GLfloat*) &v0 );
    glVertex3fv( (GLfloat*) &v1 );
    glVertex3fv( (GLfloat*) &v2 );
}

Hedge*
Object::new_hedge(Face* f, Vertex* v, Hedge* next) {
    Hedge* newh = new Hedge();
    newh->f = f;
    newh->v = v;
    newh->next = next;
    newh->pair = NULL;
    newh->co = NULL;
    newh->cv = NULL;
    newh->mp = NULL;
    v->edge = newh;
    hedges.push_back(newh);
    return newh;
}

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
    return this->prev()->v;
}

Vertex::Vertex(GLfloat* v) : edge(NULL), child(NULL) {
    this->val = vec3(v[0], v[1], v[2]);
}

Vertex::Vertex(vec3 val) : edge(NULL), child(NULL), val(val) { }

Vertex::Vertex(Hedge* h) : edge(NULL), child(NULL) {
    if (h->pair == NULL)
        assert(!"can't handle boundary vertices atm");

    vec3& v0 = h->v->val;
    vec3& v1 = h->pair->v->val;
    vec3& v2 = h->next->v->val;
    vec3& v3 = h->pair->next->v->val;

    this->val = vec3(3.0/8.0)*(v0+v1) + vec3(1.0/8.0)*(v2+v3);
}

Hedge*
Hedge::refine(Object* newo) {
    Face* f = newo->new_face(false);
    Vertex* m1 = this->mp;
    Vertex* m2 = this->next->mp;

    assert(v->child != NULL);
    Hedge* h0 = newo->new_hedge(f, v->child);
    Hedge* h2 = newo->new_hedge(f, m1, h0);
    Hedge* h1 = newo->new_hedge(f, m2, h2);
    h0->next = f->edge = h1;

    cv = h0;
    next->co = h1;

    if (pair != NULL)
        cv->set_pair(pair->co);
    if (next->pair != NULL)
        next->co->set_pair(next->pair->cv);

    return cv->prev();
}

void
Face::refine(Object* newo) {
    Face* f = newo->new_face(true);

    Hedge* pair_h0 = this->edge->refine(newo);
    Hedge* pair_h1 = this->edge->next->refine(newo);
    Hedge* pair_h2 = this->edge->next->next->refine(newo);

    Vertex* v0 = pair_h0->v;
    Vertex* v1 = pair_h1->v;
    Vertex* v2 = pair_h2->v;

    Hedge* h2 = newo->new_hedge(f, v0);
    Hedge* h1 = newo->new_hedge(f, v2, h2);
    Hedge* h0 = newo->new_hedge(f, v1, h1);
    h2->next = f->edge = h0;

    h0->set_pair(pair_h0);
    h1->set_pair(pair_h1);
    h2->set_pair(pair_h2);
}

void
Hedge::set_pair(Hedge* o) {
    this->pair = o;
    if (o != NULL)
        o->pair = this;
}

void
Hedge::set_midpoint(Object* newo) {
    bool has_local_mp = mp != NULL;
    bool has_pair = pair != NULL;
    bool has_remote_mp = has_pair && (pair->mp != NULL);

    if ( has_pair &&  has_local_mp &&  has_remote_mp) assert(mp == pair->mp);
    if ( has_pair &&  has_local_mp && !has_remote_mp) pair->mp = mp;
    if ( has_pair && !has_local_mp &&  has_remote_mp) mp = pair->mp;
    if ( has_pair && !has_local_mp && !has_remote_mp) {
        pair->mp = mp = new Vertex(this);
        newo->vertices.push_back(mp);
    }
    if (!has_pair &&  has_local_mp) return;
    if (!has_pair && !has_local_mp) {
        mp = new Vertex(this);
        newo->vertices.push_back(mp);
    }
}

Face*
Object::new_face(bool interior) {
    Face* f = new Face();
    f->interior = interior;
    this->faces.push_back(f);
    return f;
}

int
Vertex::valence() {
    int valence = 0;
    Hedge* current = edge;
    do {
        current = current->next->pair;
        valence++;
    } while (current != NULL && current != edge);

    /* handle boundary edges */
    if (current != edge) {
        current = edge;
        do {
            current = current->prev()->pair;
            valence++;
        } while (current != NULL && current != edge);
    }

    return valence;
}

void
Vertex::refine(Object* newo) {
    int ni = valence();
    float n = (float) ni;
    float s = (0.625 - pow((0.375 + 0.25*cos(2.0*M_PI/n)), 2.0)) / n;

    vec3 alpha(5.0/8.0);

    vec3 q(0.0);
    Hedge* current = this->edge;
    for(int i = 0; i < ni; i++, current = current->next->pair)
        q += current->pair->v->val;
    q = vec3(1.0/n)*q;

    vec3 childval = alpha*this->val + (vec3(1.0)-alpha)*q;
    this->child = new Vertex(childval);
    newo->vertices.push_back(this->child);
}
