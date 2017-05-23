/**
 * @file
 *
 * Data structure representing a triangle mesh in 3D space.
 */

#ifndef _MESH
#define _MESH

#include <vector>
#include <stdio.h>
#include <iostream>
#include "renderer.h"

using namespace std;

const int sphperdim = 20;

/**
 * A triangle in 3D space, with 3 indices into a vertex list and an outward facing normal. Triangle winding is counterclockwise.
 */
struct Triangle
{
    int v[3];   ///< index into the vertex list for triangle vertices
    cgp::Vector n;   ///< outward facing unit normal to the triangle
};

/**
 * An edge in 3D space, with 2 indices into a vertex list. The order of the vertices may have significance.
 */
struct Edge
{
    int v[2];   ///< indices into the vertex list for edge endpoints
};

/**
 * Abstract base class for shapes
 */
class BaseShape
{
public:

    /// virtual destructor
    virtual ~BaseShape(){}

    /**
     * Generate geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    virtual void genGeometry(ShapeGeometry * geom, View * view)=0;

    /**
     * Test whether a point falls inside the shape. Will need to be overridden by each inheriting class.
     * @param pnt   point to test for containment
     * @retval true if the point falls within the shape, 
     * @retval false otherwise
     */
    virtual bool pointContainment(cgp::Point pnt)=0;
};

/**
 * A sphere in 3D space, consisting of a center and radius. Used for bounding sphere hierarchy acceleration.
 */
class Sphere: public BaseShape
{
public:
    cgp::Point c;  ///< sphere center
    float r;       ///< sphere radius
    std::vector<int> ind; ///< triangle vertex indices included in the bounding sphere. Used for acceleration struct, otherwise ignored

    /// Default Constructor
    Sphere()
    {
        c = cgp::Point(0.0f, 0.0f, 0.0f);
        r = 0.0f;
    }

    /**
     * Constructor
     * @param center    sphere center
     * @param radius    sphere radius
     */
    Sphere(cgp::Point center, float radius)
    {
        c = center;
        r = radius;
    }

   /**
     * Generate sphere geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    void genGeometry(ShapeGeometry * geom, View * view);

    /**
     * Test whether a point falls inside the sphere
     * @param pnt   point to test for containment
     * @retval true if the point falls within the sphere,
     * @retval false otherwise
     * @todo Sphere::pointContainment to be completed for CGP Assignment2
     */
    bool pointContainment(cgp::Point pnt);

};

/**
 * A cylinder in 3D space,
 */
class Cylinder: public BaseShape
{
public:
    cgp::Point s, e;   ///< start and end points of the cylinder
    float r;        ///< cylinder radius

    /// Default Constructor
    Cylinder()
    {
        s = cgp::Point(0.0f, 0.0f, 0.0f);
        e = s;
        r = 0.0f;
    }

    /**
     * Constructor
     * @param start     start vertex of spine of cylinder
     * @param end       end vertex of spine of cylinder
     * @param radius    cylinder radius
     */
    Cylinder(cgp::Point start, cgp::Point end, float radius)
    {
        s = start;
        e = end;
        r = radius;
    }

   /**
     * Generate cylinder geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    void genGeometry(ShapeGeometry * geom, View * view);

    /**
     * Test whether a point falls inside the cylinder
     * @param pnt   point to test for containment
     * @retval true if the point falls within the sphere, 
     * @retval false otherwise
     */
    bool pointContainment(cgp::Point pnt);
};

class TestMesh;

/**
 * A triangle mesh in 3D space. Ideally this should represent a closed 2-manifold but there are validity tests to ensure this.
 */
class Mesh: public BaseShape
{
private:

    friend class TestMesh;
    std::vector<cgp::Point> verts; ///< vertices of the tesselation structure
    std::vector<cgp::Vector> norms;  ///< per vertex normals
    std::vector<Triangle> tris; ///< triangles that join to make up the mesh
    GLfloat * col;              ///< (r,g,b,a) colour
    float scale;                ///< scaling factor
    cgp::Vector trx;                 ///< translation
    float xrot, yrot, zrot;     ///< rotation angles about x, y, and z axes
    std::vector<Sphere> boundspheres; ///< bounding sphere accel structure

    /**
     * Search list of vertices to find matching point
     * @param pnt       point to search for in vertex list
     * @param[out] idx  index of point in list if found, otherwise -1
     * @retval true  if the point is found in the vertex list,
     * @retval false otherwise
     */
    bool findVert(cgp::Point pnt, int &idx);

    /**
     * Construct a hash key based on a 3D point
     * @param pnt   point to convert to key
     * @param bbox  bounding box enclosing all mesh vertices
     * @retval hash key
     */
    long hashVert(cgp::Point pnt, cgp::BoundBox bbox);

