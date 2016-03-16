//
//  Cheb.hpp
//  Assignment2
//
//  Created by Brandon Clark on 2/7/16.
//
//

#ifndef Cheb_hpp
#define Cheb_hpp

#include <stdio.h>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>
#include <vector>
#include "MatrixStack.h"
#include "Program.h"
#include "Shape.h"

class Cheb
{
public:
    
    Cheb(std::string objFile, std::string attachmentFile,
         std::string skeletonFile, Eigen::Vector3f trans);
    virtual ~Cheb();
    
    void animateCheb();
    void init();
    void draw(std::shared_ptr<MatrixStack> M, std::shared_ptr<Program> prog);
    void drawGPU(std::shared_ptr<MatrixStack> M, std::shared_ptr<Program> prog);
    
    std::vector<float> getVertPos();
    
private:
    int numVerts, numBones, numFrames;
    std::shared_ptr<Shape> shape;
    Eigen::Vector3f currentTrans;
    
    unsigned weightBufID;
    unsigned boneBufID;
    unsigned numBoneInfID;
    
    std::vector<float> origPos;
    std::vector<float> origNor;
    std::vector<float> skinningWeights;
    std::vector<int> boneIndices;
    std::vector<int> numBoneInfluences;
    std::vector<Eigen::Matrix4f> bindPose;
    std::vector<std::vector<Eigen::Matrix4f>> animTrans;
    
    void loadAttachment(std::string attachmentFile);
    void loadSkeleton(std::string skeletonFile);
    void drawMesh(std::shared_ptr<MatrixStack> M, std::shared_ptr<Program> prog);
    void drawMeshGPU(std::shared_ptr<MatrixStack> M, std::shared_ptr<Program> prog);
    
};

#endif /* Cheb_hpp */
