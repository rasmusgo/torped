#include <algorithm>
#include <vector>

#include "collide.hpp"
#include "comparelist.hpp"
//#include "gameapp.hpp"
#include "logging.hpp"
#include "physics.hpp"
//#include "world.h"

//#define min(a,b) ((a)<?(b))
//#define max(a,b) ((a)>?(b))
using namespace std;

struct aPair
{
    unsigned int a, b;
};

//        type,    rows,  cols
template <class T, int m, int n>
void GaussianElimination(T A[][n])
{
    const T epsilon = 1.0e-10;
    int i = 0;
    int j = 0;
    T max_val;
    int max_ind;

    while (i < m && j < n)
    {
        // Find pivot in column j, starting in row i:
        max_val = A[i][j];
        max_ind = i;

        for (int k = i + 1; k < m; k++)
        {
            T val = A[k][j];
            if ( fabs(val) > fabs(max_val) )
            {
                max_val = val;
                max_ind = k;
            }
        }

        if ( fabs(max_val) > epsilon )
        {
            // switch rows i and max_ind //but remain the values of i and max_ind
            for (int a = 0; a < n; a++)
                std::swap(A[i][a], A[max_ind][a]);

            // Now A[i, j] will contain the same value as max_val

            //divide row i by max_val
            for (int a = 0; a < n; a++)
                A[i][a] /= max_val;

            // Now A[i, j] will have the value 1
            for (int u = 0; u < m; u++)
            {
                if (u != i)
                {
                    // subtract A[u, j] * row i from row u
                    for (int a = 0; a < n; a++)
                        A[u][a] -= A[u][j] * A[i][a];

                    // Now A[u, j] will be 0, since A[u, j] - A[u, j] * A[i, j] = A[u, j] - 1 * A[u, j] = 0
                }
            }
            i++;
        }
        j++;
    }
}

void Collide(const std::vector<PhyPoint*> &listA, const std::vector<PhyPoint*> &listB,
             const REAL r2)
{
    // Note that the parameter r2 is radius^2

    // Enkel typ av kollision mellan punkter
    REAL distance, k;
    Vec3r delta;

    for (auto itA = listA.begin(); itA != listA.end(); ++itA)
    {
        for (auto itB = listB.begin(); itB != listB.end(); ++itB)
        {
            delta = (*itA)->pos - (*itB)->pos;
            //delta += ((*itA)->vel - (*itB)->vel)*0.001;
            distance = delta.SqrLength();
            if ( distance < r2 && distance > 0.001 )
            {
                k = (r2/distance-1) * 0.2;
                (*itA)->force += delta*k;
                (*itB)->force -= delta*k;
            }
        }
    }
}

inline void NarrowLists(const std::vector<PhyPoint*> &listA, const std::vector<PhyPoint*> &listB,
                        std::vector<PhyPoint*> &retListA, std::vector<PhyPoint*> &retListB,
                        const Vec3r &min, const Vec3r &max)
{
    // List of points inside bounds given by min and max
    auto it = listA.begin();
    auto end = listA.end();
    for (; it != end; ++it)
    {
        if ((*it)->pos.x < min.x || (*it)->pos.x > max.x)
            continue;
        if ((*it)->pos.y < min.y || (*it)->pos.y > max.y)
            continue;
        if ((*it)->pos.z < min.z || (*it)->pos.z > max.z)
            continue;

        retListA.push_back(*it);
    }

    it = listB.begin();
    end = listB.end();
    for (; it != end; ++it)
    {
        if ((*it)->pos.x < min.x || (*it)->pos.x > max.x)
            continue;
        if ((*it)->pos.y < min.y || (*it)->pos.y > max.y)
            continue;
        if ((*it)->pos.z < min.z || (*it)->pos.z > max.z)
            continue;

        retListB.push_back(*it);
    }


}

static int depthCounterDepth = 0;

