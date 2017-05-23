#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <test/testutil.h>
#include "test_mesh.h"
#include <tesselate/timer.h>
#include <stdio.h>
#include <cstdint>
#include <sstream>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

void TestMesh::setUp()
{
    mesh = new Mesh();
    voxel = new VoxelVolume();
    voxel->setDim(2,2,2);
    mySphere= new Sphere(cgp::Point(0,0,0),1.0f);
}

void TestMesh::tearDown()
{
    delete mesh;
    delete voxel;
    delete mySphere;
}

void TestMesh::testBunny()
{
    Timer t;

    t.start();
    mesh->readSTL("../meshes/bunny.stl");
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity()); // bunny has known holes in the bottom
    t.stop();
    cerr << "BUNNY TEST PASSED in " << t.peek() << "s" << endl << endl;

    t.start();
    mesh->readSTL("../meshes/dragon.stl");
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity());
    t.stop();
    cerr << "DRAGON TEST PASSED in " << t.peek() << "s" << endl << endl;
}

void TestMesh::validTetCase()
{
    Triangle t;

    mesh->clear();

    // base
    mesh->verts.push_back(cgp::Point(0.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 0.0f, 1.0f));

    // apex
    mesh->verts.push_back(cgp::Point(0.0f, 1.0f, 0.0f));

    // 4 triangles in tetrahedron
    t.v[0] = 0; t.v[1] = 1; t.v[2] = 2;    // base triangle 0-1-2
    mesh->tris.push_back(t);
    t.v[0] = 1; t.v[1] = 0; t.v[2] = 3;    // side triangle 1-0-3
    mesh->tris.push_back(t);
    t.v[0] = 2; t.v[1] = 1; t.v[2] = 3;    // side triangle 2-1-3
    mesh->tris.push_back(t);
    t.v[0] = 0; t.v[1] = 2; t.v[2] = 3;    // side triangle 0-2-3
    mesh->tris.push_back(t);
}

void TestMesh::testSimple()
{
    // test simple valid 2-manifold
    validTetCase();
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(mesh->manifoldValidity());
    cerr << "SIMPLE VALIDITY TEST PASSED" << endl << endl;
}

void TestMesh::basicBreakCase()
{
    Triangle t;

    mesh->clear();

    // duplicated dangling vertex at origin
    mesh->verts.push_back(cgp::Point(0.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(0.0f, 0.0f, 0.0f));

    // single triangle pointing to non-existent vertices
    t.v[0] = 2; t.v[1] = 3; t.v[2] = 4;
    mesh->tris.push_back(t);
}

void TestMesh::testBreak()
{
    // test for duplicate vertices, dangling vertices and out of bounds on vertex indices
    // mesh->basicBreakTest();
    basicBreakCase();
    CPPUNIT_ASSERT(!mesh->basicValidity());
    cerr << "BASIC INVALID MESH TEST PASSED" << endl << endl;
}

void TestMesh::openTetCase()
{
    Triangle t;

    mesh->clear();

    // base
    mesh->verts.push_back(cgp::Point(0.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 0.0f, 1.0f));

    // apex
    mesh->verts.push_back(cgp::Point(0.0f, 1.0f, 0.0f));

    // 4 triangles in tetrahedron
    t.v[0] = 0; t.v[1] = 1; t.v[2] = 2;    // base triangle 0-1-2
    mesh->tris.push_back(t);
    t.v[0] = 1; t.v[1] = 0; t.v[2] = 3;    // side triangle 1-0-3
    mesh->tris.push_back(t);
    t.v[0] = 2; t.v[1] = 1; t.v[2] = 3;    // side triangle 2-1-3
    mesh->tris.push_back(t);
    // closing side triangle missing
}

void TestMesh::testOpen()
{
    // test for 2-manifold with boundary
    openTetCase();
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity());
    cerr << "INVALID MESH WITH BOUNDARY TEST PASSED" << endl << endl;
}

