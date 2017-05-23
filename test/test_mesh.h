#ifndef TILER_TEST_MESH_H
#define TILER_TEST_MESH_H


#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "tesselate/mesh.h"
#include "tesselate/voxels.h"

/// Test code for @ref Mesh
class TestMesh : public CppUnit::TestFixture
{

    CPPUNIT_TEST_SUITE(TestMesh);
    CPPUNIT_TEST(testBunny);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST(testBreak);
    CPPUNIT_TEST(testOpen);
    CPPUNIT_TEST(testPinch);
    CPPUNIT_TEST(testOverlap);
    CPPUNIT_TEST(testSphere);
    CPPUNIT_TEST(testFill);
    CPPUNIT_TEST(testSetandGet);
    CPPUNIT_TEST_SUITE_END();

private:
    Mesh * mesh;
    VoxelVolume * voxel;
    Sphere * mySphere;

public:

    /// Initialization before unit tests
    void setUp();

    /// Tidying up after unit tests
    void tearDown();
	
    /** 
     * Run standard validity tests on bunny mesh
     * @pre bunny.stl must be located in the project root directory
     */
    void testBunny();

    /**
     * Build a simple valid 2-manifold tetrahedron with correct winding
     */
    void validTetCase();

    /**
     * Run standard validity tests for simple 2-manifold tetrahedron
     */
    void testSimple();

    /**
     * Build a simple mesh that will break basicValidity, with duplicate and dangling vertices and out of bound indices.
     */
    void basicBreakCase();

    /**
     * Run standard validity tests for tetrahedron with basic flaws
     */
    void testBreak();

    /**
     * Build a simple 2-manifold partial tetrahedron with boundary
     */
    void openTetCase();

    /**
     * Run standard validity tests for tetrahedron with boundary
     */
    void testOpen();

    /**
     * Build two simple tetrahedra that touch at a single vertex
     */
    void touchTetsCase();

    /**
     * Run standard validity tests for two tetrahedrons joined at a single vertex
     */
    void testPinch();

    /**
     * Build a simple tetrahedron that has a double shell breaking 2-manifold validity
     */
    void overlapTetCase();

    /**
     * Run standard validity tests for a tetrahedron with a double coincident shell
     */
    void testOverlap();
    
    void testSetandGet();
    
    void testFill();
    
    void testSphere();
};

#endif /* !TILER_TEST_MESH_H */
