#include "physics.h"
#include "console.h"

#include <windows.h>

void Physics::DoFrame(double p_time)
{
    //App::console << "entering DoFrame\n";
    m_time = p_time;

    LARGE_INTEGER freq, t;
    std::vector<long long int> times;

    ::QueryPerformanceFrequency(&freq);
#define REMEMBER_TIME \
    ::QueryPerformanceCounter(&t); \
    times.push_back(t.QuadPart);

    REMEMBER_TIME
    App::console << "before DoFrame0(rigids)\n";

    // uppdatera noderna
    for(unsigned int i = 0; i < m_rigids_count; i++)
        DoFrame0(m_rigids[i]);

    App::console << "before DoFrame1(springs)\n";
    REMEMBER_TIME
    // räkna ut linjernas krafter på deras punkter
    //assert(m_springs != 0 || m_springs_count == 0);
    for(unsigned int i = 0; i < m_springs_count; i++)
        DoFrame1(m_springs[i]);

    App::console << "before DoFrame1(angles)\n";
    REMEMBER_TIME
    // räkna ut vinklarnas krafter på deras punkter
    for(unsigned int i = 0; i < m_angles_count; i++)
        DoFrame1(m_angles[i]);

    App::console << "before DoFrame1(joints)\n";
    REMEMBER_TIME
    // håll ihop saker
    for(unsigned int i = 0; i < m_joints_count; i++)
        DoFrame1(m_joints[i]);

    App::console << "before DoFrame1(tetras)\n";
    REMEMBER_TIME
	// Räkna ut tetrahedrornas tryck och dess kraft på deras punkter
    for(unsigned int i = 0; i < m_tetras_count; i++)
        DoFrame1(m_tetras[i]);

    App::console << "before CollideFloor()\n";
    REMEMBER_TIME
    // kollidera med markplanet
    for(unsigned int i=0; i < m_points_count+m_nodes_count; i++)
        CollideFloor(m_points[i]);

    App::console << "before DoFrame1(points)\n";
    REMEMBER_TIME
    // räkna ut acceleration och hastighet (och gravitation)
    for(unsigned int i = 0; i < m_points_count; i++)
        DoFrame1(m_points[i]);

    App::console << "before DoFrame1(rigids)\n";
    REMEMBER_TIME
    // räkna ut acceleration och hastighet (och gravitation)
    for(unsigned int i = 0; i < m_rigids_count; i++)
        DoFrame1(m_rigids[i]);

    App::console << "before DoFrame2(points)\n";
    REMEMBER_TIME
    // räkna ut position
    for(unsigned int i = 0; i < m_points_count; i++)
        DoFrame2(m_points[i]);

    App::console << "before DoFrame2(rigids)\n";
    REMEMBER_TIME
    // räkna ut position
    for(unsigned int i = 0; i < m_rigids_count; i++)
        DoFrame2(m_rigids[i]);

    App::console << "after DoFrame2(rigids)\n";
    REMEMBER_TIME

    App::console << "Timings from Physics::DoFrame():" << std::endl;
    for ( unsigned int i = 1; i < times.size(); ++i )
    {
        App::console << "  span #" << i << ": " <<
            static_cast<double>( times[i]-times[i-1] ) / static_cast<double>(freq.QuadPart)*1000.0
            << " ms" << std::endl;
    }
    App::console << "  total: " <<
        static_cast<double>( times[times.size()-1]-times[0] ) / static_cast<double>(freq.QuadPart)*1000.0
        << " ms" << std::endl;
/*
    for(int i=0; i < m_numPoints; i++)
        CollideBorder(m_points[i]);
*/
    App::console << "end of Physics::DoFrame()\n";

#undef REMEMBER_TIME
}
//---------------------------------------------------------------------------
/*
void Physics::SaveTo(const char p_filename[])
{
	ofstream file(p_filename);
	file << m_numPoints << endl
		<< m_numLines << endl
    	<< m_numAngles << endl
    	<< m_numJoints << endl
    	<< m_numTriangles << endl
    	<< m_numTetras << endl;

	file <<  m_gravity << endl;
		//<< m_borderMin << endl
		//<< m_borderMax << endl;

	for (int i=0; i<m_numPoints; i++)
	{
    	file << m_points[i].pos << endl
	    	<< m_points[i].vel << endl
	    	<< m_points[i].m << endl;
	}
	for (int i=0; i<m_numLines; i++)
	{
    	file << int(m_springs[i].p1-m_points) << endl
	    	<< int(m_springs[i].p2-m_points) << endl
	    	<< m_springs[i].l << endl
	    	<< m_springs[i].k << endl
	    	<< m_springs[i].d << endl
	    	<< m_springs[i].s << endl;
	}
	for (int i=0; i<m_numAngles; i++)
	{
    	file << int(m_angles[i].p1-m_points) << endl
	    	<< int(m_angles[i].p2-m_points) << endl
	    	<< int(m_angles[i].p3-m_points) << endl
	    	<< int(m_angles[i].l1-m_springs) << endl
	    	<< int(m_angles[i].l2-m_springs) << endl
	    	<< m_angles[i].a << endl
	    	<< m_angles[i].k << endl
	    	<< m_angles[i].d << endl;
	}
	for (int i=0; i<m_numJoints; i++)
	{
    	file << int(m_joints[i].p1-m_points) << endl
	    	<< int(m_joints[i].p2-m_points) << endl
	    	<< m_joints[i].maxforce << endl;
	}
	for (int i=0; i<m_numTriangles; i++)
	{
    	file << int(m_triangles[i].p1-m_points) << endl
	    	<< int(m_triangles[i].p2-m_points) << endl
	    	<< int(m_triangles[i].p3-m_points) << endl
	    	<< m_triangles[i].k << endl
	    	<< m_triangles[i].d << endl
	    	<< m_triangles[i].s << endl;
	}
	for (int i=0; i<m_numTetras; i++)
	{
    	file << int(m_tetras[i].p1-m_points) << endl
	    	<< int(m_tetras[i].p2-m_points) << endl
	    	<< int(m_tetras[i].p3-m_points) << endl
	    	<< int(m_tetras[i].p4-m_points) << endl
	    	<< m_tetras[i].nRT << endl;
	}
}

void Physics::LoadFrom(const char p_filename[])
{
	ifstream file(p_filename);

	if (!file.good())
	{
        m_numPoints = 0;
        m_numLines = 0;
        m_numAngles = 0;
        m_numJoints = 0;
        m_numTriangles = 0;
        m_numTetras = 0;
        cout << "file \"" << p_filename << "\" is not good\n";
	    return;
	}

	file >> m_numPoints
		>> m_numLines
    	>> m_numAngles
    	>> m_numJoints
    	>> m_numTriangles
    	>> m_numTetras;

	file >>  m_gravity;// >> m_borderMin >> m_borderMax;

    int p1, p2, p3, p4, l1, l2;

	for (int i=0; i<m_numPoints; i++)
	{
    	file >> m_points[i].pos
	    	>> m_points[i].vel
	    	>> m_points[i].m;
	}
	for (int i=0; i<m_numLines; i++)
	{
    	file >> p1
	    	>> p2
	    	>> m_springs[i].l
	    	>> m_springs[i].k
	    	>> m_springs[i].d
	    	>> m_springs[i].s;
        m_springs[i].p1 = &m_points[p1];
        m_springs[i].p2 = &m_points[p2];
	}
	for (int i=0; i<m_numAngles; i++)
	{
    	file >> p1
	    	>> p2
	    	>> p3
	    	>> l1
	    	>> l2
	    	>> m_angles[i].a
	    	>> m_angles[i].k
	    	>> m_angles[i].d;
        m_angles[i].p1 = &m_points[p1];
        m_angles[i].p2 = &m_points[p2];
        m_angles[i].p3 = &m_points[p3];
        m_angles[i].l1 = &m_springs[l1];
        m_angles[i].l2 = &m_springs[l2];
	}
	for (int i=0; i<m_numJoints; i++)
	{
    	file >> p1
	    	>> p2
	    	>> m_joints[i].maxforce;
        m_joints[i].p1 = m_points + p1;
        m_joints[i].p2 = m_points + p2;
	}
	for (int i=0; i<m_numTriangles; i++)
	{
    	file >> p1
	    	>> p2
	    	>> p3
	    	>> m_triangles[i].k
	    	>> m_triangles[i].d
	    	>> m_triangles[i].s;
        m_triangles[i].p1 = &m_points[p1];
        m_triangles[i].p2 = &m_points[p2];
        m_triangles[i].p3 = &m_points[p3];
	}
	for (int i=0; i<m_numTetras; i++)
	{
    	file >> p1
	    	>> p2
	    	>> p3
	    	>> p4
	    	>> m_tetras[i].nRT;
	    m_tetras[i].p1 = &m_points[p1];
	    m_tetras[i].p2 = &m_points[p2];
	    m_tetras[i].p3 = &m_points[p3];
	    m_tetras[i].p4 = &m_points[p4];
	}
}
*/
inline void Physics::DoFrame1(PhyPoint &p_point)
{
    // uppdatera hastighet
    if (p_point.mass == std::numeric_limits<REAL>::infinity())
    {
    	p_point.force.SetToZero();
        return;
	}
    p_point.vel += m_time * ( p_point.force / p_point.mass + m_gravity );
	p_point.vel *= 0.99999;

    // nollställ krafter
    p_point.force.SetToZero();
}

