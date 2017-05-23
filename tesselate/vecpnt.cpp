#include "vecpnt.h"
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace cgp;

void rayPointDist(cgp::Point start, Vector dirn, cgp::Point query, float &tval, float &dist)
{
    float den;
    cgp::Point closest;
    cgp::Vector closevec;

    den = dirn.sqrdlength();
    if(den == 0.0f) // not a valid line segment
        dist = -1.0f;
    else
    {
        // get parameter value of closest point
        tval = dirn.i * (query.x - start.x) + dirn.j * (query.y - start.y) + dirn.k * (query.z - start.z);
        tval /= den;

        // find closest point on line
        closevec = dirn;
        closevec.mult(tval);
        closevec.pntplusvec(start, &closest);
        closevec.diff(query, closest);
        dist = closevec.length();
    }
}

void clamp(float & t)
{
    if(t > 1.0f+minuszero)
        t = 1.0f+minuszero;
    if(t < 0.0f)
        t = 0.0f;
}