void SplitAndCollide(const std::vector<PhyPoint*> &listA, const std::vector<PhyPoint*> &listB,
                     const REAL r, const Vec3r &min, const Vec3r &max)
{
    class DepthCounter
    {
    public:
        DepthCounter()
        {
            depthCounterDepth++;
        };
        ~DepthCounter()
        {
            depthCounterDepth--;
        }
    } depthCounter;

    // TODO: Make maximum size*size and maximum depthCounterDepth console variables and tweak them
    if (listA.size() * listB.size() < 4*4 || depthCounterDepth >= 16)
    {
        // FIXME: Is it possible for the same pair of points to collide multiple times?
        Collide(listA, listB, r*r);
        return;
    }

    Vec3r box1, box2;
    Vec3r mid = (min+max)*0.5;
    Vec3r diff = (max-min);
    if (diff.x > diff.y)
        if (diff.x > diff.z)
        {
            //X
            if (diff.x < r*5)
            {
                Collide(listA, listB, r*r);
                return;
            }
            box1 = Vec3r(mid.x+r, max.y, max.z);
            box2 = Vec3r(mid.x-r, min.y, min.z);
        }
        else
        {
            //Z
            if (diff.z < r*5)
            {
                Collide(listA, listB, r*r);
                return;
            }
            box1 = Vec3r(max.x, max.y, mid.z+r);
            box2 = Vec3r(min.x, min.y, mid.z-r);
        }
    else
        if (diff.y > diff.z)
        {
            //Y
            if (diff.y < r*5)
            {
                Collide(listA, listB, r*r);
                return;
            }
            box1 = Vec3r(max.x, mid.y+r, max.z);
            box2 = Vec3r(min.x, mid.y-r, min.z);
        }
        else
        {
            //Z
            if (diff.z < r*5)
            {
                Collide(listA, listB, r*r);
                return;
            }
            box1 = Vec3r(max.x, max.y, mid.z+r);
            box2 = Vec3r(min.x, min.y, mid.z-r);
        }

    std::vector<PhyPoint*> retListA, retListB;
    NarrowLists(listA, listB, retListA, retListB, min, box1);
    // TODO: The lists and region could be tightened here
    //       by calculating separate regions for retListA
    //       and retListB and removing the points outside the
    //       intersection
    SplitAndCollide(retListA, retListB, r, min, box1);

    retListA.clear();
    retListB.clear();
    NarrowLists(listA, listB, retListA, retListB, box2, max);
    // TODO: The lists and region could be tightened here
    SplitAndCollide(retListA, retListB, r, box2, max);
}

#ifndef MIN
#define MIN(a,b) (a<b? a: b)
#endif
#ifndef MAX
#define MAX(a,b) (a>b? a: b)
#endif

void Physics::TestBounds(Physics &a, const REAL r)
{
    // Calculate intersection
    // Note that the intersection is increased by the radius
    Vec3r min, max;

    min.x = MAX(a.bounds_min.x, bounds_min.x) - r;
    min.y = MAX(a.bounds_min.y, bounds_min.y) - r;
    min.z = MAX(a.bounds_min.z, bounds_min.z) - r;

    max.x = MIN(a.bounds_max.x, bounds_max.x) + r;
    max.y = MIN(a.bounds_max.y, bounds_max.y) + r;
    max.z = MIN(a.bounds_max.z, bounds_max.z) + r;

    if (min.x > max.x || max.x < min.x)
        return;

    if (min.y > max.y || max.y < min.y)
        return;

    if (min.z > max.z || max.z < min.z)
        return;

    profiler.RememberTime(__func__);

    // List of points inside bounds given by min and max
    std::vector<PhyPoint*> listA, listB;
    PhyPoint *it, *end;

    it = a.points;
    end = a.points + a.points_count + a.nodes_count;
    for (; it != end; ++it)
    {
        if (it->pos.x < min.x || it->pos.x > max.x)
            continue;
        if (it->pos.y < min.y || it->pos.y > max.y)
            continue;
        if (it->pos.z < min.z || it->pos.z > max.z)
            continue;

        listA.push_back(it);
    }

    it = this->points;
    end = this->points + this->points_count + this->nodes_count;
    for (; it != end; ++it)
    {
        if (it->pos.x < min.x || it->pos.x > max.x)
            continue;
        if (it->pos.y < min.y || it->pos.y > max.y)
            continue;
        if (it->pos.z < min.z || it->pos.z > max.z)
            continue;

        listB.push_back(it);
    }

    SplitAndCollide(listA, listB, r, min, max);
    profiler.RememberTime("TestBounds end");
}

