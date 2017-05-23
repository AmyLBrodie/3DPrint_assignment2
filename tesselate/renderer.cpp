#include "renderer.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <algorithm>

Renderer::Renderer(QGLWidget *drawTo, const std::string& dir)
{
    canvas = drawTo;
    shaderDir = dir;
    shadersReady = false;

    // lights
    pointLight = glm::vec4(0.0f, 0.0f, -1.0f, 1.0f);
    directionalLight[0] = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    directionalLight[1] = glm::vec4(0.5f, 0.5f, 0.0f, 0.0f);

    lightDiffuseColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f); // colour of light
    lightSpecColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    lightAmbientColour = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);
    shinySpec = 5.0f; // specular power

    // default camera
    viewMx = glm::lookAt(glm::vec3(2.0f, 3.0f, 3.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0, 1.0f, 0.0f));
    MVmx = viewMx;
    normalMatrix = glm::transpose(glm::inverse(glm::mat3(MVmx)));
    projMx = glm::frustum(-8.0f, 8.0f, -8.0f, 8.0f, 50.0f, 100000.0f);
    MVP = projMx  * MVmx;
}

Renderer::~Renderer()
{
    // delete shaders
    std::map<std::string, shaderProgram*>::iterator it = shaders.begin();
    while (it != shaders.end() )
    {
        delete (*it).second; it++;
    }
}

void Renderer::initShaders(void)
{
    // set up shaders for loading and compilation

    if (shadersReady) return; // already compiled

    shaderProgram *s;

    s = new shaderProgram();
    s->setShaderSources(std::string("basic.frag"), std::string("basic.vert"));
    shaders["basicShader"] = s;

    s = new shaderProgram();
    s->setShaderSources(std::string("genNormal.frag"), std::string("genNormal.vert"));
    shaders["normalShader"] = s;

    s = new shaderProgram();
    s->setShaderSources(std::string("phong.frag"), std::string("phong.vert"));
    shaders["phong"] = s;

    s = new shaderProgram();
    s->setShaderSources(std::string("rad_scaling_pass1.frag"), std::string("rad_scaling_pass1.vert"));
    shaders["rscale1"] = s;

    s = new shaderProgram();
    s->setShaderSources(std::string("rad_scaling_pass2.frag"), std::string("rad_scaling_pass2.vert"));
    shaders["rscale2"] = s;

    s = new shaderProgram();
    s->setShaderSources(std::string("phongRS.frag"), std::string("phongRS.vert"));
    shaders["phongRS"] = s;

    s = new shaderProgram();
    s->setShaderSources(std::string("phongRSmanip.frag"), std::string("phongRSmanip.vert"));
    shaders["phongRSmanip"] = s;

    std::cout << "Compiling shaders...\n";
    std::map<std::string, shaderProgram*>::iterator it = shaders.begin();
    while (it != shaders.end())
    {
        std::cout << " -- shader: " << (*it).first << " -- ";
        (void)((*it).second)->compileAndLink();
        std::cout << "ID = " << ((*it).second)->getProgramID() << std::endl;
        it++;
    }
    shadersReady = true;
    std::cout << "done!\n";
}

void Renderer::draw(View * view)
{
    if (!shadersReady) // not compiled!
    {
        std::cerr << "Shaders not built before draw() call - compiling...\n";
        initShaders();
    }

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // OpenGL settings
    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f ); CE();
    glEnable(GL_DEPTH_TEST); CE();
    glDepthMask(GL_TRUE); CE();
    glDepthFunc(GL_LEQUAL); CE();
    glDepthRange(0.0f, 1.0f); CE();
    glEnable(GL_CULL_FACE); CE();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); CE();

    // configure shading params
    normalMatrix = view->getNormalMtx(); // TO FIX?
    MVP = view->getMatrix();
    MVmx = view->getViewMtx();
    projMx = view->getProjMtx();

    GLuint programID = (*shaders["phong"]).getProgramID();

    glUseProgram(programID); CE();

    for (int i = 0; i < (int)drawCallData.size(); i++)
    {
        glUniformMatrix4fv(glGetUniformLocation(programID, "MV"), 1, GL_FALSE, glm::value_ptr(MVmx) ); CE();
        glUniformMatrix4fv(glGetUniformLocation(programID, "MVproj"), 1, GL_FALSE, glm::value_ptr(MVP) ); CE();
        glUniformMatrix3fv(glGetUniformLocation(programID, "normMx"), 1, GL_FALSE, glm::value_ptr(normalMatrix)); CE();

        glm::vec4 MatDiffuse = glm::vec4(drawCallData[i].diffuse[0], drawCallData[i].diffuse[1],
                                         drawCallData[i].diffuse[2], drawCallData[i].diffuse[3]); // diffuse colour
        glm::vec4 MatAmbient = glm::vec4(drawCallData[i].ambient[0], drawCallData[i].ambient[1],
                                         drawCallData[i].ambient[2], drawCallData[i].ambient[3]); // ambient colour
        glm::vec4 MatSpecular = glm::vec4(drawCallData[i].specular[0], drawCallData[i].specular[1],
                                          drawCallData[i].specular[2], drawCallData[i].specular[3]); // specular colour
        glm::vec4 lightDiffuseColour = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f); // colour of light
        glm::vec4 lightAmbientColour = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);

        // set colours and light
        glUniform4fv(glGetUniformLocation(programID, "matDiffuse"), 1, glm::value_ptr(MatDiffuse) ); CE();
        glUniform4fv(glGetUniformLocation(programID, "matAmbient"), 1, glm::value_ptr(MatAmbient) ); CE();
        glUniform4fv(glGetUniformLocation(programID, "matSpec"), 1, glm::value_ptr(MatSpecular) ); CE();
        glUniform4fv(glGetUniformLocation(programID, "lightPos"), 1, glm::value_ptr(pointLight)); CE();
        glUniform4fv(glGetUniformLocation(programID, "diffuseCol"), 1, glm::value_ptr(lightDiffuseColour) ); CE();
        glUniform4fv(glGetUniformLocation(programID, "ambientCol"), 1, glm::value_ptr(lightAmbientColour) ); CE();
        glUniform4fv(glGetUniformLocation(programID, "specularCol"), 1, glm::value_ptr(lightSpecColour) ); CE();
        glUniform1f(glGetUniformLocation(programID, "shiny"), shinySpec); CE();

        glBindVertexArray(drawCallData[i].VAO); CE();
        glDrawElements(GL_TRIANGLES, drawCallData[i].indexBufSize, GL_UNSIGNED_INT, (void*)(0)); CE();
        glBindVertexArray(0); CE();
    }
    
    // unbind vao
    glBindVertexArray(0); CE();

    glUseProgram(0);  CE();
}