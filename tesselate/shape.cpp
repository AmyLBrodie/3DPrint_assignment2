#include <GL/glew.h>
#include "shape.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace cgp;

void ShapeGeometry::setColour(GLfloat * col)
{
    int i;

    for(i = 0; i < 4; i++)
        diffuse[i] = col[i];
    for(i = 0; i < 3; i++)
        ambient[i] = diffuse[i] * 0.75f;
    for(i = 0; i < 3; i++)
        specular[i] = std::min(1.0f, diffuse[i] * 1.25f);
}

void ShapeGeometry::genCylinder(float radius, float height, int slices, int stacks, glm::mat4x4 trm)
{
    int i, j, base;

    float a, x, y, h = 0.0f;
    float stepa = PI2 / (float) slices;
    float stepz = height / (float) stacks;
    glm::vec4 p;
    glm::vec3 v;

    base = int(verts.size()) / 8;
    for(i = 0; i <= stacks; i++)
    {
        a = 0.0f;
        for (j = 0; j < slices; j++)
        {
            x = cosf(a) * radius;
            y = sinf(a) * radius;

            // apply transformation
            p = trm * glm::vec4(x, y, h, 1.0f);
            v = glm::transpose(glm::inverse(glm::mat3(trm))) * glm::normalize(glm::vec3(x, y, 0.0f));
            v = glm::normalize(v);

            verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
            verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
            verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal

            if(i > 0)
            {
                if(j < slices-1)
                {
                    indices.push_back(base-slices+j); indices.push_back(base-slices+j+1);indices.push_back(base+j);
                    indices.push_back(base-slices+j+1); indices.push_back(base+j+1); indices.push_back(base+j);
                }
                else // wrap
                {
                    indices.push_back(base-slices+j); indices.push_back(base-slices); indices.push_back(base+j);
                    indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+j);
                }
            }
            a += stepa;
        }
        base += slices;
        h += stepz;
    }
}

void ShapeGeometry::genSphereVert(float radius, float lat, float lon, glm::mat4x4 trm)
{

    float la, lo, x, y, z;
    glm::vec4 p;
    glm::vec3 v;

    la = PI+PI*lat;
    lo = PI2*lon;
    // this is unoptimized
    x = cosf(lo)*sinf(la)*radius;
    y = sinf(lo)*sinf(la)*radius;
    z = cosf(la)*radius;

    // apply transformation
    p = trm * glm::vec4(x, y, z, 1.0f);
    // v = glm::mat3(trm) * glm::normalize(glm::vec3(x, y, z));
    v = glm::transpose(glm::inverse(glm::mat3(trm))) * glm::normalize(glm::vec3(x, y, z));
    v = glm::normalize(v);

    verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
    verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
    verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal
}

void ShapeGeometry::genSphere(float radius, int slices, int stacks, glm::mat4x4 trm)
{
    int lat, lon, base;
    float plat, plon;

    // doesn't produce very evenly sized triangles, tend to cluster at poles
    base = int(verts.size()) / 8;
    for(lat = 0; lat <= stacks; lat++)
    {
        for(lon = 0; lon < slices; lon++)
        {
            plat = (float) lat / (float) stacks;
            plon = (float) lon / (float) slices;
            genSphereVert(radius, plat, plon, trm);

            if(lat > 0)
            {
                if(lon < slices-1)
                {
                    indices.push_back(base-slices+lon); indices.push_back(base-slices+lon+1); indices.push_back(base+lon);
                    indices.push_back(base-slices+lon+1); indices.push_back(base+lon+1); indices.push_back(base+lon);
                }
                else // wrap
                {
                    indices.push_back(base-slices+lon); indices.push_back(base-slices); indices.push_back(base+lon);
                    indices.push_back(base-slices); indices.push_back(base); indices.push_back(base+lon);
                }
            }
        }
        base += slices;
    }
}

void ShapeGeometry::genMesh(std::vector<cgp::Point> * points, std::vector<cgp::Vector> * norms, std::vector<int> * faces, glm::mat4x4 trm)
{
    int i, base;
    glm::vec4 p;
    glm::vec3 v;

    base = int(verts.size()) / 8;
    for(i = 0; i < (int) points->size(); i++)
    {
        // apply transformation
        p = trm * glm::vec4((* points)[i].x, (* points)[i].y, (* points)[i].z, 1.0f);
        // v = glm::mat3(trm) * glm::normalize(glm::vec3(x, y, z));
        v = glm::transpose(glm::inverse(glm::mat3(trm))) * glm::normalize(glm::vec3((* norms)[i].i, (* norms)[i].j, (* norms)[i].k));
        v = glm::normalize(v);


        verts.push_back(p.x); verts.push_back(p.y); verts.push_back(p.z); // position
        verts.push_back(0.0f); verts.push_back(0.0f); // texture coordinates
        verts.push_back(v.x); verts.push_back(v.y); verts.push_back(v.z); // normal
    }

    for(i = 0; i < (int) faces->size(); i++)
    {
        indices.push_back((* faces)[i]+base);
    }
}

ShapeDrawData ShapeGeometry::getDrawParameters()
{
    ShapeDrawData sdd;

    sdd.VAO = vaoGeom;
    for(int i = 0; i < 4; i++)
        sdd.diffuse[i] = diffuse[i];
    for(int i = 0; i < 4; i++)
        sdd.specular[i] = specular[i];
    for(int i = 0; i < 4; i++)
        sdd.ambient[i] = ambient[i];
    sdd.indexBufSize = (int) indices.size();
    sdd.texID = 0;
    sdd.current = false; // default setting

    return sdd;
}

bool ShapeGeometry::bindBuffers(View * view)
{
    if((int) indices.size() > 0)
    {
        if (vboGeom != 0)
        {
            glDeleteVertexArrays(1, &vaoGeom);
            glDeleteBuffers(1, &vboGeom);
            glDeleteBuffers(1, &iboGeom);
            vaoGeom = 0;
            vboGeom = 0;
            iboGeom = 0;
        }

        // vao
        glGenVertexArrays(1, &vaoGeom);
        glBindVertexArray(vaoGeom);

        // vbo
        // set up vertex buffer and copy in data
        glGenBuffers(1, &vboGeom);
        glBindBuffer(GL_ARRAY_BUFFER, vboGeom);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*(int) verts.size(), (GLfloat *) &verts[0], GL_STATIC_DRAW);

        // ibo
        glGenBuffers(1, &iboGeom);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboGeom);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*(int) indices.size(), (GLuint *) &indices[0], GL_STATIC_DRAW);

        // enable position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)(0));

        // enable texture coord attribute
        const int sz = 3*sizeof(GLfloat);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)(sz) );

        // enable normals
        const int nz = 5*sizeof(GLfloat);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8*sizeof(GLfloat), (void*)(nz) );

        //glBindVertexArray(vaoConstraint);
        //glDrawElements(GL_TRIANGLES, (int) indices.size(), GL_UNSIGNED_INT, (void*)(0));

        //glBindVertexArray(0);
        return true;
    }
    else
    {
        return false;
    }
}