inline void Physics::DoFrame2(PhyPoint &p_point)
{
    // uppdatera position
    p_point.pos += m_time * p_point.vel;
}

inline void Physics::DoFrame1(PhySpring &p_spring)
{
    //App::console << "entering DoFrame1(spring)" << std::endl;
    if (p_spring.p1==p_spring.p2)
    {
        App::console << "p1 == p2 == " << p_spring.p1 - m_points << std::endl;
    }
/*
    if ( p_spring.maxforce==0 && p_spring.minforce==0 )
        return;
*/
	REAL tmp4;

    // skillnaden i x, y och z
    Vec3r tmp1(p_spring.p2->pos - p_spring.p1->pos);

    // avståndet mellan punkterna
    p_spring.rl = tmp1.Length();

    // hur stor kraft som ska korrigera felet
    tmp4 = (p_spring.rl-p_spring.l)*p_spring.k;

    REAL tmp5 = p_spring.s * sqrt(fabs(p_spring.rl - p_spring.l));
    if (p_spring.rl > p_spring.l)
        tmp4 += tmp5;
    else if (p_spring.rl < p_spring.l)
        tmp4 -= tmp5;

    //App::console << "middle of DoFrame1(spring)" << std::endl;
	/*
    if (tmp4 > p_spring.maxforce)
        tmp4 = p_spring.maxforce;
    if (tmp4 < p_spring.minforce)
        tmp4 = p_spring.minforce;
	*/

    // lägg till kraften efter normaliserad riktning
    // undvik division med 0
    if ( 0 == p_spring.rl )
    {
        //App::console << "premature return DoFrame1(spring)" << std::endl;
        return;
    }

	Vec3r tmp2(p_spring.p2->vel - p_spring.p1->vel);

    p_spring.p1->force += tmp1/p_spring.rl * tmp4;
    p_spring.p1->force += tmp2*tmp1/tmp1.SqrLength()*tmp1 * p_spring.d; // projektionen av hastigheten på fjädern

    p_spring.p2->force -= tmp1/p_spring.rl * tmp4;
    p_spring.p2->force -= tmp2*tmp1/tmp1.SqrLength()*tmp1 * p_spring.d; // projektionen av hastigheten på fjädern
    //App::console << "exiting DoFrame1(spring)" << std::endl;
}

