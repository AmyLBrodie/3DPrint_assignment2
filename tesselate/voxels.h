#ifndef _VOXELVOLUME
#define _VOXELVOLUME
/**
 * @file
 *
 * VoxelVolume class for storing a 3d cuboid of binary voxel data
 */


#include <vector>
#include <stdio.h>
#include <iostream>
#include "vecpnt.h"

/**
 * A cuboid volume regularly subdivided into uniformly sized cubes (voxels). A bit packing approach can used to compress storage.
 */
class VoxelVolume
{
private:
    int * voxgrid;  ///< flattened voxel volume, bit packed to save memory
    int xdim;       ///< number of voxels in x dimension
    int ydim;       ///< number of voxels in y dimension
    int zdim;       ///< number of voxels in z dimension

    cgp::Point origin;     ///< corner point in world space
    cgp::Vector diagonal;  ///< diagonal extent of the volume in world space
    cgp::Vector cell;      ///< diagonal extent of a single voxel cell

    /// Calculate the diagonal extent of a single cell and store internally
    void calcCellDiag();

public:

    /// Default constructor
    VoxelVolume();

    /**
     * Create voxel volume with specified dimensions. x dimension is padded to be divisible by 32
     * @param xsize, ysize, zsize      number of voxels in x, y, z dimensions
     * @param corner  origin position of the volume
     * @param diag     diagonal extent of the volume
     */
    VoxelVolume(int xsize, int ysize, int zsize, cgp::Point corner, cgp::Vector diag);

    /// Destructor
    ~VoxelVolume();

    /**
     * Delete voxel volume grid
     */
    void clear();

    /**
     * Set all voxel elements in volume to empty or occupied
     * @param setval    new value for all voxel elements, either empty (false) or occupied (true)
     * @todo VoxelVolume::fill to be completed for CGP Assignment2
     */
    void fill(bool setval);

    /**
     * Obtain the dimensions of the voxel volume
     * @param dimx, dimy, dimz     number of voxels in x, y, z dimensions
     */
    void getDim(int &dimx, int &dimy, int &dimz);

    /**
     * Set the dimensions of the voxel volume and allocate memory accordingly
     * @param dimx, dimy, dimz     number of voxels in x, y, z dimensions
     * @todo VoxelVolume::setDim to be completed for CGP Assignment2
     */
    void setDim(int dimx, int dimy, int dimz);

    /**
     * Getter for the placement and dimensions of the volume in 3d space
     * @param corner    bottom, front, left corner of the volume
     * @param diag      diagonal vector across the volume
     */
    void getFrame(cgp::Point &corner, cgp::Vector &diag);

    /**
     * Setter for the placement and dimensions of the volume in 3d space
     * @param corner    bottom, front, left corner of the volume
     * @param diag      diagonal vector across the volume
     */
    void setFrame(cgp::Point corner, cgp::Vector diag);

    /**
     * Set a single voxel element to either empty or occupied
     * @param x, y, z   3D location, zero indexed
     * @param setval    new voxel value, either empty (false) or occupied (true)
     * @retval true if the voxel is within volume bounds,
     * @retval false otherwise.
     * @todo VoxelVolume::set to be completed for CGP Assignment2
     */
    bool set(int x, int y, int z, bool setval);

    /**
     * Get the status of a single voxel element at the specified position
     * @param x, y, z   3D location, zero indexed
     * @retval true if the voxel is occupied,
     * @retval false if the voxel is empty.
     * @todo VoxelVolume::get to be completed for CGP Assignment2
     */
    bool get(int x, int y, int z);

    /**
     * Find the world-space position of the centre of a voxel
     * @param x, y, z   3D location, zero indexed
     * @returns voxel centre point 
     */
    cgp::Point getVoxelPos(int x, int y, int z);
};

#endif
