//
//  AnimatedCharacter.hpp
//  FinalProject
//
//  Created by Brandon Clark on 3/12/16.
//
//

#ifndef AnimatedCharacter_hpp
#define AnimatedCharacter_hpp

#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>
#include <stdio.h>
#include <stdlib.h>
#include "GLSL.h"
#include <iostream>
#include <vector>

#define EIGEN_DONT_ALIGN_STATICALLY
#include <Eigen/Dense>
#include <Eigen/Sparse>

class MatrixStack;
class Program;

using namespace std;

class AnimatedCharacter
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    AnimatedCharacter(const string file, int inTexNum, float hiddenScale, int daeToBe);
    virtual ~AnimatedCharacter();
    
    void drawChar(std::shared_ptr<MatrixStack> MV, const std::shared_ptr<Program> p, float time);
    void startAnimation(string animation);
    bool isAnimating();
    
    bool randomStart;
    int animChoice;
    int lastAnim;
    Eigen::Vector3f position;
    Eigen::Vector3f scale;
    float rotate;
    GLuint texBuf;
    
private:
    aiScene* scene;
    const aiAnimation* sceneAnim;
    
    aiNode* root;
    aiMesh** meshes;
    Eigen::Vector3f hiddenScale;
    
    unsigned int numInd;
    unsigned int* indices;
    unsigned int boneCount;
    // keep track of all the bones
    float* normals;
    float* positions;
    float* texture;
    
    unsigned int* numBones; // number of bones affecting a given vertex
    unsigned int* boneId; // 4 per vertex
    float* boneWeight; // 4 per vertex
    unsigned int* boneId2; // 4 per vertex
    float* boneWeight2; // 4 per vertex
    float* floatModel;
    
    float lastTime;
    float animStart;
    float endTime;
    
    int daeType;
    int meshInd;
    int texInd;
    
    GLuint posBuf;
    GLuint norBuf;
    GLuint indBuf;
    GLuint boneIdBuf;
    GLuint boneWeightBuf;
    GLuint boneIdBuf2;
    GLuint boneWeightBuf2;
    
    void recursivePrint(aiNode* toPrint, int level, aiMesh** meshes);
    void recursiveUpdate(aiNode* toUpdate, float time);
    
    // methods to interpolate transform between animation frames
    aiQuaternion intRot(float time, const aiNodeAnim* nodeAnim);
    aiVector3D intScale(float time, const aiNodeAnim* nodeAnim);
    aiVector3D intTrans(float time, const aiNodeAnim* nodeAnim);
    void updateBones(float time);
    const aiNodeAnim* findNodeAnim(const aiAnimation* anim, const aiNode* toFind);
    
    
    int findBone(const aiNode* toFind);
};

#endif /* AnimatedCharacter_hpp */