inline void Physics::DoFrame1(PhyAngle &p_angle)
{
    if ( 0==p_angle.l1->rl || 0==p_angle.l2->rl )
        return;

    //REAL A[3], B[3], C[3], D[3], E[3];

	Vec3r A(p_angle.p1->pos - p_angle.p2->pos);
	Vec3r B(p_angle.p3->pos - p_angle.p2->pos);

    A /= p_angle.l1->rl;
    B /= p_angle.l2->rl;

	REAL tmp1, tmp4;

    tmp1=p_angle.ra;
    p_angle.ra = acos(A*B);
    // Om inre produkten ovan "råkar" bli större än ett går det åt h-vete
    // eftersom acos då returnarar -1.#IND dvs not-a-number.
    // Detta icke-tal sprider sig sedan och "äter upp" allt som är anslutet.
    // därför skyddar vi oss med följande if-sats
    if (p_angle.ra != p_angle.ra)
        p_angle.ra = M_PI;

    tmp1 = p_angle.ra - tmp1;

    tmp4 = p_angle.ra - p_angle.a;

    // Vektor C är normalen till vinkelns plan
    // D är kraftriktningen på punkt1, E på punkt3
    Vec3r C(A % B);
    Vec3r D(C % A);
    Vec3r E(B % C);

    REAL length=D.Length();
    if (length)
    {
        D /= length;
    }

    length=E.Length();
    if (length)
    {
        E /= length;
    }

    tmp1 = (D*(p_angle.p1->vel-p_angle.p2->vel)) /  p_angle.l1->rl -
		(E*(p_angle.p3->vel-p_angle.p2->vel)) / p_angle.l2->rl;

    D*=(tmp4*p_angle.k + tmp1*p_angle.d) / p_angle.l1->rl;
    E*=(tmp4*p_angle.k + tmp1*p_angle.d) / p_angle.l2->rl;

    p_angle.p1->force += D;
    p_angle.p2->force -= D+E;
    p_angle.p3->force += E;
}