void TestMesh::touchTetsCase()
{
    Triangle t;

    mesh->clear();

    // base
    mesh->verts.push_back(cgp::Point(0.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 0.0f, 1.0f));

    // apex
    mesh->verts.push_back(cgp::Point(0.0f, 1.0f, 0.0f));

    // upper base of inverted tet
    mesh->verts.push_back(cgp::Point(0.0f, 2.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 2.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 2.0f, 1.0f));

    // 4 triangles in lower tetrahedron
    t.v[0] = 0; t.v[1] = 1; t.v[2] = 2;    // base triangle 0-1-2
    mesh->tris.push_back(t);
    t.v[0] = 1; t.v[1] = 0; t.v[2] = 3;    // side triangle 1-0-3
    mesh->tris.push_back(t);
    t.v[0] = 2; t.v[1] = 1; t.v[2] = 3;    // side triangle 2-1-3
    mesh->tris.push_back(t);
    t.v[0] = 0; t.v[1] = 2; t.v[2] = 3;    // side triangle 0-2-3
    mesh->tris.push_back(t);

    // 4 triangles in upper tetrahedron
    t.v[0] = 6; t.v[1] = 5; t.v[2] = 4;    // base triangle 0-1-2
    mesh->tris.push_back(t);
    t.v[0] = 4; t.v[1] = 5; t.v[2] = 3;    // side triangle 1-0-3
    mesh->tris.push_back(t);
    t.v[0] = 5; t.v[1] = 6; t.v[2] = 3;    // side triangle 2-1-3
    mesh->tris.push_back(t);
    t.v[0] = 6; t.v[1] = 4; t.v[2] = 3;    // side triangle 0-2-3
    mesh->tris.push_back(t);

}

void TestMesh::testPinch()
{
    // test for non 2-manifold failure where surfaces touch at a single vertex
    touchTetsCase();
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity());
    cerr << "INVALID PINCHED SURFACE TEST PASSED" << endl << endl;
}

void TestMesh::overlapTetCase()
{
    Triangle t;

    mesh->clear();

    // base
    mesh->verts.push_back(cgp::Point(0.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 0.0f, 0.0f));
    mesh->verts.push_back(cgp::Point(1.0f, 0.0f, 1.0f));

    // apex
    mesh->verts.push_back(cgp::Point(0.0f, 1.0f, 0.0f));

    // 4 triangles in tetrahedron
    t.v[0] = 0; t.v[1] = 1; t.v[2] = 2;    // base triangle 0-1-2
    mesh->tris.push_back(t);
    mesh->tris.push_back(t);
    t.v[0] = 1; t.v[1] = 0; t.v[2] = 3;    // side triangle 1-0-3
    mesh->tris.push_back(t);
    mesh->tris.push_back(t);
    t.v[0] = 2; t.v[1] = 1; t.v[2] = 3;    // side triangle 2-1-3
    mesh->tris.push_back(t);
    mesh->tris.push_back(t);
    t.v[0] = 0; t.v[1] = 2; t.v[2] = 3;    // side triangle 0-2-3
    mesh->tris.push_back(t);
    mesh->tris.push_back(t);
}

void TestMesh::testOverlap()
{
    // test for non 2-manifold overlapping triangles
    overlapTetCase();
    CPPUNIT_ASSERT(mesh->basicValidity());
    CPPUNIT_ASSERT(!mesh->manifoldValidity());
    cerr << "INVALID NON-2-MANIFOLD TEST PASSED" << endl << endl;
}

void TestMesh::testSphere(){
	cgp::Point point;
    point.x = 0;
    point.y = 0;
    point.z = 2;
    CPPUNIT_ASSERT(!mySphere->pointContainment(point));
    
    cgp::Point point1;
    point.x = 1;
    point.y = 0;
    point.z = 1;
    CPPUNIT_ASSERT(mySphere->pointContainment(point1));
    
    cerr << "SPHERE POINT CONTAINMENT TEST PASSED" << endl;
}

void TestMesh::testFill(){
	voxel->fill(false);
	CPPUNIT_ASSERT(!voxel->get(0,0,1));
	CPPUNIT_ASSERT(!voxel->get(1,1,1));
	CPPUNIT_ASSERT(!voxel->get(2,2,2));
	cerr << "FILL TEST PASSED" << endl;
}

void TestMesh::testSetandGet(){
	voxel->fill(true);
	voxel->set(0,0,0,true);
	//cerr << voxel->get(0,0,0) << endl;
	CPPUNIT_ASSERT(voxel->get(0,0,0));
	voxel->set(0,0,1,false);
	//cerr << voxel->get(0,0,1) << endl;
	CPPUNIT_ASSERT(!voxel->get(0,0,1));
	CPPUNIT_ASSERT(!voxel->set(3,2,2,true));
	cerr << "SET AND GET TEST PASSED" << endl;
}


//#if 0 /* Disabled since it crashes the whole test suite */
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TestMesh, TestSet::perBuild());
//#endif
