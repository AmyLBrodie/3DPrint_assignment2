//
// Mesh
//

#include "mesh.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <list>
#include <sys/stat.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/intersect.hpp>
#include <unordered_map>

using namespace std;
using namespace cgp;

GLfloat stdCol[] = {0.7f, 0.7f, 0.75f, 0.4f};
const int raysamples = 2;

void Sphere::genGeometry(ShapeGeometry * geom, View * view)
{
    glm::mat4 tfm, idt;
    glm::vec3 trs;

    idt = glm::mat4(1.0f); // identity matrix
    trs = glm::vec3(c.x, c.y, c.z);
    tfm = glm::translate(idt, trs);
    geom->genSphere(r, 40, 40, tfm);
}

bool Sphere::pointContainment(cgp::Point pnt)
{
    float dist = c.dist(pnt);
    
    if (dist <= r){
    	return true;
    }
    else{
    	return false;
    }
    
    // stub, needs completing
}

void Cylinder::genGeometry(ShapeGeometry * geom, View * view)
{
    glm::mat4 tfm, idt;
    glm::vec3 trs, rot;
    Vector edgevec, zerovec, axisvec, zaxis;
    float edgelen, aval;

    edgevec.diff(s, e);
    edgelen = edgevec.length();
    edgevec.normalize();

    // translate to starting vertex
    trs = glm::vec3(s.x, s.y, s.z);
    tfm = glm::translate(idt, trs);

    // determine rotation axis, normal to plane of z axis and edge vector
    zaxis = Vector(0.0f, 0.0f, 1.0f);
    axisvec.cross(zaxis, edgevec);

    // check for degenerate cases
    if(axisvec == zerovec)
    {
        if(!(edgevec == zaxis)) // diametrically opposite
        {
            aval = 180.0f;
            rot = glm::vec3(0.0f, 1.0f, 0.0f);
            tfm = glm::rotate(tfm, aval, rot);
        }
        // otherwise aligned so no rotation
    }
    else
    {
        axisvec.normalize();
        aval = acosf(zaxis.dot(edgevec)) * RAD2DEG;
        rot = glm::vec3(axisvec.i, axisvec.j, axisvec.k);
        tfm = glm::rotate(tfm, aval, rot);
    }

    // align to edge vector by rotation
    geom->genCylinder(r, edgelen, 12, 4, tfm);
}

bool Cylinder::pointContainment(cgp::Point pnt)
{
    cgp::Vector dirvec;
    float dist, tval;

    // find distance and parameter value to closest point on the axis of the cylinder
    dirvec.diff(s, e);
    rayPointDist(s, dirvec, pnt, tval, dist);
    if(tval >= 0.0f && tval <= 1.0f && dist <= r)
        return true;
    else
        return false;
}

bool Mesh::findVert(cgp::Point pnt, int &idx)
{
    bool found = false;
    int i = 0;

    idx = -1;
    // linear search of vertex list
    while(!found && i < (int) verts.size())
    {
        if(verts[i] == pnt)
        {
            found = true;
            idx = i;
        }
        i++;
    }
    return found;
}

long Mesh::hashVert(cgp::Point pnt, cgp::BoundBox bbox)
{
    long x, y, z;
    float range = 2500.0f;
    long lrangesq, lrange = 2500;

    lrangesq = lrange * lrange;

    // discretise vertex within bounds of the enclosing bounding box
    x = (long) (((pnt.x - bbox.min.x) * range) / bbox.diagLen()) * lrangesq;
    y = (long) (((pnt.y - bbox.min.y) * range) / bbox.diagLen()) * lrange;
    z = (long) (((pnt.z - bbox.min.z) * range) / bbox.diagLen());
    return x+y+z;
}

long Mesh::hashEdge(int v0, int v1)
{
    long key = ((long) (v0+v1) * (long) (v0+v1+1)) / 2;
    // use Cantor pairing function to derive a unique hash key function
    if(v0 < v1)
        key += (long) v1;
    else
        key += (long) v0;
    return key;
}