void CollideTriangles(PhyPoint *a1, PhyPoint *a2, PhyPoint *a3, PhyPoint *b1, PhyPoint *b2, PhyPoint *b3)
{
    Vec3r AB(a2->pos - a1->pos);
    Vec3r AC(b1->pos - a1->pos);
    Vec3r AD(b2->pos - a1->pos);

    Vec3r A2B2(a2->pos + a2->vel - a1->pos - a1->vel);
    Vec3r A2C2(b1->pos + b1->vel - a1->pos - a1->vel);
    Vec3r A2D2(b2->pos + b2->vel - a1->pos - a1->vel);

    (AB%AC*AD) * (A2B2%A2C2*A2D2);
}

void Physics::DoCollideTriangles()
{
    /*
    for (unsigned int i = 0; i < gltriangle_indices_count/3; i++)
    {
        for (unsigned int j = i; j < gltriangle_indices_count/3; j++)
        {
            unsigned int a = 3*i;
            unsigned int b = 3*j;
            CollideTriangles(&points[gltriangle_indices[a]],
                             &points[gltriangle_indices[a+1]],
                             &points[gltriangle_indices[a+2]],
                             &points[gltriangle_indices[b]],
                             &points[gltriangle_indices[b+1]],
                             &points[gltriangle_indices[b+2]]);
        }
    }
    */
    /*
    CompareList<aPair> side(10);
    for (unsigned int i = 0; i < 10; ++i)
    {
        for (unsigned int j = 0; j < 10; ++j)
        {
            if (i!=j)
            {
                side.Pair(i,j).a  = i;
                side.Pair(i,j).b  = j;
            }
        }
    }
    */
}

void Physics::DoCollidePoints()
{
    // Enkel typ av kollision mellan punkter
    REAL k = 1000*timestep_squared;

    for(unsigned int i=0; i < points_count; i++)
    {
        for(unsigned int j=i+1; j < points_count; j++)
        {
            PhyPoint &p1 = points[i];
            PhyPoint &p2 = points[j];
            Vec3r delta = p1.pos + p1.vel - p2.pos - p2.vel;
            REAL distance = delta.SqrLength();
            if ( distance < 0.75 && distance > 0.001 )
            {
                Vec3r force = delta * ((0.1-distance)/distance*k);
                p1.force += force;
                p2.force -= force;
            }
        }
    }
}

void Physics::CollideFloor()
{
    PhyPoint *it = points;
    PhyPoint *end = points + points_count;
    while (it != end)
    {
        REAL z2 = it->pos.z;
        if ( z2 < 0 )
        {
            // tangential friction
            Vec3r delta = Vec3r(it->vel.x, it->vel.y, 0);
            REAL friction = it->vel.z * -10;

            if ( delta.SqrLength() > friction*friction )
                delta = Normalize(delta) * fabs(friction);

            it->vel -= delta;

            // Displacement
            it->vel.z -= z2;
        }
        ++it;
    }

    end = points + (points_count + nodes_count);
    PhyRigid *rigid = rigids;
    while (it != end)
    {
        while (it > rigid->points + rigid->nodes_count)
            ++rigid;

        //REAL z2 = point.pos.z + point.vel.z*timestep;
        REAL z2 = it->pos.z;
        if ( z2 < 0 )
        {
            Vec3r force(0,0,0);
            force.z -= z2*100*timestep_squared; // spring k
/*
            if (it->vel.z < 0)
                force.z -= it->vel.z*10*timestep; // spring d
*/
            // tangential friction
            Vec3r vec = it->vel/timestep;
            vec.z=0;
#if 0
            if (vec.SqrLength() < 1)
                force -= vec*120;//*z2;
            else
                force -= vec*8;//*z2;
#else
            force -= vec*0.3*force.z;
#endif

            Vec3r spin = (it->pos - rigid->pos).Cross(force).ElemMult(rigid->inv_inertia);
            force /= it->inv_mass + spin.Length();

            // * rigid->orient
            it->force += force;
        }
        ++it;
    }
}

#define A (p1.pos)
#define B (p2.pos)
#define C (p3.pos)
#define D (p4.pos)
#define Avel (p1.vel)
#define Bvel (p2.vel)
#define Cvel (p3.vel)
#define Dvel (p4.vel)

