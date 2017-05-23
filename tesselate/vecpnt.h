/**
 * @file
 *
 * Basic vector and point arithmetic library. Inlined for efficiency. Also includes some simple geometry routines.
*/
// changes: included helpful geometry routines (2006)
#ifndef _INC_VECPNT
#define _INC_VECPNT

#include <math.h>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/split_member.hpp>

#define pluszero 0.000001f
#define minuszero -0.000001f
#define PI 3.14159265
#define PI2 6.2831853

inline float sign(float n)
{
    if(n >= 0.0f)
        return 1.0f;
    else
        return -1.0f;
}

namespace cgp {

class Point
{
private:
    friend class boost::serialization::access;
    /// Boost serialization
    template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {
        ar & x;
        ar & y;
        ar & z;
    }

public:

    float x, y, z;

    /// default constructor, sets point to origin
    inline Point()
    { x = 0.0;
      y = 0.0;
      z = 0.0;
    }

    /// constructor assigns (a,b,c) to (x,y,z) fields of point
    inline Point(float a, float b, float c)
    { x = a;
      y = b;
      z = c;
    }

    /// copy assignment
    inline Point & operator =(const Point & from)
    {
      x = from.x;
      y = from.y;
      z = from.z;
      return *this;
    }

    /**
     * Determine the distance between two points
     * @param p distance from this point to the current one
     * @return euclidean distance
     */
    inline double dist(Point p)
    { float dx, dy, dz;

      dx = (p.x - x) * (p.x - x);
      dy = (p.y - y) * (p.y - y);
      dz = (p.z - z) * (p.z - z);
      return sqrt(dx + dy + dz);
    }

    /**
     * Weighted affine combination of two points
     * @param c1        weight assigned to first point
     * @param p1        first point in affine combination
     * @param c2        weight assigned to second point
     * @param p2        second point in affine combination
     */
    inline void affinecombine(float c1, Point& p1, float c2, Point& p2)
    { x = c1 * p1.x + c2 * p2.x;
      y = c1 * p1.y + c2 * p2.y;
      z = c1 * p1.z + c2 * p2.z;
    }

    /// Test the equality of two points within a given tolerance
    inline bool operator == (Point p)
    { float dx, dy, dz;
      dx = x - p.x;
      dy = y - p.y;
      dz = z - p.z;
      return ((dx < pluszero) && (dx > minuszero) && (dy < pluszero) &&
              (dy > minuszero) && (dz < pluszero) && (dz > minuszero));
    }
};

class Vector
{
private:

    friend class boost::serialization::access;
    /// Boost serialization
    template<class Archive> void serialize(Archive & ar, const unsigned int version)
    {
        ar & i;
        ar & j;
        ar & k;
    }

public:

    float i, j, k;

    /// Default constructor
    inline Vector()
    { i = 0.0;
      j = 0.0;
      k = 0.0;
    }

    /// copy assignment
    Vector& operator=(const Vector &v)
    {
        i = v.i;
        j = v.j;
        k = v.k;

        return *this;
    }


    /// Constructor assigns (a,b,c) to (i,j,k) fields of vector
    inline Vector(float a, float b, float c)
    { i = a;
      j = b;
      k = c;
    }

    /**
     * Find the angle (in radians) between two vectors
     * @param b vector forming an angle with the current vector
     * @return  angle in radians
     */
    inline float angle(Vector b)
    { Vector a;

      a = (* this);
      a.normalize();
      b.normalize();
      return (float) acos(a.dot(b));
    }

    /// Return the euclidian length of a vector
    inline float length()
    { float len;

      len = i * i + j * j + k * k;
      len = sqrt(len);
      return len;
    }

    /// Return the square of the length of a vector. Avoids costly square root computation
    inline float sqrdlength()
    { float len;

      len = i * i;
      len += j * j;
      len +=  k * k;
      return len;
    }

    /// Transform vector to unit length
    inline void normalize()
    {
        float len;
        len = (float) sqrt(i*i + j*j + k*k);
        if(len > 0.0f)
            len = 1.0f / len;
        i *= len;
        j *= len;
        k *= len;
    }

/*
    inline float lnormalize()
    {
        float len;
        len = (float) sqrt(i*i + j*j + k*k);
        if(len > 0.0f)
            len = 1.0f / len;
        i *= len;
        j *= len;
        k *= len;
        return len;
    }
*/
    /// Scale the vector by a scalar factor c
    inline void mult (float c)
    { i = i * c;
      j = j * c;
      k = k * c;
    }

    /// Component-wise multiplication with another vector v
    inline void mult (Vector &v)
    {
        i *= v.i;
        j *= v.j;
        k *= v.k;
    }

    /// The vector result of dividing vector a by vector b
    inline void div(Vector& a, Vector& b)
    { i = a.i / b.i;
      j = a.j / b.j;
      k = a.k / b.k;
    }

    /// Convert a point p into a vector
    inline void pntconvert(Point& p)
    { i = p.x;
      j = p.y;
      k = p.z;
    }