void Mesh::mergeVerts()
{
    vector<cgp::Point> cleanverts;
    long key;
    int i, p, hitcount = 0;
    // use hashmap to quickly look up vertices with the same coordinates
    std::unordered_map<long, int> idxlookup; // key is concatenation of vertex position, value is index into the cleanverts vector
    cgp::BoundBox bbox;

    // construct a bounding box enclosing all vertices
    for(i = 0; i < (int) verts.size(); i++)
        bbox.includePnt(verts[i]);

    // remove duplicate vertices
    for(i = 0; i < (int) verts.size(); i++)
    {
        key = hashVert(verts[i], bbox);
        if(idxlookup.find(key) == idxlookup.end()) // key not in map
        {
            idxlookup[key] = (int) cleanverts.size(); // put index in map for quick lookup
            cleanverts.push_back(verts[i]);
        }
        else
        {
            hitcount++;
        }
    }
    cerr << "num duplicate vertices found = " << hitcount << " of " << (int) verts.size() << endl;
    cerr << "clean verts = " << (int) cleanverts.size() << endl;
    cerr << "bbox min = " << bbox.min.x << ", " << bbox.min.y << ", " << bbox.min.z << endl;
    cerr << "bbox max = " << bbox.max.x << ", " << bbox.max.y << ", " << bbox.max.z << endl;
    cerr << "bbox diag = " << bbox.diagLen() << endl;


    // re-index triangles
    for(i = 0; i < (int) tris.size(); i++)
        for(p = 0; p < 3; p++)
        {
            key = hashVert(verts[tris[i].v[p]], bbox);
            if(idxlookup.find(key) != idxlookup.end())
                tris[i].v[p] = idxlookup[key];
            else
                cerr << "Error Mesh::mergeVerts: vertex not found in map" << endl;

        }

    verts.clear();
    verts = cleanverts;
}

void Mesh::deriveVertNorms()
{
    vector<int> vinc; // number of faces incident on vertex
    int p, t;
    cgp::Vector n;

    // init structures
    for(p = 0; p < (int) verts.size(); p++)
    {
        vinc.push_back(0);
        norms.push_back(cgp::Vector(0.0f, 0.0f, 0.0f));
    }

    // accumulate face normals into vertex normals
    for(t = 0; t < (int) tris.size(); t++)
    {
        n = tris[t].n; n.normalize();
        for(p = 0; p < 3; p++)
        {
            norms[tris[t].v[p]].add(n);
            vinc[tris[t].v[p]]++;
        }
    }

    // complete average
    for(p = 0; p < (int) verts.size(); p++)
    {
        norms[p].mult(1.0f/((float) vinc[p]));
        norms[p].normalize();
    }
}

void Mesh::deriveFaceNorms()
{
    int t;
    cgp::Vector evec[2];

    for(t = 0; t < (int) tris.size(); t++)
    {
        // right-hand rule for calculating normals, i.e. counter-clockwise winding from front on vertices
        evec[0].diff(verts[tris[t].v[0]], verts[tris[t].v[1]]);
        evec[1].diff(verts[tris[t].v[0]], verts[tris[t].v[2]]);
        evec[0].normalize();
        evec[1].normalize();
        tris[t].n.cross(evec[0], evec[1]);
        tris[t].n.normalize();
    }

}

void Mesh::buildTransform(glm::mat4x4 &tfm)
{
    glm::mat4x4 idt;

    idt = glm::mat4(1.0f);
    tfm = glm::translate(idt, glm::vec3(trx.i, trx.j, trx.k));
    tfm = glm::rotate(tfm, zrot, glm::vec3(0.0f, 0.0f, 1.0f));
    tfm = glm::rotate(tfm, yrot, glm::vec3(0.0f, 1.0f, 0.0f));
    tfm = glm::rotate(tfm, xrot, glm::vec3(1.0f, 0.0f, 0.0f));
    tfm = glm::scale(tfm, glm::vec3(scale));
}

