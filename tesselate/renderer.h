#ifndef RENDERER_H
#define RENDERER_H

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include "glheaders.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <common/map.h>
#include <common/debug_string.h>
#include "shaderProgram.h"
#include <QGLWidget>
#include "shape.h"

/**
 * Class for managing OpenGL 3.2 rendering
 */
class Renderer
{
private:

    QGLWidget *canvas;              ///< Qt drawing surface

    std::string shaderDir;          ///< location of all shaders

    glm::vec4 pointLight;           ///< position of light source in world space
    glm::vec4 directionalLight[2];  ///< directional lights (vector) for radiance scaling
    glm::vec4 lightDiffuseColour;   ///< diffuse colour of light source
    glm::vec4 lightSpecColour;      ///< specular colour of light source
    glm::vec4 lightAmbientColour;   ///< ambient colour of light source
    GLfloat shinySpec;              ///< specular coefficient

    bool shadersReady;              ///< have shaders been compiled/linked?

    glm::mat4x4 viewMx, MVmx, projMx, MVP; ///< OpenGL transformation/view matrices
    glm::mat3x3 normalMatrix;       ///< normal transformation matrix

    std::map<std::string, shaderProgram*> shaders;  ///< available shaders
    std::vector<ShapeDrawData> drawCallData;        ///< drawing state for scene shapes

public:

    /// constructor
    Renderer(QGLWidget *drawTo = NULL, const std::string &dir=".");

    /// destructor
    ~Renderer();

    /**
     * setter for camera view
     * @param mx    view matrix
     */
    void setCamera(glm::mat4x4& mx)
    {
        viewMx = mx;
        MVmx = mx;
        normalMatrix = glm::transpose(glm::inverse(glm::mat3(MVmx)));
        MVP = projMx * MVmx;
    }

    /**
     * setter for both projection and model-view matrices
     * @param mv    model-view matrix
     * @param proj  projection matrix
     */
    void setModelViewProjection(glm::mat4x4& mv, glm::mat4x4& proj)
    {
      MVmx = mv;
      projMx = proj;
      normalMatrix = glm::transpose(glm::inverse(glm::mat3(MVmx)));
      MVP = projMx * MVmx;

    }

    /**
     * setter for model-view matrix
     * @param mx    model-view matrix
     */
    void setModelView(glm::mat4x4& mx)
    {
      MVmx = mx;
      normalMatrix = glm::transpose(glm::inverse(glm::mat3(MVmx)));
      MVP = projMx * MVmx;
    }

    /**
     * setter for projection matrix
     * @param mx    projection matrix
     */
    void setProjection(glm::mat4x4 &mx)
    {
        projMx = mx;
        MVP = projMx * MVmx;
    }

    /**
     * Set world-space light position
     * @param x, y, z   3D coordinates of light
     */
    void setPointLight(float x, float y, float z)
    {
        pointLight = glm::vec4(x, y, z, 1.0);
        //std::cout << "point Light position = (" << x << "," << y << ","<< z << ")\n";
    }

    /**
     * Set directional light source
     * @param n         index for which light to set
     * @param x, y, z   direction vector for light
     */
    void setDirectionalLight(int n, float x, float y, float z)
    {
        if (n < 0 || n > 1)
        {
            std::cerr << "Directional light index must be 0 or 1!\n";
            return;
        }
        directionalLight[n] = glm::vec4(x, y, z, 0.0);
        // std::cout << "Directional light " << n << " = (" << x << "," << y << ","<< z << ")\n";
    }

    /// get a pointer to a compiled shader program object; this can be queried for program ID.
    shaderProgram* getShaderProgramObject(const std::string& name) const
    {
        std::map<std::string, shaderProgram*>::const_iterator it;
        if ((it = shaders.find(name)) == shaders.end())
            return NULL;
        else
            return it->second;
    }

    // copy across manipulator/constraint draw data
    /**
     * Copy in drawing state data for each shape
     * @param indata    state information, one entry per shape
     */
    void setDrawParams(const std::vector<ShapeDrawData>& indata)
    {
        drawCallData = indata;
    }

    /// Initialise render object. Must be called before any other operations to set up and compile shaders
    void initShaders(void);

    /**
     * Setup and issue OpenGL draw call. setDrawParams must be issued first.
     * @param view  current view state
     */
    void draw(View * view);
};

#endif // RENDERER_H
