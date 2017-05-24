//
// Voxels
//

#include "voxels.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <limits>

using namespace std;

VoxelVolume::VoxelVolume()
{
    xdim = ydim = zdim = 0;
    voxgrid = NULL;
    setFrame(cgp::Point(0.0f, 0.0f, 0.0f), cgp::Vector(0.0f, 0.0f, 0.0f));
}

VoxelVolume::VoxelVolume(int xsize, int ysize, int zsize, cgp::Point corner, cgp::Vector diag)
{
    voxgrid = NULL;
    setDim(xsize, ysize, zsize);
    setFrame(corner, diag);
}

VoxelVolume::~VoxelVolume()
{
    clear();
}

void VoxelVolume::clear()
{
    if(voxgrid != NULL)
    {
        delete [] voxgrid;
        voxgrid = NULL;
    }
}

void VoxelVolume::fill(bool setval)
{
    // stub, needs completing
	for (int i=0; i<xdim; i++){
		for (int j=0; j<ydim; j++){
			for (int k=0; k<ydim; k++){
				cerr << i << endl;
				set(i,j,k,setval);
			}
		}
	}
}

void VoxelVolume::calcCellDiag()
{
    if(xdim > 0 && ydim > 0 && zdim > 0)
        cell = cgp::Vector(diagonal.i / (float) xdim, diagonal.j / (float) ydim, diagonal.k / (float) zdim);
    else
        cell = cgp::Vector(0.0f, 0.0f, 0.0f);
}

void VoxelVolume::getDim(int &dimx, int &dimy, int &dimz)
{
    dimx = xdim; dimy = ydim; dimz = zdim;
}

void VoxelVolume::setDim(int dimx, int dimy, int dimz)
{
    // stub, needs completing
    xdim = dimx;
    ydim = dimy;
    zdim = dimz;
    
    voxgrid = new int [(xdim*ydim*zdim)]();

    calcCellDiag();
}

void VoxelVolume::getFrame(cgp::Point &corner, cgp::Vector &diag)
{
    corner = origin;
    diag = diagonal;
}

void VoxelVolume::setFrame(cgp::Point corner, cgp::Vector diag)
{
    origin = corner;
    diagonal = diag;
    calcCellDiag();
}

bool VoxelVolume::set(int x, int y, int z, bool setval)
{
    // stub, needs completing
   	if (voxgrid == NULL){
   		return false;
   	}
   	
   	int coordinates = (xdim*ydim*x+ydim*y+z);
   	int bitIndex = coordinates%32;
   	int mask = 0x32 >> bitIndex;
   	
   	/*if (setval==true) {
   		voxgrid[coordinates] |= mask;
   	}
   	else{
   		mask = ~mask;
   		voxgrid[coordinates] &= mask;
   	}*/
   	
   	if (setval==true) {
   		voxgrid[coordinates] = 1;
   	}
   	else{
   		//mask = ~mask;
   		voxgrid[coordinates] = 0;
   	}
   	
   	if (x > xdim || y > ydim || z > zdim){
   		return false;
   	}
   	else{
   		return true;
   	}
}

bool VoxelVolume::get(int x, int y, int z)
{
    // stub, needs completing
    if (voxgrid == NULL){
   		return false;
   	}
   	
   	int coordinates = (xdim*ydim*x+ydim*y+z);
   	int bitIndex = coordinates%32;
   	int mask = 0x32 >> bitIndex;
   	//bool flag = (mask & voxgrid[coordinates]) > 0;
   	bool flag;
   	if (voxgrid[coordinates] == 0){
   		flag = false;
   		//cerr << "33" << endl;
   	}
   	else{
   		flag = true;
   		//cerr << "22" << endl;
   	}
   	return flag;
}

cgp::Point VoxelVolume::getVoxelPos(int x, int y, int z)
{
    cgp::Point pnt;
    cgp::Vector halfcell;
    float px, py, pz;

    px = (float) x / (float) xdim;
    py = (float) y / (float) ydim;
    pz = (float) z / (float) zdim;

    pnt = cgp::Point(origin.x + px * diagonal.i + 0.5f * cell.i, origin.y + py * diagonal.j + 0.5f * cell.j, origin.z + pz * diagonal.k + 0.5f * cell.k); // convert from voxel space to world coordinates
    return pnt;
}

int VoxelVolume::getdimX(){
	return xdim;
}
    
int VoxelVolume::getdimY(){
	return ydim;
}
    
int VoxelVolume::getdimZ(){
	return zdim;
}
