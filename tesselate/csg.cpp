//
// csg
//

#include "csg.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include <limits>
#include <stack>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace std;
// using namespace cgp;

GLfloat defaultCol[] = {0.243f, 0.176f, 0.75f, 1.0f};

void Scene::traverseTree(SceneNode* root, vector<ShapeNode *> & leaves){
	if (dynamic_cast<ShapeNode*>(root) != NULL){
		leaves.push_back(dynamic_cast<ShapeNode*>(root));
		cerr << "5" << endl;
	}
	else if (dynamic_cast<OpNode*>(root) != NULL){
		traverseTree(dynamic_cast<OpNode*>(root)->left,leaves);
		traverseTree(dynamic_cast<OpNode*>(root)->right,leaves);
	} 
}

bool Scene::genVizRender(View * view, ShapeDrawData &sdd)
{
    std::vector<ShapeNode *> leaves;
    int i;

    geom.clear();
    geom.setColour(defaultCol);

    // TO DO HERE, traverse csg tree pushing leaf nodes (shapes) to leaves vector
    // note: this displays all the constituent shapes in the tree but doesn't apply any set operations to them
    // so it is purely a pre-visualization
	
	traverseTree(csgroot,leaves);
	
	cerr << leaves.size() << endl;
	
    // traverse leaf shapes generating geometry
    for(i = 0; i < (int) leaves.size(); i++)
    {
        leaves[i]->shape->genGeometry(&geom, view);
    }

    // bind geometry to buffers and return drawing parameters, if possible
    if(geom.bindBuffers(view))
    {
        sdd = geom.getDrawParameters();
        return true;
    }
    else
        return false;
}

bool Scene::genVoxRender(View * view, ShapeDrawData &sdd)
{
    int x, y, z, xdim, ydim, zdim;
    glm::mat4 tfm, idt;
    glm::vec3 trs;
    cgp::Point pnt;

    geom.clear();
    geom.setColour(defaultCol);

    if(voxactive)
    {
        idt = glm::mat4(1.0f); // identity matrix

        vox.getDim(xdim, ydim, zdim);

        // place a sphere at filled voxels but subsample to avoid generating too many spheres
        for(x = 0; x < xdim; x+=10)
            for(y = 0; y < ydim; y+=10)
                for(z = 0; z < zdim; z+=10)
                {
                    if(vox.get(x, y, z))
                    {
                        pnt = vox.getVoxelPos(x, y, z); // convert from voxel space to world coordinates
                        trs = glm::vec3(pnt.x, pnt.y, pnt.z);
                        tfm = glm::translate(idt, trs);
                        geom.genSphere(voxsidelen * 5.0f, 3, 3, tfm);
                    }
                }

    }

    // bind geometry to buffers and return drawing parameters, if possible
    if(geom.bindBuffers(view))
    {
        sdd = geom.getDrawParameters();
        return true;
    }
    else
        return false;
}

Scene::Scene()
{
    csgroot = NULL;
    col = defaultCol;
    voldiag = cgp::Vector(20.0f, 20.0f, 20.0f);
    voxsidelen = 0.0f;
    voxactive = false;
}

Scene::~Scene()
{
    clear();
}

void Scene::clear()
{
    geom.clear();
    vox.clear();

    // TO DO HERE, code to walk csg tree and deallocate nodes
    // will require dynamic casting of SceneNode pointers
   	clearTree(csgroot);
   	csgroot = NULL; 
    
}

void Scene::clearTree(SceneNode * root){
	if (dynamic_cast<OpNode*>(root) != NULL){
		clearTree(dynamic_cast<OpNode*>(root)->left);
		clearTree(dynamic_cast<OpNode*>(root)->right);
		root = NULL;
	}
	else if (dynamic_cast<ShapeNode*>(root) != NULL){
		delete root;
	}	
}


bool Scene::bindGeometry(View * view, ShapeDrawData &sdd)
{
    if(voxactive)
    {
        return genVoxRender(view, sdd);
    }
    else
        return genVizRender(view, sdd);
}