    /// Return true if two vectors are identical within a tolerance
    inline int operator ==(Vector v)
    { double di, dj, dk;

      di = i - v.i;
      dj = j - v.j;
      dk = k - v.k;

      return ((di < pluszero) && (di > minuszero) && (dj < pluszero) && (dj > minuszero)
               && (dk < pluszero) && (dk > minuszero));
    }

    /// Difference of points: create a vector from point p to point q
    inline void diff(Point p, Point q)
    { i = (q.x - p.x);
      j = (q.y - p.y);
      k = (q.z - p.z);
    }


    /// Component-wise addition of a vector v to the current vector
    inline void add(Vector& v)
    { i += v.i;
      j += v.j;
      k += v.k;
    }

    /// Component-wise subtraction of a vector v fromthe current vector
    inline void sub(Vector& v)
    { i -= v.i;
      j -= v.j;
      k -= v.k;
    }

    /// Generate the cross product of two vectors
    inline void cross(Vector& x, Vector& y)
    { i = (x.j * y.k) - (x.k * y.j);
      j = (x.k * y.i) - (x.i * y.k);
      k = (x.i * y.j) - (x.j * y.i);
    }

    /// Generate the dot product of two vectors
    inline float dot(Vector& v)
    { return ((i * v.i) + (j * v.j) + (k * v.k));
    }

    /// Find the point at the head of a vector which is placed with its tail at p
    inline void pntplusvec(Point& p, Point * r)
    { r->x = p.x + i;
      r->y = p.y + j;
      r->z = p.z + k;
    }

    /// Create an affine combination of two vectors v1 and v2, weighted by c1 and c2
    inline void affinecombine(float c1, Vector &v1, float c2, Vector &v2)
    { i = c1 * v1.i + c2 * v2.i;
      j = c1 * v1.j + c2 * v2.j;
      k = c1 * v1.k + c2 * v2.k;
    }

    /// Interpolate between vectors s and e using linear parameter t
    inline void interp(Vector s, Vector e, float t)
    {
        i = (1.0f - t)*s.i + t*e.i;
        j = (1.0f - t)*s.j + t*e.j;
        k = (1.0f - t)*s.k + t*e.k;
    }

    /// Turn the vector in the x-y plane by an angle a in radians
    inline void rotate(float a)
    {
        float ni, nj, ca, sa;

        ca = cos(a); sa = sin(a);
        ni = i * ca - j * sa;
        nj = j * ca + i * sa;
        i = ni; j = nj;
    }
};


/**
 * Bounding box in 3D space. Useful for accelerating geometric operations such as intersection.
 */
class BoundBox
{

public:

    Point min; ///< minimal vertex (bottom, left, front cornder of bounding box)
    Point max; ///< maximal vertex (top, right, back corner of bounding box)

    BoundBox(){ reset(); }

    /// Initialize bounding box to its default settting
    inline void reset()
    {
        min = Point(HUGE_VALF, HUGE_VALF, HUGE_VALF);
        max = Point(-HUGE_VALF, -HUGE_VALF, -HUGE_VALF);
    }

    /// Compare point against current bounding box and expand as required
    inline void includePnt(Point pnt)
    {
        if(pnt.x < min.x)
            min.x = pnt.x;
        if(pnt.x > max.x)
            max.x = pnt.x;
        if(pnt.y < min.y)
            min.y = pnt.y;
        if(pnt.y > max.y)
            max.y = pnt.y;
        if(pnt.z < min.z)
            min.z = pnt.z;
        if(pnt.z > max.z)
            max.z = pnt.z;
    }

    /// Return the length of the diagonal of the bounding box
    inline float diagLen()
    {
        Vector diag;

        diag.diff(min, max);
        return diag.length();
    }

    /// Return the diagonal of the bounding box
    Vector getDiag()
    {
        Vector diag;

        diag.diff(min, max);
        return diag;
    }

    /// Return true if the bounding box is at default initial values
    inline bool empty()
    {
        Point absmin, absmax;

        absmax = Point(HUGE_VALF, HUGE_VALF, HUGE_VALF);
        absmin = Point(-HUGE_VALF, -HUGE_VALF, -HUGE_VALF);
        return (min == absmax && max == absmin);
        // (min.x ==  && min.y == 0.0f && min.z == farextent && max.x == -1.0f*farextent && max.y == 0.0f && max.z == -1.0f*farextent);
    }

    /// Enlarge the bounding box uniformly in all directions
    inline void expand(float extent)
    {
        max.x += extent; min.x -= extent;
        max.y += extent; min.y -= extent;
        max.z += extent; min.z -= extent;
    }
};

}

////
//// USEFUL GEOMETRY ROUTINES
////

// rayPointDist:   Find the shortest distance from a point <query> to a line segment, represented by an origin <start>
//                  and direction <dirn>. Return the shortest distance from the line segment to the point as <dist> and
//                  the parameter value on the line segment of this intersection as <tval>.
void rayPointDist(cgp::Point start, cgp::Vector dirn, cgp::Point query, float &tval, float &dist);

// clamp: ensure that parameter <t> falls in [0,1]
void clamp(float & t);
#endif