void Mesh::buildSphereAccel(int maxspheres)
{
    vector<Sphere> packedspheres;
    list<int> insphere;
    int i, v, p, t, s, numsx, numsy, numsz, sx, sy, sz, x, y, z, sind = 0;
    cgp::Point centroid, center;
    cgp::Vector radvec, diag, edgevec;
    float radius, dist, packrad, packspace, maxedge;
    cgp::BoundBox bbox;
    Sphere sph;
    bool found;

    if((int) tris.size() > 0) // must actually have a mesh to build the acceleration structure on top of
    {
        // calculate longest triangle edge
        maxedge = 0.0f;
        for(t = 0; t < (int) tris.size(); t++)
        {
            edgevec.diff(verts[tris[t].v[0]], verts[tris[t].v[1]]);
            dist = edgevec.length();

            edgevec.diff(verts[tris[t].v[1]], verts[tris[t].v[2]]);
            dist = min(dist, edgevec.length());

            edgevec.diff(verts[tris[t].v[2]], verts[tris[t].v[0]]);
            dist = min(dist, edgevec.length());
            if(dist > maxedge)
                maxedge = dist;
        }

        // calculate mesh bounding box
        for(v = 0; v < (int) verts.size(); v++)
            bbox.includePnt(verts[v]);

        diag = bbox.getDiag();
        dist = std::max(diag.i, diag.j); dist = std::max(dist, diag.k);
        packrad = dist / (float) maxspheres;
        if(packrad < maxedge) // sphere radius must be larger than the longest shorest edge in any triangle
        {
            cerr << "Error Mesh::buildSphereAccel: bounding spheres too small relative to edge length. Inflating spheres accordingly." << endl;
            packrad = maxedge;
        }
        packspace = 2.0f * packrad;

        // number of spheres to pack in each dimension filling the bounding box
        numsx = (int) ceil(diag.i / packspace) +1;
        numsy = (int) ceil(diag.j / packrad) +1;
        numsz = (int) ceil(diag.k / packrad) +1;

        // pack spheres into volume so that there is no unsampled space
        // alternating rows are offset by the sphere radius and x-adjacent spheres are spaced to just touch
        // this provides a minimal overlap between spheres that nevertheless covers all available space
        for(x = 0; x < numsx; x++)
            for(y = 0; y < numsy; y++)
                for(z = 0; z < numsz; z++)
                {
                    center = cgp::Point((float) x * packspace + bbox.min.x, (float) y * packrad + bbox.min.y, (float) z * packrad + bbox.min.z);
                    if(z%2==1) // row shifting alternates in layers
                    {
                        if(y%2 == 1) // shift alternating rows by half spacing in x
                            center.x += packrad;
                    }
                    else
                    {
                        if(y%2 == 0) // shift alternating rows by half spacing in x
                            center.x += packrad;
                    }

                    sph.c = center;
                    sph.r = packrad;
                    sph.ind.clear();
                    packedspheres.push_back(sph);
                }

        // assign triangles to spheres based on their vertices
        for(t = 0; t < (int) tris.size(); t++)
        {

            // find the first sphere that contains any of the triangle vertices
            found = false; s = 0;
            while(!found && s < (int) packedspheres.size())
            {
                for(p = 0; p < 3; p++)
                    if(packedspheres[s].pointContainment(verts[tris[t].v[p]]))
                    {
                        found = true;
                        // add triangle to packedsphere
                        packedspheres[s].ind.push_back(t);
                        sind = s;
                    }
                s++;
            }

            if(!found)
            {
                cerr << "Error Mesh::buildSphereAccel: packed spheres do not entirely fill the volume." << endl;
            }
            else
            {
                // once a sphere triangle intersection is found only a few adjacent spheres need be tested for intersection
                // also test against +x, +y, +z spheres (and diagonal adjacents as well for possible intersection due to partial sphere overlaps

                // neighbour search of packed spheres
                // decompose into x, y, z sphere grid position

                // unflatten index
                sx = (int) ((float) sind / (float) (numsy*numsz));
                sy = (int) ((float) sind / (float) numsz);
                sz = sind - (sx * numsy * numsz + sy * numsz);
                for(x = sx; x <= sx+1; x++)
                    for(y = sy; y <= sy+1; y++)
                        for(z = sz; z <= sz+1; z++)
                        {
                            if(x < numsx && y < numsy && z < numsz) // bounds check
                                if(!(x == sx && y == sy && z == sz)) // ignore starter sphere
                                {
                                    s = x * numsy * numsz + y * numsz + z; // flatten 3d index
                                    found = false;
                                    for(p = 0; p < 3; p++)
                                        if(packedspheres[s].pointContainment(verts[tris[t].v[p]]))
                                            found = true;
                                    if(found) // add triangle to packedsphere
                                        packedspheres[s].ind.push_back(t);
                                }

                        }

            }
        }

        // discard any spheres containing no triangles
        for(i = 0; i < (int) packedspheres.size(); i++)
        {
            if(!packedspheres[i].ind.empty()) // bounding sphere has at least one triangle
                boundspheres.push_back(packedspheres[i]);
        }
        packedspheres.clear();

        // recaculate spheres by using center of mass and furthest triangle vertex to get tighter bounding
        for(i = 0; i < (int) boundspheres.size(); i++)
        {
            insphere.clear();
            // gather included vertices
            for(t = 0; t < (int) boundspheres[i].ind.size(); t++)
                for(p = 0; p < 3; p++)
                    insphere.push_back(tris[boundspheres[i].ind[t]].v[p]);

            // remove duplicates because they will bias the center of mass
            insphere.sort();
            insphere.unique();

            // calculate centroid as average of vertices
            centroid = cgp::Point(0.0f, 0.0f, 0.0f);
            for (list<int>::iterator it=insphere.begin(); it!=insphere.end(); ++it)
            {
                centroid.x += verts[(* it)].x; centroid.y += verts[(* it)].y; centroid.z += verts[(* it)].z;

            }
            centroid.x /= (float) insphere.size();  centroid.y /= (float) insphere.size();  centroid.z /= (float) insphere.size();
            boundspheres[i].c = centroid;

            // calculate new radius
            radius = 0.0f;
            for (list<int>::iterator it=insphere.begin(); it!=insphere.end(); ++it)
            {
                radvec.diff(boundspheres[i].c, verts[(* it)]);
                dist = radvec.length();
                if(dist > radius)
                    radius = dist;
            }
            boundspheres[i].r = radius;

            /*
             // check current radius
             for (list<int>::iterator it=insphere.begin(); it!=insphere.end(); ++it)
             {
             radvec.diff(boundspheres[i].c, verts[(* it)]);
             dist = radvec.length();
             if(dist > boundspheres[i].r)
             cerr << "Error Mesh::buildSphereAccel:vertex " << (* it) << " outside containing sphere by " << (dist - packrad) << endl;
             }*/
        }
    }
}

