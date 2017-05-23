#ifndef _shape_h
#define _shape_h
/**
 * @file
 *
 * ShapeGeometry class for rendering shapes in triangle mesh format
 */

#include "view.h"

/**
 * Container for rendering properties, primarily colour
 */
struct ShapeDrawData
{
    GLuint VAO;             ///< vertex array object id
    GLfloat diffuse[4];     ///< diffuse colour
    GLfloat specular[4];    ///< specular colour
    GLfloat ambient[4];     ///< ambient colour
    GLuint indexBufSize;    ///< index buffer size - as required by DrawElements
    bool   current;         ///< set to true is this is part of current manipulator
    GLuint texID;           ///< texture ID
};

/**
 * Geometry in a format suitable for OpenGL
 */
class ShapeGeometry
{
private:
    std::vector<float> verts;               ///< vertex, texture and normal data
    std::vector<unsigned int> indices;      ///< vertex indices for triangles
    GLuint vaoGeom, vboGeom, iboGeom;       ///< openGL handle for various buffers
    GLfloat diffuse[4], ambient[4], specular[4]; ///< material properties

    /**
     * Create a sphere vertex at specified integer latitude and longitude with a transformation matrix applied and append to existing geometry
     * @param radius    radius of sphere
     * @param lat       latitude as proportion
     * @param lon       longitude as proportion
     * @param trm       model transformation matrix
     */
    void genSphereVert(float radius, float lat, float lon, glm::mat4x4 trm);

public:

    /// default constructor
    ShapeGeometry()
    {
        vaoGeom = 0;
        vboGeom = 0;
        iboGeom = 0;

        // default colour
        diffuse[0] = 0.325f; diffuse[1] = 0.235f; diffuse[3] = diffuse[2] = 1.0f;
    }

    /// destructor
    ~ShapeGeometry()
    {
        clear();
    }

    /// Clear internal data structures
    void clear()
    {
        verts.clear();
        indices.clear();
    }

    /// Getter for shape colour
    GLfloat * getColour(){ return diffuse; }

    /// Setter for shape colour
    void setColour(GLfloat * col);

    /**
     * Create an uncapped cylinder originally lying along the positive z-axis and append to existing geometry
     * @param radius      radius of cylinder
     * @param height    length of cylinder
     * @param slices    number of arcs subdividing the cylinder circle
     * @param stacks    number of subdivision along the z-axis
     * @param trm       model transformation matrix
     */
    void genCylinder(float radius, float height, int slices, int stacks, glm::mat4x4 trm);
    
    /**
     * Create a sphere with a transformation matrix applied and append to existing geometry
     * @param radius    radius of sphere
     * @param slices    number of azimuth subdivisions
     * @param stacks    number of elevation subdivisions
     * @param trm       model transformation matrix
     */
    void genSphere(float radius, int slices, int stacks, glm::mat4x4 trm);

    /**
     * Convert a mesh structure to openGL geometry
     * @param points    list of vertices
     * @param norms     list of vertex normals
     * @param faces     flattened list of vertex indices, with each group of 3 indices representing a triangle
     * @param trm       model transformation matrix
     */
    void genMesh(std::vector<cgp::Point> * points, std::vector<cgp::Vector> * norms, std::vector<int> * faces, glm::mat4x4 trm);

    /**
     * Return data required for a draw call, such as the VAO, colour, etc.
     */
    ShapeDrawData getDrawParameters();

    /**
     * Bind the appropriate OpenGL buffers for rendering the constraint shape. Only needs to be done if
     * the shape changes.
     * @param view      current viewpoint
     * @retval true if buffers successfully bound
     */
    bool bindBuffers(View * view);
};
#endif