void Scene::voxSetOp(SetOp op, VoxelVolume *leftarg, VoxelVolume *rightarg)
{
    // stub, needs completing
    bool left, right;
    //cerr<<"here"<<endl;
    for (int i=0; i<leftarg->getdimX(); i++){
    	for (int j=0; j<leftarg->getdimY(); j++){
    		for (int k=0; k<leftarg->getdimZ(); k++){
    			left = leftarg->get(i,j,k);
    			right = rightarg->get(i,j,k);
				if (op == SetOp::UNION){
					leftarg->set(i,j,k, left | right);
				}
				else if (op == SetOp::INTERSECTION){
					leftarg->set(i,j,k, left & right);
				}
				else if (op == SetOp::DIFFERENCE){
					leftarg->set(i,j,k, left - right);
				}
			}
		}
    }
    
}

void Scene::traverseTree2(SceneNode* root, vector<OpNode *> & leaves){
	if (dynamic_cast<OpNode*>(root) != NULL){
		traverseTree2(dynamic_cast<OpNode*>(root)->left,leaves);
		traverseTree2(dynamic_cast<OpNode*>(root)->right,leaves);
		leaves.push_back(dynamic_cast<OpNode*>(root));
	} 
}

VoxelVolume* Scene::setVoxel(float voxlen){
	VoxelVolume* vox1 = new VoxelVolume();
	int xdim, ydim, zdim;
	cerr << voxlen << endl;
    // calculate voxel volume dimensions based on voxlen
    xdim = ceil(voldiag.i / voxlen)+2; // needs a 1 voxel border to ensure a closed mesh if shapes reach write up to the border
    ydim = ceil(voldiag.j / voxlen)+2;
    zdim = ceil(voldiag.k / voxlen)+2;
    voxsidelen = voxlen;
    voxactive = true;

    cgp::Vector voxdiag = cgp::Vector((float) xdim * voxlen, (float) ydim * voxlen, (float) zdim * voxlen);
    cgp::Point voxorigin = cgp::Point(-0.5f*voxdiag.i, -0.5f*voxdiag.j, -0.5f*voxdiag.k);
    vox1->setDim(xdim, ydim, zdim);
    vox1->setFrame(voxorigin, voxdiag);
    
    return vox1;
}

void Scene::voxWalk(SceneNode *root, VoxelVolume *voxels)
{
    // stub, needs completing
    // will require dynamic casting of SceneNode pointers
    
    std::vector<ShapeNode *> leaves;
    traverseTree(root,leaves);
    
    std::vector<OpNode *> nodes;
    traverseTree2(root,nodes);
    
    vector<VoxelVolume*> tempVoxVols;
    
    for (int l=0; l<(int)leaves.size(); l++){
    	VoxelVolume* voxelsTemp = setVoxel(0.05f);
    	for (int i=0; i<voxelsTemp->getdimX(); i++){
			for (int j=0; j<voxelsTemp->getdimY(); j++){
				for (int k=0; k<voxelsTemp->getdimZ(); k++){
					
					if (dynamic_cast<ShapeNode*>(leaves[l])->shape->pointContainment(voxelsTemp->getVoxelPos(i,j,k))){
						voxelsTemp->set(i,j,k,true);
					}
					else{
						voxelsTemp->set(i,j,k,false);
					}
				}
			}
		}
		voxVols.push_back(voxelsTemp);
		tempVoxVols.push_back(voxelsTemp);
    }
    //voxels = voxVols[0];
    
    for (int i=(int)nodes.size()-1; i>=0; i--){
    	VoxelVolume* temp1 =  voxVols[0];
    	VoxelVolume* temp2 = voxVols[1];
    	OpNode* tempNode = nodes[i];
    	cerr << "did" << endl;
    	voxSetOp(tempNode->op, temp1, temp2);
    	voxVols.erase(voxVols.begin()+1);
    	voxVols[0] = temp1;
    	
    	//voxels = temp1;
    	//break;
    }
    
    for (int i=0; i<voxVols[0]->getdimX(); i++){
		for (int j=0; j<voxVols[0]->getdimY(); j++){
			for (int k=0; k<voxVols[0]->getdimZ(); k++){
				
				if (voxVols[0]->get(i,j,k)){
					voxels->set(i,j,k,true);
				}
				else{
					voxels->set(i,j,k,false);
				}
			}
		}
	}
    
    //voxels = voxVols[0];
    
    
    /*if (dynamic_cast<OpNode*>(root) != NULL){
    	currentOp = dynamic_cast<OpNode*>(root)->op;
    	if (dynamic_cast<ShapeNode*>(root)->left != NULL && dynamic_cast<ShapeNode*>(root)->right != NULL){
    		currentLeft = dynamic_cast<ShapeNode*>(root)->left;
    		currentRight = dynamic_cast<ShapeNode*>(root)->right;
    	}*/
		//voxWalk(dynamic_cast<OpNode*>(root)->left,voxels);
		/*currentOp = dynamic_cast<OpNode*>(root)->op;
    	if (dynamic_cast<OpNode*>(root)->left != NULL && dynamic_cast<ShapeNode*>(root)->right != NULL){
    		//currentLeft = dynamic_cast<ShapeNode*>(root)->left;
    		currentRight = dynamic_cast<ShapeNode*>(root)->right;
    	}*/
		/*voxWalk(dynamic_cast<OpNode*>(root)->right,voxels);
	} 
	else if (dynamic_cast<ShapeNode*>(root) != NULL){
		
		for (int i=0; i<voxels->getdimX(); i++){
			for (int j=0; j<voxels->getdimY(); j++){
				for (int k=0; k<voxels->getdimZ(); k++){
					
					if (dynamic_cast<ShapeNode*>(root)->shape->pointContainment(voxels->getVoxelPos(i,j,k))){
						voxels->set(i,j,k,true);
					}
					else{
						voxels->set(i,j,k,false);
					}
				}
			}
		}
	}*/
}