Mesh::Mesh()
{
    col = stdCol;
    scale = 1.0f;
    xrot = yrot = zrot = 0.0f;
    trx = cgp::Vector(0.0f, 0.0f, 0.0f);
}

Mesh::~Mesh()
{
    clear();
}

void Mesh::clear()
{
    verts.clear();
    tris.clear();
    geometry.clear();
    col = stdCol;
    scale = 1.0f;
    xrot = yrot = zrot = 0.0f;
    trx = cgp::Vector(0.0f, 0.0f, 0.0f);
}

void Mesh::genGeometry(ShapeGeometry * geom, View * view)
{
    vector<int> faces;
    int t, p;
    glm::mat4x4 tfm;

    // transform mesh data structures into a form suitable for rendering
    // by flattening the triangle list
    for(t = 0; t < (int) tris.size(); t++)
        for(p = 0; p < 3; p++)
            faces.push_back(tris[t].v[p]);

    // construct transformation matrix
    buildTransform(tfm);
    geom->genMesh(&verts, &norms, &faces, tfm);
}

bool Mesh::bindGeometry(View * view, ShapeDrawData &sdd)
{
    geometry.clear();
    geometry.setColour(col);
    genGeometry(&geometry, view);

    // bind geometry to buffers and return drawing parameters, if possible
    if(geometry.bindBuffers(view))
    {
        sdd = geometry.getDrawParameters();
        return true;
    }
    else
       return false;
}