int Physics::CollideLineLine(PhyPoint &p1, PhyPoint &p2, PhyPoint &p3, PhyPoint &p4)
{
    Vec3r A2(A + Avel);
    Vec3r B2(B + Bvel);
    Vec3r AB(B - A);

    Vec3r C2(C + Cvel);
    Vec3r D2(D + Dvel);

    Vec3r AC(C - A);
    Vec3r CD(D - C);

    Vec3r EF = AB.UnitCross(CD);
    Vec3r E2F2 = (B2-A2).UnitCross(D2-C2);
    REAL distance = EF.Dot(AC);       // Avstånd mellan linjerna
    REAL distance2 = E2F2.Dot(C2-A2); // Avstånd mellan linjerna vid nästa tidssteg
    REAL AB_kollide_dist;
    REAL CD_kollide_dist;
    if (distance*distance2 > 0)
        return 0;
    // Kollision detekteras preliminärt

    AC -= EF * distance;
    REAL inversedivisor = (AB.x * CD.y - CD.x * AB.y);
    // Testa för div med noll... isf göra alternativ uträkning
    if (inversedivisor != 0)
    {
        inversedivisor = 1/inversedivisor;

        AB_kollide_dist = (CD.y * AC.x - CD.x * AC.y) * inversedivisor;
        CD_kollide_dist = (AB.y * AC.x - AB.x * AC.y) * inversedivisor;
    }
    else if (inversedivisor = (AB.x * CD.z - CD.x * AB.z) != 0)
    {
        inversedivisor = 1/inversedivisor;

        AB_kollide_dist = (CD.z * AC.x - CD.x * AC.z) * inversedivisor;
        CD_kollide_dist = (AB.z * AC.x - AB.x * AC.z) * inversedivisor;
    }
    else if (inversedivisor = (AB.z * CD.y - CD.z * AB.y) != 0)
    {
        inversedivisor = 1/inversedivisor;

        AB_kollide_dist = (CD.y * AC.z - CD.z * AC.y) * inversedivisor;
        CD_kollide_dist = (AB.y * AC.z - AB.z * AC.y) * inversedivisor;
    }
    else
        return 0;

    if (AB_kollide_dist < 0.0f)
        return 0;
    if (AB_kollide_dist > 1.0f)
        return 0;
    if (CD_kollide_dist < 0.0f)
        return 0;
    if (CD_kollide_dist > 1.0f)
        return 0;
    // Kollision konstaterad

    //kollisionsrespons
    Vec3r rel_vel(
        Cvel * (1.0f-CD_kollide_dist) +
        Dvel * CD_kollide_dist -
        Avel * (1.0f-AB_kollide_dist) -
        Bvel * AB_kollide_dist
        );
    rel_vel = ( 2.0f*(0.5f+min(CD_kollide_dist,(1.0f-CD_kollide_dist)))
        *rel_vel.Dot(EF) )*EF;

    Avel += rel_vel * (1.0f-CD_kollide_dist);
    Bvel += rel_vel * CD_kollide_dist;
    Cvel -= rel_vel * (1.0f-CD_kollide_dist);
    Dvel -= rel_vel * CD_kollide_dist;

//  char tmp[1024];
//  sprintf(tmp, "rel_vel: %2.2f, %2.2f, %2.2f", rel_vel.x, rel_vel.y, rel_vel.z);
//  text_debug->set_text(tmp);
    return 1;
}