void Scene::voxelise(float voxlen)
{
    int xdim, ydim, zdim;
	cerr << voxlen << endl;
    // calculate voxel volume dimensions based on voxlen
    xdim = ceil(voldiag.i / voxlen)+2; // needs a 1 voxel border to ensure a closed mesh if shapes reach write up to the border
    ydim = ceil(voldiag.j / voxlen)+2;
    zdim = ceil(voldiag.k / voxlen)+2;
    voxsidelen = voxlen;
    voxactive = true;

    cgp::Vector voxdiag = cgp::Vector((float) xdim * voxlen, (float) ydim * voxlen, (float) zdim * voxlen);
    cgp::Point voxorigin = cgp::Point(-0.5f*voxdiag.i, -0.5f*voxdiag.j, -0.5f*voxdiag.k);
    vox.setDim(xdim, ydim, zdim);
    vox.setFrame(voxorigin, voxdiag);

    cerr << "Voxel volume dimensions = " << xdim << " x " << ydim << " x " << zdim << endl;

    // actual recursive depth-first walk of csg tree
    if(csgroot != NULL)
        voxWalk(csgroot, &vox);
}

void Scene::sampleScene()
{
    ShapeNode * sph = new ShapeNode();
    sph->shape = new Sphere(cgp::Point(0.0f, 0.0f, 0.0f), 4.0f);

    ShapeNode * cyl1 = new ShapeNode();
    cyl1->shape = new Cylinder(cgp::Point(-7.0f, -7.0f, 0.0f), cgp::Point(7.0f, 7.0f, 0.0f), 2.0f);

    ShapeNode * cyl2 = new ShapeNode();
    cyl2->shape = new Cylinder(cgp::Point(0.0f, -7.0f, 0.0f), cgp::Point(0.0f, 7.0f, 0.0f), 2.5f);

    OpNode * combine = new OpNode();
    combine->op = SetOp::UNION;
    combine->left = sph;
    combine->right = cyl1;

    OpNode * diff = new OpNode();
    diff->op = SetOp::DIFFERENCE;
    diff->left = combine;
    diff->right = cyl2;

    csgroot = diff;
}

void Scene::expensiveScene()
{
    ShapeNode * sph = new ShapeNode();
    sph->shape = new Sphere(cgp::Point(1.0f, -2.0f, -2.0f), 3.0f);

    ShapeNode * cyl = new ShapeNode();
    cyl->shape = new Cylinder(cgp::Point(-7.0f, -7.0f, 0.0f), cgp::Point(7.0f, 7.0f, 0.0f), 2.0f);

    ShapeNode * mesh = new ShapeNode();
    Mesh * bunny = new Mesh();
    bunny->readSTL("../meshes/bunny.stl");
    bunny->boxFit(10.0f);
    mesh->shape = bunny;

    OpNode * combine = new OpNode();
    combine->op = SetOp::UNION;
    combine->left = mesh;
    combine->right = cyl;

    OpNode * diff = new OpNode();
    diff->op = SetOp::DIFFERENCE;
    diff->left = combine;
    diff->right = mesh;

    csgroot = diff;
}

SceneNode* Scene::getRoot(){
	return csgroot;
}