bool Mesh::pointContainment(cgp::Point pnt)
{
    int incount = 0, outcount = 0, hits, i, t, p, s;
    glm::vec3 v[3], origin, xsect, ray;
    glm::mat4x4 tfm, idt;
    glm::vec4 vxfm, cxfm;
    cgp::Point vert, sphc;
    cgp::Vector dir;
    float dist, tval;
    list<int> inspheres;

    srand(time(0));

    // sample over multiple rays to avoid numerical issues (e.g., ray hits a vertex or edge)
    origin = glm::vec3(pnt.x, pnt.y, pnt.z);

    // construct transformation matrix
    buildTransform(tfm);

    if(boundspheres.empty()) // no acceleration structure so build
        buildSphereAccel((int) sphperdim);

    for(i = 0; i < raysamples; i++)
    {
        hits = 0;
        inspheres.clear();

        // sampling ray with random direction
        // avoid axis aligned rays because more likely to lead to numerical issues with axis aligned structures
        dir = cgp::Vector((float) (rand()%1000-500), (float) (rand()%1000-500), (float) (rand()%1000-500));
        dir.normalize();
        ray = glm::vec3(dir.i, dir.j, dir.k);

        // gather potential triangle intersector indices and remove duplicates
        for(s = 0; s < (int) boundspheres.size(); s++)
        {
            // sphere accel structure is in model space, so need to apply transform
            cxfm = tfm * glm::vec4(boundspheres[s].c.x, boundspheres[s].c.y, boundspheres[s].c.z, 1.0f);
            sphc = cgp::Point(cxfm.x, cxfm.y, cxfm.z);

            rayPointDist(pnt, dir, sphc, tval, dist);
            if(dist <= boundspheres[s].r) // if ray hits the bounding sphere
                for(t = 0; t < (int) boundspheres[s].ind.size(); t++) // include all triangles allocated to the bounding sphere
                    inspheres.push_back(boundspheres[s].ind[t]);
        }

        // remove duplicate triangle indices because a triangle may appear in multiple bounding spheres
        inspheres.sort();
        inspheres.unique();

        // test intersection against list of triangles
        for (list<int>::iterator it=inspheres.begin(); it!=inspheres.end(); ++it)
        {
            for(p = 0; p < 3; p++)
            {
                vert = verts[tris[(* it)].v[p]];
                vxfm = tfm * glm::vec4(vert.x, vert.y, vert.z, 1.0f);
                v[p] = glm::vec3(vxfm.x, vxfm.y, vxfm.z);
            }
            if(glm::intersectRayTriangle(origin, ray, v[0], v[1], v[2], xsect) || glm::intersectRayTriangle(origin, ray, v[0], v[2], v[1], xsect)) // test triangle in both windings because intersectLineTriangle is winding dependent
                hits++;
        }

        if(hits%2 == 0) // even number of intersection means point is outside
            outcount++;
        else // point is inside
            incount++;
    }
    
    // consensus wins
    return (incount > outcount);
}

void Mesh::boxFit(float sidelen)
{
    cgp::Point pnt;
    cgp::Vector shift, diag, halfdiag;
    float scale;
    int v;
    cgp::BoundBox bbox;

    // calculate current bounding box
    for(v = 0; v < (int) verts.size(); v++)
        bbox.includePnt(verts[v]);

    if((int) verts.size() > 0)
    {
        // calculate translation necessary to move center of bounding box to the origin
        diag = bbox.getDiag();
        shift.pntconvert(bbox.min);
        halfdiag = diag; halfdiag.mult(0.5f);
        shift.add(halfdiag);
        shift.mult(-1.0f);

        // scale so that largest side of bounding box fits sidelen
        scale = max(diag.i, diag.j); scale = max(scale, diag.k);
        if(scale > 0.0f)
        {
            scale = sidelen / scale;

            // shift center to origin and scale uniformly
            for(v = 0; v < (int) verts.size(); v++)
            {
                pnt = verts[v];
                shift.pntplusvec(pnt, &pnt);
                pnt.x *= scale; pnt.y *= scale; pnt.z *= scale;
                verts[v] = pnt;
            }
        }
        buildSphereAccel((int) sphperdim);
    }
}

