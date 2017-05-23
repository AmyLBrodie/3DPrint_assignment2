#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "glheaders.h"
//#include <glew.h>
//#include <GL/glu.h>

#include <string>
#include <common/source2cpp.h>

class shaderProgram
{
private:

    GLuint program_ID;
    GLuint frag_ID;
    GLuint vert_ID;
    bool shaderReady;
    bool fileInput; // input comes from file rather than strings
    std::string fragSrc, vertSrc;

    // private mehods
    GLenum compileProgram(GLenum target, GLchar* sourcecode, GLuint & shader);
    GLenum linkProgram(GLuint program);

public:

    /// construct from string source
    shaderProgram(const std::string& frageSource, const std::string& vertSource);

    /// construct from file names
    shaderProgram(const char * fragSourceFile, const char * vertSourceFile);

    shaderProgram(void)
    {
        program_ID = frag_ID = vert_ID = 0;
        shaderReady = false;
        fileInput = false;
        fragSrc ="";
        vertSrc = "";
    }

    ~shaderProgram(){}

    void setShaderSources(const std::string& frageSource, const std::string& vertSource);
    void setShaderSources(const char * fragSourceFile, const char * vertSourceFile);

    bool  compileAndLink(void);

    GLuint getProgramID(void) const { return program_ID; }
    bool initialised(void) const {return shaderReady; }
};

#endif