    /**
     * Construct a hash key based on the indices of an edge
     * @param v0    first endpoint index
     * @param v1    second endpoint index
     * @retval hash key
     */
    long hashEdge(int v0, int v1);

    /// Connect triangles together by merging duplicate vertices
    void mergeVerts();

    /// Generate vertex normals by averaging normals of the surrounding faces
    void deriveVertNorms();

    /// Generate face normals from triangle vertex positions
    void deriveFaceNorms();

    /**
     * Composite rotations, translation and scaling into a single transformation matrix
     * @param tfm   composited transformation matrix
     */
    void buildTransform(glm::mat4x4 &tfm);

    /**
     * Create bounding sphere acceleration structure for mesh
     * @param maxspheres    the number of spheres placed along the longest side of the bounding volume
     */
    void buildSphereAccel(int maxspheres);

    /**
     * Compare two Triangles to see if they index the same vertices
     * @param t1    first triangle
     * @param t2    second triangle
     * @return true if the triangles are the same
     * @return false otherwise
     */
    bool sameTriangle(Triangle t1, Triangle t2);

    /**
     * Compare two Edges to see if they index the same vertices
     * @param e1    first edge
     * @param e2    second edge
     * @param[out] opposite true if the edges traverse in opposite directions
     * @return true if the edges index the same vertices
     * @return false otherwise
     */
    bool sameEdge(Edge e1, Edge e2, bool & opposite);

public:

    ShapeGeometry geometry;         ///< renderable version of mesh

    /// Default constructor
    Mesh();

    /// Destructor
    virtual ~Mesh();

    /// Remove all vertices and triangles, resetting the structure
    void clear();

    /// Test whether mesh is empty of any geometry (true if empty, false otherwise)
    bool empty(){ return verts.empty(); }

    /// Setter for scale
    void setScale(float scf){ scale = scf; }

    /// Getter for scale
    float getScale(){ return scale; }

    /// Setter for translation
    void setTranslation(cgp::Vector tvec){ trx = tvec; }

    /// Getter for translation
    cgp::Vector getTranslation(){ return trx; }

    /// Setter for rotation angles
    void setRotations(float ax, float ay, float az){ xrot = ax; yrot = ay; zrot = az; }

    /// Getter for rotation angles
    void getRotations(float &ax, float &ay, float &az){ ax = xrot; ay = yrot; az = zrot; }

    /// Setter for colour
    void setColour(GLfloat * setcol){ col = setcol; }

    /**
     * Generate and bind triangle mesh geometry for OpenGL rendering
     * @param view      current view parameters
     * @param[out] sdd  openGL parameters required to draw this geometry
     * @retval @c true  if buffers are bound successfully, in which case sdd is valid
     * @retval @c false otherwise
     */
    bool bindGeometry(View * view, ShapeDrawData &sdd);

    /**
     * Generate triangle mesh geometry for OpenGL rendering
     * @param[out] geom triangle-mesh geometry packed for OpenGL
     * @param view      current view parameters
     */
    void genGeometry(ShapeGeometry * geom, View * view);

    /**
     * Test whether a point falls inside the mesh using ray-mesh intersection tests
     * @param pnt   point to test for containment
     * @retval true if the point falls within the mesh, 
     * @retval false otherwise
     */
    bool pointContainment(cgp::Point pnt);

    /**
     * Scale geometry to fit bounding cube centered at origin
     * @param sidelen   length of one side of the bounding cube
     */
    void boxFit(float sidelen);

    /**
     * Read in triangle mesh from STL format binary file
     * @param filename  name of file to load (STL format)
     * @retval true  if load succeeds,
     * @retval false otherwise.
     */
    bool readSTL(string filename);

    /**
     * Write triangle mesh to STL format binary file
     * @param filename  name of file to save (STL format)
     * @retval true  if save succeeds,
     * @retval false otherwise.
     */
    bool writeSTL(string filename);

    /**
     * Basic mesh validity tests - report euler's characteristic, no dangling vertices, edge indices within bounds of the vertex list
     * @retval true if basic validity tests are passed,
     * @retval false otherwise
     * @todo basicValidity requires completing for CGP Prac1
     */
    bool basicValidity();

    /**
     * Check that the mesh is a closed two-manifold - every edge has two incident triangles, every vertex has
     *                                                a closed ring of triangles around it
     * This test does not include self-intersection of individual triangles as this is outside the scope.
     * @retval true if the mesh is two-manifold,
     * @retval false otherwise
     * @todo manifoldValidity requires completing for CGP Prac1
     */
    bool manifoldValidity();
};

#endif