inline void Physics::DoFrame1(PhyJoint &p_joint)
{
    // skillnaden i position och hastighet
    Vec3r tmp1(p_joint.p2->pos - p_joint.p1->pos);
    Vec3r tmp2(p_joint.p2->vel - p_joint.p1->vel);

    if (p_joint.s)
    {
        Vec3r tmp3(tmp1);
        tmp3.Normalize();
        if (tmp3.x != 0 || tmp3.y != 0 || tmp3.z != 0)
        {
            tmp3 *= p_joint.s;
            p_joint.p1->force += tmp3;
        	p_joint.p2->force -= tmp3;
        }
    }

    REAL length = tmp1.Length();
    if (length < 0.0005)
    {
        REAL tmp5 = p_joint.s / sqrt(length);
        Vec3r tmp3(tmp1);
        tmp3 *= tmp5;
        p_joint.p1->force += tmp3;
        p_joint.p2->force -= tmp3;
    }
	//tmp1 = p_joint.p2->force + p_joint.p1->force;
    tmp1 *= p_joint.k;
    tmp2 *= p_joint.d;

    p_joint.p1->force += tmp1;
	p_joint.p2->force -= tmp1;

    p_joint.p1->force += tmp2;
	p_joint.p2->force -= tmp2;
}

inline void Physics::DoFrame1(PhyTriangle &p_triangle)
{
	Vec3r A = p_triangle.p2->pos - p_triangle.p1->pos;
	Vec3r B = p_triangle.p3->pos - p_triangle.p2->pos;
	Vec3r C = p_triangle.p1->pos - p_triangle.p3->pos;

	p_triangle.n = A.Cross(B);

	Vec3r Na = A.UnitCross(p_triangle.n);
	Vec3r Nb = B.UnitCross(p_triangle.n);
	Vec3r Nc = C.UnitCross(p_triangle.n);
/*
	p2->force += BC;
	p3->force += CA;
	p1->force -= AB+BC+CA;
*/
}

inline void Physics::DoFrame1(PhyTetra &p_tetra)
{
	Vec3r A = p_tetra.p2->pos - p_tetra.p1->pos;
	Vec3r B = p_tetra.p3->pos - p_tetra.p1->pos;
	Vec3r C = p_tetra.p4->pos - p_tetra.p1->pos;

	p_tetra.V = (A % B) * C;

	// allmänna gaslagen: p*V = n*R*T
	REAL p = p_tetra.nRT/p_tetra.V - 1; // inre tryck minus yttre tryck
	if (p_tetra.V < 0)
		p = -p;

	if (p == 0 || p == -std::numeric_limits<REAL>::infinity())
		return;

	Vec3r AB = (A % B)*p;
	Vec3r BC = (B % C)*p;
	Vec3r CA = (C % A)*p;

	p_tetra.p4->force += AB;
	p_tetra.p2->force += BC;
	p_tetra.p3->force += CA;
	p_tetra.p1->force -= AB+BC+CA;
}