bool Mesh::readSTL(string filename)
{
    ifstream infile;
    char * inbuffer;
    struct stat results;
    int insize, inpos, numt, t, i;
    cgp::Point vpos;
    Triangle tri;

    // assumes binary format STL file
    infile.open((char *) filename.c_str(), ios_base::in | ios_base::binary);
    if(infile.is_open())
    {
        clear();

        // get the size of the file
        stat((char *) filename.c_str(), &results);
        insize = results.st_size;

        // put file contents in buffer
        inbuffer = new char[insize];
        infile.read(inbuffer, insize);
        if(!infile) // failed to read from the file for some reason
        {
            cerr << "Error Mesh::readSTL: unable to populate read buffer" << endl;
            return false;
        }

        // interpret buffer as STL file
        if(insize <= 84)
        {
            cerr << "Error Mesh::readSTL: invalid STL binary file, too small" << endl;
            return false;
        }

        inpos = 80; // skip 80 character header
        if(inpos+4 >= insize){ cerr << "Error Mesh::readSTL: malformed header on stl file" << endl; return false; }
        numt = (int) (* ((long *) &inbuffer[inpos]));
        inpos += 4;

        t = 0;

        // triangle vertices have consistent outward facing clockwise winding (right hand rule)
        while(t < numt) // read in triangle data
        {
            // normal
            if(inpos+12 >= insize){ cerr << "Error Mesh::readSTL: malformed stl file" << endl; return false; }
            // IEEE floating point 4-byte binary numerical representation, IEEE754, little endian
            tri.n = cgp::Vector((* ((float *) &inbuffer[inpos])), (* ((float *) &inbuffer[inpos+4])), (* ((float *) &inbuffer[inpos+8])));
            inpos += 12;

            // vertices
            for(i = 0; i < 3; i++)
            {
                if(inpos+12 >= insize){ cerr << "Error Mesh::readSTL: malformed stl file" << endl; return false; }
                vpos = cgp::Point((* ((float *) &inbuffer[inpos])), (* ((float *) &inbuffer[inpos+4])), (* ((float *) &inbuffer[inpos+8])));
                tri.v[i] = (int) verts.size();
                verts.push_back(vpos);
                inpos += 12;
            }
            tris.push_back(tri);
            t++;
            inpos += 2; // handle attribute byte count - which can simply be discarded
        }

        // tidy up
        delete inbuffer;
        infile.close();

        cerr << "num vertices = " << (int) verts.size() << endl;
        cerr << "num triangles = " << (int) tris.size() << endl;

        // STL provides a triangle soup so merge vertices that are coincident
        mergeVerts();
        // normal vectors at vertices are needed for rendering so derive from incident faces
        deriveVertNorms();
        if(basicValidity())
            cerr << "loaded file has basic validity" << endl;
        else
            cerr << "loaded file does not pass basic validity" << endl;
    }
    else
    {
        cerr << "Error Mesh::readSTL: unable to open " << filename << endl;
        return false;
    }
    return true;
}

bool Mesh::writeSTL(string filename)
{
    ofstream outfile;
    int t, p, numt;
    unsigned short val;

    outfile.open((char *) filename.c_str(), ios_base::out | ios_base::binary);
    if(outfile.is_open())
    {
        outfile.write("File Generated by Tesselator. Binary STL", 80); // skippable header
        numt = (int) tris.size();
        outfile.write((char *) &numt, 4); // number of triangles

        for(t = 0; t < numt; t++)
        {
            // normal
            outfile.write((char *) &tris[t].n.i, 4);
            outfile.write((char *) &tris[t].n.j, 4);
            outfile.write((char *) &tris[t].n.k, 4);

            // triangle vertices
            for(p = 0; p < 3; p++)
            {
                outfile.write((char *) &verts[tris[t].v[p]].x, 4);
                outfile.write((char *) &verts[tris[t].v[p]].y, 4);
                outfile.write((char *) &verts[tris[t].v[p]].z, 4);
            }

            // attribute byte count - null
            val = 0;
            outfile.write((char *) &val, 2);
        }

        // tidy up
        outfile.close();
    }
    else
    {
        cerr << "Error Mesh::writeSTL: unable to open " << filename << endl;
        return false;
    }
    return true;
}