void TraceLine(const Vec3r &pos, const Vec3r &dir, const REAL max_dist, Physics *phys, TraceResult *res)
{
    res->dist = max_dist*max_dist;
    res->point = NULL;
    REAL sqrdist, length;
    Vec3r AB;

    for(unsigned int i=0; i < phys->points_count+phys->nodes_count; ++i)
    {
        AB = phys->points[i].pos - pos;
        length = dir.Dot(AB);
        if (length < 0)
            continue;
        AB = AB - dir * length;
        sqrdist = AB.SqrLength();
        if (sqrdist < res->dist)
        {
            res->length = length;
            res->dist = sqrdist;
            res->point = &phys->points[i];
            res->delta = AB;
        }
    }

    res->dist = sqrt(res->dist);
}
/*
    REAL d0, d1, d2, d3; // statiskt plan
    REAL p0p1[3], p0p2[3], p1p2[3], normal[3], p0p1normal[3], p0p2normal[3], p1p2normal[3];
    // Skapa tre vektorer
    p0p1[0] = points[1].x-points[0].x;
    p0p1[1] = points[1].y-points[0].y;
    p0p1[2] = points[1].z-points[0].z;

    p0p2[0] = points[2].x-points[0].x;
    p0p2[1] = points[2].y-points[0].y;
    p0p2[2] = points[2].z-points[0].z;

    p1p2[0] = points[2].x-points[1].x;
    p1p2[1] = points[2].y-points[1].y;
    p1p2[2] = points[2].z-points[1].z;

    // skapa normalen till vektorerna
    CrossProduct(p0p1, p0p2, normal);
    CrossProduct(p0p1, normal, p0p1normal);
    CrossProduct(p0p2, normal, p0p2normal); // pekar utåt
    CrossProduct(p1p2, normal, p1p2normal);
    // normera
    d0 = FastSqrt(normal[0]*normal[0] + normal[1]*normal[1] + normal[2]*normal[2]);
    d1 = FastSqrt(p0p1normal[0]*p0p1normal[0] + p0p1normal[1]*p0p1normal[1] + p0p1normal[2]*p0p1normal[2]);
    d2 = FastSqrt(p0p2normal[0]*p0p2normal[0] + p0p2normal[1]*p0p2normal[1] + p0p2normal[2]*p0p2normal[2]);
    d3 = FastSqrt(p1p2normal[0]*p1p2normal[0] + p1p2normal[1]*p1p2normal[1] + p1p2normal[2]*p1p2normal[2]);
    normal[0] /= d0;
    normal[1] /= d0;
    normal[2] /= d0;
    // hitta avståndet från planet till punkt p0
    d0 = -(normal[0]*points[0].x + normal[1]*points[0].y + normal[2]*points[0].z);
    d1 = -(p0p1normal[0]*points[0].x + p0p1normal[1]*points[0].y + p0p1normal[2]*points[0].z);
    d2 = -(p0p2normal[0]*points[0].x + p0p2normal[1]*points[0].y + p0p2normal[2]*points[0].z);
    d3 = -(p1p2normal[0]*points[1].x + p1p2normal[1]*points[1].y + p1p2normal[2]*points[1].z);

    for(int i=3; i < numPoints; i++)
    {
        distance = points[i].x*normal[0] + points[i].y*normal[1] + points[i].z*normal[2] + d0;
        // på fel sida om planet?
        if ( 0.1 > distance && -0.1 < distance)
        {
            // utanför triangeln?
            if (0 < points[i].x*p0p1normal[0] + points[i].y*p0p1normal[1] + points[i].z*p0p1normal[2] + d1)
                continue;
            if (0 > points[i].x*p0p2normal[0] + points[i].y*p0p2normal[1] + points[i].z*p0p2normal[2] + d2)
                continue;
            if (0 < points[i].x*p1p2normal[0] + points[i].y*p1p2normal[1] + points[i].z*p1p2normal[2] + d3)
                continue;

            if ( distance < 0 )
                distance += 0.1;
            else
                distance -= 0.1;

            points[i].x -= distance*normal[0];
            points[i].y -= distance*normal[1];
            points[i].z -= distance*normal[2];

            distance = points[i].vx*normal[0] + points[i].vy*normal[1] + points[i].vz*normal[2];
            distance *= 1.5f;

            points[i].vx -= distance*normal[0];
            points[i].vy -= distance*normal[1];
            points[i].vz -= distance*normal[2];

            //points[i].vx=points[i].vy=points[i].vz=0;
        }
    }
*/
/*
    for(int i=0; i < numLines; i++)
    {
        for(int j=i+1; j < numLines; j++)
        {
            if (lines[i].p1 == lines[j].p1)
                continue;
            if (lines[i].p1 == lines[j].p2)
                continue;
            if (lines[i].p2 == lines[j].p1)
                continue;
            if (lines[i].p2 == lines[j].p2)
                continue;
            CollideLineLine(i,j, _time);
        }
    }
*/