inline void Physics::DoFrame0(PhyRigid &p_rigid)
{
	Mat3x3r rotationMatrix(p_rigid.rot);

	for (int i=0; i<p_rigid.nodes_count; i++)
	{
		p_rigid.points[i].pos = rotationMatrix * p_rigid.nodes[i].pos; // glöm inte  att addera m_pos
		p_rigid.points[i].vel = p_rigid.vel + (p_rigid.spin).Cross(p_rigid.points[i].pos);
		p_rigid.points[i].pos += p_rigid.pos; // glöm inte  att addera m_pos
	}
}

inline void Physics::DoFrame1(PhyRigid &p_rigid)
{
	for (int i=0; i<p_rigid.nodes_count; i++)
	{
        p_rigid.force += p_rigid.points[i].force;
        p_rigid.torque += (p_rigid.points[i].pos-p_rigid.pos).Cross(p_rigid.points[i].force);
		p_rigid.points[i].force.SetToZero();
	}
	p_rigid.vel += (p_rigid.force/p_rigid.mass + m_gravity) * m_time;
	p_rigid.spin += p_rigid.torque * (m_time / p_rigid.inertia);
	p_rigid.force.SetToZero();
	p_rigid.torque.SetToZero();
}

inline void Physics::DoFrame2(PhyRigid &p_rigid)
{
	p_rigid.pos += p_rigid.vel * m_time;
	if (p_rigid.spin != p_rigid.spin)
	{
	    App::console << "ERROR: Rigid with NaN value in spin, setting to zero" << std::endl;
	    p_rigid.spin.SetToZero();
        return;
	}
	Quat4r delta_rot = (0.5 * Quat4r(0, p_rigid.spin.x, p_rigid.spin.y, p_rigid.spin.z)) * p_rigid.rot;
	p_rigid.rot += (delta_rot * m_time);
	//App::console << "DoFrame2(rigids)" << std::endl;
    App::console << "  spin: " << p_rigid.spin << std::endl;
    //App::console << "  delta_rot: " << delta_rot << std::endl;
    //App::console << "  quat: " << p_rigid.rot << std::endl;
	p_rigid.rot.Normalize();
    //App::console << "  quat: " << p_rigid.rot << std::endl;
}

void Physics::RecalculateNormals()
{
    for (unsigned int i=0; i < m_points_count + m_nodes_count; i++)
        m_points[i].glnormal.SetToZero();

    Vec3r normal;
    for (unsigned int i=0; i < m_gltriangle_indices_count; i+=3)
    {
        /*
        assert(m_gltriangle_indices[i] < m_points_count + m_nodes_count);
        assert(m_gltriangle_indices[i+1] < m_points_count + m_nodes_count);
        assert(m_gltriangle_indices[i+2] < m_points_count + m_nodes_count);
        */
        normal = (m_points[m_gltriangle_indices[i+1]].pos - m_points[m_gltriangle_indices[i]].pos).Cross(
            m_points[m_gltriangle_indices[i+2]].pos - m_points[m_gltriangle_indices[i]].pos);
        m_points[m_gltriangle_indices[i]].glnormal += normal;
        m_points[m_gltriangle_indices[i+1]].glnormal += normal;
        m_points[m_gltriangle_indices[i+2]].glnormal += normal;
    }

    for (unsigned int i=0; i < m_glquad_indices_count; i+=4)
    {
        /*
        assert(m_glquad_indices[i] < m_points_count + m_nodes_count);
        assert(m_glquad_indices[i+1] < m_points_count + m_nodes_count);
        assert(m_glquad_indices[i+2] < m_points_count + m_nodes_count);
        assert(m_glquad_indices[i+3] < m_points_count + m_nodes_count);
        */
        normal = (m_points[m_glquad_indices[i+2]].pos - m_points[m_glquad_indices[i]].pos).Cross(
            m_points[m_glquad_indices[i+3]].pos - m_points[m_glquad_indices[i+1]].pos);
        m_points[m_glquad_indices[i]].glnormal += normal;
        m_points[m_glquad_indices[i+1]].glnormal += normal;
        m_points[m_glquad_indices[i+2]].glnormal += normal;
        m_points[m_glquad_indices[i+3]].glnormal += normal;
    }

    /*
    for (unsigned int i=0; i < m_points_count + m_nodes_count; i++)
        m_points[i].glnormal.Normalize();
    */
}