bool Mesh::basicValidity()
{
    int i, p, t, v, e, numedges, numverts, numtris;
    vector<bool> dangle;
    long key;
    // use hashmap to quickly look up vertices with the same coordinates
    std::unordered_map<long, int> idxlookup; // key is concatenation of vertex position, value is index into the cleanverts vector
    std::unordered_map<long, int> edgelookup;
    cgp::BoundBox bbox;
    vector<cgp::Point> cleanverts;

    // search vertex list for duplicates
    // duplicate vertices will not occur if MergeVerts has taken place

    /*
     // inefficient search of vertex list for duplicates - O(n^2)
     int v = 0, vrest = 0;
     bool found = false;
     while(!found && v < (int) verts.size()-1)
     {
     // search remainder of list
     vrest = v+1;
     while(!found && vrest < (int) verts.size())
     {
     if(verts[v] == verts[vrest])
     {
     found = true;
     }
     vrest++;
     }
     v++;
     }
     if(found)
     {
     cerr << "Error Mesh::basicValidity(): duplicate vertex found" << endl;
     return false;
     }
     */

    // construct a bounding box enclosing all vertices
    bbox.reset();
    for(i = 0; i < (int) verts.size(); i++)
        bbox.includePnt(verts[i]);

    cleanverts.clear();

    // search vertex list for duplicates using hash table
    for(i = 0; i < (int) verts.size(); i++)
    {
        key = hashVert(verts[i], bbox);
        if(idxlookup.find(key) == idxlookup.end()) // key not in map
        {
            idxlookup[key] = (int) cleanverts.size(); // put index in map for quick lookup
            cleanverts.push_back(verts[i]);
        }
        else
        {
            cerr << "Error Mesh::basicValidity(): duplicate vertex found" << endl;
            return false; // early out - exits on first duplicate encountered
        }
    }

    // create an edge list to count the number of edges
    // usual trick for more efficient version using hash table
    numedges = 0;
    for(i = 0; i < (int) tris.size(); i++)
    {
        for(e = 0; e < 3; e++)
        {
            key = hashEdge(tris[i].v[e], tris[i].v[(e+1)%3]);
            if(edgelookup.find(key) == edgelookup.end()) // key not in map
            {
                edgelookup[key] = i; // put index in map for quick lookup
                numedges++;
            }
        }
    }

    // Report Euler's Characteristic
    numverts = (int) verts.size();
    numtris = (int) tris.size();
    int euler = numverts - numedges + numtris;
    cerr << "Euler's Characteristic: Vertices - Edges + Faces = " << numverts << " - " << numedges << " + " << numtris << " = " << euler << endl;

    // test for dangling vertices that do not belong to any triangles
    // build flag list corresponding to vertex list. Entry true if a vertex is dangling.
    for(v = 0; v < (int) verts.size(); v++)
        dangle.push_back(true);

    // switch flags for vertices incident on an edge
    for(t = 0; t < (int) tris.size(); t++)
        for(p = 0; p < 3; p++)
        {
            if(tris[t].v[p] >= 0 && tris[t].v[p] < (int) verts.size()) // check for in range at the same time
            {
                dangle[tris[t].v[p]] = false;
            }
            else // vertex index out of bounds
            {
                cerr << "Error Mesh::basicValidity(): vertex index out of bounds" << endl;
                return false; // early out
            }
        }

    // check if any vertices are still flagged as dangling
    for(v = 0; v < (int) verts.size(); v++)
        if(dangle[v])
        {
            cerr << "Error Mesh::basicValidity(): dangling vertex found" << endl;
            return false; // early out
        }

    return true;
}

bool Mesh::sameTriangle(Triangle t1, Triangle t2)
{
    std::vector<int> ind1, ind2;

    // a duplicate has the same set of vertices but they may be ordered differently
    // so sort vertex indices of both triangles and then compare index by index
    for(int i = 0; i < 3; i++)
    {
        ind1.push_back(t1.v[i]);
        ind2.push_back(t2.v[i]);
    }
    std::sort(ind1.begin(), ind1.end());
    std::sort(ind2.begin(), ind2.end());
    return (ind1 == ind2);
}

bool Mesh::sameEdge(Edge e1, Edge e2, bool & opposite)
{
    if(e1.v[0] == e2.v[0] && e1.v[1] == e2.v[1])
    {
        opposite = false;
        return true;
    }
    if(e1.v[0] == e2.v[1] && e1.v[1] == e2.v[0])
    {
        opposite = true;
        return true;
    }
    return false;
}

bool Mesh::manifoldValidity()
{
    std::unordered_multimap<long, int> trilookup; // key is sum of vertex indices, needs a multimap because this is not unique
    long key;
    int i, j, k, t, v, e, erest, mcount, ocount;
    std::vector<std::vector<int>> incident;
    std::vector<int> incempty;
    std::vector<Edge> edges;
    std::vector<bool> visited;
    bool opposite, fin, found;
    Edge edge1, edge2;

    /*
     // inefficient search of triangle list for duplicates - O(n^2)
     int t = 0, trest = 0;
     bool found = false;
     while(!found && t < (int) tris.size()-1)
     {
     // search remainder of list
     trest = t+1;
     while(!found && trest < (int) tris.size())
     {
     if(sameTriangle(tris[t], tris[trest]))
     {
     found = true;
     }
     trest++;
     }
     t++;
     }
     if(found)
     {
     cerr << "Error Mesh::manifoldValidity(): duplicate triangle found" << endl;
     return false;
     }
     */

    // search triangle list for duplicates
    // usual trick for more efficient version using hash table but this time keys map to multiple values
    for(i = 0; i < (int) tris.size(); i++)
    {
        key = (long) tris[i].v[0] + (long) tris[i].v[1] + (long) tris[i].v[2]; // use sum of vertex indices as the key, not unique but a good start
        if(!(trilookup.find(key) == trilookup.end())) // key in map
        {
            // cerr << "possible dup" << endl;
            // iterate over triangles with the same key to see if any are duplicates
            auto range = trilookup.equal_range(key);
            for(auto it = range.first; it != range.second; it++) // now check if each truly is a duplicate, by looking at all values with the same key
            {
                // cerr << tris[i].v[0] << " " << tris[i].v[1] << " " << tris[i].v[2] << " ?= " << tris[it->second].v[0] << " " << tris[it->second].v[1] << " " << tris[it->second].v[2] << endl;
                if(sameTriangle(tris[i], tris[it->second]))
                {
                    cerr << "Error Mesh::manifoldValidity(): duplicate triangle found" << endl;
                    return false; // early out - exits on first duplicate encountered
                }
            }
        }
        trilookup.emplace(key, i); // add triangle index to multimap
    }

    // make sure every edge appears exactly twice in triangle list, with edges traversed in different directions
    // build list of triangles incident on each vertex - single pass over tris list
    for(v = 0; v < (int) verts.size(); v++)
        incident.push_back(incempty);
    for(t = 0; t < (int) tris.size(); t++)
        for(i = 0; i < 3; i++)
            incident[tris[t].v[i]].push_back(t);

    // make sure edges match up around each vertex. Each edge is shared by two triangles with opposite directions - single pass over incident list
    for(i = 0; i < (int) incident.size(); i++)
    {

        // note: this edge counting approach does not pick up cases where two surfaces touch at a single vertex
        edges.clear();
        // gather incident edges
        for(j = 0; j < (int) incident[i].size(); j++)
        {
            t = (incident[i])[j]; // index of incident triangle
            for(k = 0; k < 3; k++) // gather edges incident on vertex
            {
                if(tris[t].v[k] == i) // vertex for incidence, gather edge before and after
                {
                    edge1.v[0] = i; edge1.v[1] = tris[t].v[(k+1)%3]; // outgoing edge
                    edges.push_back(edge1);

                    // incoming edge
                    edge2.v[0] = tris[t].v[(k+2)%3]; edge2.v[1] = i; // incoming edge
                    edges.push_back(edge2);
                }
            }
        }

        // compare edges - O(n^2) but small n
        for(e = 0; e < (int) edges.size(); e++)
        {
            mcount = 0; ocount = 0;
            for(erest = 0; erest < (int) edges.size(); erest++)
            {
                if(e != erest)
                    if(sameEdge(edges[e], edges[erest], opposite))
                    {
                        if(opposite)
                            ocount++;
                        else
                            mcount++;
                    }
            }
            if(ocount != 1 || mcount != 0) // only one matching edge wound oppositely
            {
                cerr << "Error Mesh::manifoldValidity(): edges do not have exactly two incident triangles correctly wound" << endl;
                return false;
            }
        }

        // check for reachability - there should only be a single cycle around a vertex
        // more efficient if this was combined with the previous loop but less readable
        visited.clear(); visited.resize((int) incident[i].size(), false);
        e = 0; visited[0] = true; fin = false;
        while(!fin)
        {
            // find adjacent triangle to outgoing edge using incident edge table
            found = false; erest = 0;
            while(!found && erest < (int) edges.size())
            {
                if(e != erest)
                    if(sameEdge(edges[e], edges[erest], opposite))
                    {
                        found = true;
                        e = erest;

                        // check if triangle is already visited
                        if(visited[e/2])
                            fin = true;
                        else
                            visited[e/2] = true;

                        if(e % 2 == 0) // find outgoing edge, come in pairs
                            e++;
                        else
                            e--;
                    }
                erest++;
            }
        }

        for(j = 0; j < (int) incident[i].size(); j++)
        {
            if(!visited[j])
            {
                cerr << "Error Mesh::manifoldValidity(): vertices do note have a single cycle of incident triangles" << endl;
                return false;
            }
        }
    }

    // For true 2-manifold validity it would also be necessary to see if the object is self-intersecting by testing triangles against
    // each other for intersection. This would require a spatial data structure such as a bounding sphere hierarchy to accelerate properly
    // which is beyond the scope of this assignment
    return true;
}
