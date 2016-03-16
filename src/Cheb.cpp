//
//  Cheb.cpp
//  Assignment2
//
//  Created by Brandon Clark on 2/7/16.
//
//

#include "Cheb.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "GLSL.h"

#include <iostream>
#include <fstream>
//#include <sstream>

using namespace Eigen;
using namespace std;

int NUM_BONE_INFLUENCES = 16;
double ANIM_SPEED = 50.0;

Cheb::Cheb(string objFile, string attachmentFile, string skeletonFile, Vector3f trans)
{
    numVerts = numBones = numFrames = 0;
    currentTrans = trans;
    shape = make_shared<Shape>();
    shape->loadMesh(objFile);
    shape->init();
    origPos = shape->getPosBuf();
    origNor = shape->getNorBuf();
    
    weightBufID = 0;
    boneBufID = 0;
    numBoneInfID = 0;
    
    loadAttachment(attachmentFile);
    loadSkeleton(skeletonFile);
}

Cheb::~Cheb()
{
}

void Cheb::loadAttachment(string attachmentFile)
{
    ifstream in;
    in.open(attachmentFile);
    if(!in.good()) {
        std::cout << "Cannot read " << attachmentFile << endl;
        return;
    }

    string line;

    getline(in, line);
    while(!in.eof() && (line.at(0) == '#' || line.size() < 2))
    {
        getline(in, line);
    }
    
    stringstream ss(line);
    ss >> numVerts;
    ss >> numBones;
    skinningWeights.resize(numVerts*numBones);
    skinningWeights.clear();
    
    int vertNdx = 0;
    int boneNdx = 0;
    int boneInfluenceNdx = 0;
    int count = 0;

    boneIndices.resize(numVerts * 16);//numBones);
    numBoneInfluences.resize(numVerts);
    std::fill(boneIndices.begin(), boneIndices.end(), 0);
    std::fill(numBoneInfluences.begin(), numBoneInfluences.end(), 0);
    
    while(1) {
        getline(in, line);
        if(in.eof()) {
            break;
        }
        // Skip empty lines
        if(line.size() < 2) {
            continue;
        }
        // Skip comments
        if(line.at(0) == '#') {
            continue;
        }
        
        // Parse line
        stringstream ss(line);
        for (int j = 0; j < numBones; ++j)
        {
            float weight;
            ss >> weight;
            skinningWeights.push_back(weight);
            
            if (weight > 0.0)
            {
                boneIndices[count] = j;
                numBoneInfluences[vertNdx]++;
                count++;
            }
            
            boneNdx++;
        }
        boneInfluenceNdx += 16;
        vertNdx++;
//        count = boneNdx;
        count = boneInfluenceNdx;
    }
    in.close();
}

void Cheb::loadSkeleton(string skeletonFile)
{
    ifstream in;
    in.open(skeletonFile);
    if(!in.good()) {
        std::cout << "Cannot read " << skeletonFile << endl;
        return;
    }
    
    string line;
    
    getline(in, line);
    while(!in.eof() && (line.at(0) == '#' || line.size() < 2))
    {
        getline(in, line);
    }
    
    stringstream ss(line);
    ss >> numFrames;
    ss >> numBones;
    
    bindPose.resize(numBones);
    bindPose.clear();
    animTrans.resize(numFrames);
    animTrans.clear();
    
    int frameCount = 0;
    int lineCount = 1;
    while(1) {
        getline(in, line);
        if(in.eof()) {
            break;
        }
        // Skip empty lines
        if(line.size() < 2) {
            continue;
        }
        // Skip comments
        if(line.at(0) == '#') {
            continue;
        }
        // Parse line
        stringstream ss(line);
        
        if (lineCount == 1)
        {
            for (int j = 0; j < numBones; ++j)
            {
                float qx, qy, qz, qw;
                float px, py, pz;
                
                ss >> qx;
                ss >> qy;
                ss >> qz;
                ss >> qw;
                
                ss >> px;
                ss >> py;
                ss >> pz;
                
                MatrixXf p(3,1);
                p << px, py, pz;
                
                Quaternionf q;
                q.vec() << qx, qy, qz;
                q.w() = qw;
                    
                Matrix4f E = Matrix4f::Identity();
                E.block<3,3>(0,0) = q.toRotationMatrix();
                E.block<3,1>(0,3) = p;
                
                bindPose.push_back(E.inverse());
            }
            lineCount++;
        }
        else
        {
            animTrans[frameCount].resize(numBones);
            animTrans[frameCount].clear();
            for (int j = 0; j < numBones; ++j)
            {
                float qx, qy, qz, qw;
                float px, py, pz;
                
                ss >> qx;
                ss >> qy;
                ss >> qz;
                ss >> qw;
                
                ss >> px;
                ss >> py;
                ss >> pz;
                
                MatrixXf p(3,1);
                p << px, py, pz;
                
                Quaternionf q;
                q.vec() << qx, qy, qz;
                q.w() = qw;
                
                Matrix4f E = Matrix4f::Identity();
                E.block<3,3>(0,0) = q.toRotationMatrix();
                E.block<3,1>(0,3) = p;
                
                animTrans[frameCount].push_back(E);
            }
            frameCount++;
        }
    }
    in.close();
}

void Cheb::animateCheb()
{
    Vector4f sumPos, sumNor;
    double t = glfwGetTime();
    int k = ((int)(t * ANIM_SPEED) % numFrames);
    vector<float> curPos, curNor;
    
    for (int i = 0; i < numVerts; ++i)
    {
        sumPos = Vector4f(0.0, 0.0, 0.0, 0.0);
        sumNor = Vector4f(0.0, 0.0, 0.0, 0.0);
        //Instead of going through all 17 bones, go through a list of vertices.
        for (int jNdx = 0; jNdx < numBoneInfluences[i]; ++jNdx)
        {
            int j = boneIndices[(i * 16) + jNdx];
            MatrixXf tempPos(4, 1);
            MatrixXf tempNor(4, 1);
            
            tempPos = bindPose[j] * Vector4f(origPos[(i*3)], origPos[(i*3)+1], origPos[(i*3)+2], 1.0);
            tempNor = bindPose[j] * Vector4f(origNor[(i*3)], origNor[(i*3)+1], origNor[(i*3)+2], 0.0);
            
            tempPos = animTrans[k][j] * tempPos;
            tempNor = animTrans[k][j] * tempNor;
            
            sumPos += (skinningWeights[(i * numBones) + j] * tempPos);
            sumNor += (skinningWeights[(i * numBones) + j] * tempNor);
        }

        curPos.push_back(sumPos.x());
        curPos.push_back(sumPos.y());
        curPos.push_back(sumPos.z());
        
        curNor.push_back(sumNor.x());
        curNor.push_back(sumNor.y());
        curNor.push_back(sumNor.z());
    }

    shape->setPosBuf(curPos);
    shape->setNorBuf(curNor);
}

std::vector<float> Cheb::getVertPos()
{
    return shape->getPosBuf();
}

void Cheb::init()
{
    shape->init();
    
    // Send the weights array to the GPU
    glGenBuffers(1, &weightBufID);
    glBindBuffer(GL_ARRAY_BUFFER, weightBufID);
    glBufferData(GL_ARRAY_BUFFER, skinningWeights.size()*sizeof(float), &skinningWeights[0], GL_STATIC_DRAW);
    
    // Send the bone influence array to the GPU
    glGenBuffers(1, &boneBufID);
    glBindBuffer(GL_ARRAY_BUFFER, boneBufID);
    glBufferData(GL_ARRAY_BUFFER, boneIndices.size()*sizeof(int), &boneIndices[0], GL_STATIC_DRAW);
    
    // Send the number of bones influences array to the GPU
    glGenBuffers(1, &numBoneInfID);
    glBindBuffer(GL_ARRAY_BUFFER, numBoneInfID);
    glBufferData(GL_ARRAY_BUFFER, numBoneInfluences.size()*sizeof(int), &numBoneInfluences[0], GL_STATIC_DRAW);
    
    // Unbind the arrays
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    GLSL::checkError(GET_FILE_LINE);
}

void Cheb::drawMesh(shared_ptr<MatrixStack> M, shared_ptr<Program> prog)
{
    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, M->topMatrix().data());
    
    shape->draw(prog);
}

void Cheb::drawMeshGPU(shared_ptr<MatrixStack> M, shared_ptr<Program> prog)
{
    int h_numBoneInfluence = prog->getAttribute("numBoneInfluences");
    
    int h_weightBuf1 = prog->getAttribute("weights0");
    int h_weightBuf2 = prog->getAttribute("weights1");
    int h_weightBuf3 = prog->getAttribute("weights2");
    int h_weightBuf4 = prog->getAttribute("weights3");
    int h_weightBuf5 = prog->getAttribute("weights4");
    
    int h_boneBuf1 = prog->getAttribute("bones0");
    int h_boneBuf2 = prog->getAttribute("bones1");
    int h_boneBuf3 = prog->getAttribute("bones2");
    int h_boneBuf4 = prog->getAttribute("bones3");
    
    double t = glfwGetTime();
    int k = ((int)(t * ANIM_SPEED) % numFrames);
    
    glUniformMatrix4fv(prog->getUniform("MV"), 1, GL_FALSE, M->topMatrix().data());
    glUniformMatrix4fv(prog->getUniform("bindPose"), 18, GL_FALSE, bindPose[0].data());
    glUniformMatrix4fv(prog->getUniform("animTrans"), 18, GL_FALSE, animTrans[k][0].data());
    
    GLSL::enableVertexAttribArray(h_weightBuf1);
    GLSL::enableVertexAttribArray(h_weightBuf2);
    GLSL::enableVertexAttribArray(h_weightBuf3);
    GLSL::enableVertexAttribArray(h_weightBuf4);
    GLSL::enableVertexAttribArray(h_weightBuf5);
    glBindBuffer(GL_ARRAY_BUFFER, weightBufID);
    unsigned stride = numBones*sizeof(float);
    glVertexAttribPointer(h_weightBuf1, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 0*sizeof(float)));
    glVertexAttribPointer(h_weightBuf2, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 4*sizeof(float)));
    glVertexAttribPointer(h_weightBuf3, 4, GL_FLOAT, GL_FALSE, stride, (const void *)( 8*sizeof(float)));
    glVertexAttribPointer(h_weightBuf4, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(12*sizeof(float)));
    glVertexAttribPointer(h_weightBuf5, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(16*sizeof(float)));
    
    GLSL::enableVertexAttribArray(h_boneBuf1);
    GLSL::enableVertexAttribArray(h_boneBuf2);
    GLSL::enableVertexAttribArray(h_boneBuf3);
    GLSL::enableVertexAttribArray(h_boneBuf4);
    glBindBuffer(GL_ARRAY_BUFFER, boneBufID);
    unsigned stride1 = 16*sizeof(int);
    glVertexAttribPointer(h_boneBuf1, 4, GL_INT, GL_FALSE, stride1, (const void *)( 0*sizeof(int)));
    glVertexAttribPointer(h_boneBuf2, 4, GL_INT, GL_FALSE, stride1, (const void *)( 4*sizeof(int)));
    glVertexAttribPointer(h_boneBuf3, 4, GL_INT, GL_FALSE, stride1, (const void *)( 8*sizeof(int)));
    glVertexAttribPointer(h_boneBuf4, 4, GL_INT, GL_FALSE, stride1, (const void *)(12*sizeof(int)));
    
    GLSL::enableVertexAttribArray(h_numBoneInfluence);
    glBindBuffer(GL_ARRAY_BUFFER, numBoneInfID);
    glVertexAttribPointer(h_numBoneInfluence, 1, GL_INT, GL_FALSE, 0, (const void *)0);
    
    shape->draw(prog);
    
    GLSL::disableVertexAttribArray(h_weightBuf1);
    GLSL::disableVertexAttribArray(h_weightBuf2);
    GLSL::disableVertexAttribArray(h_weightBuf3);
    GLSL::disableVertexAttribArray(h_weightBuf4);
    GLSL::disableVertexAttribArray(h_weightBuf5);
    GLSL::disableVertexAttribArray(h_boneBuf1);
    GLSL::disableVertexAttribArray(h_boneBuf2);
    GLSL::disableVertexAttribArray(h_boneBuf3);
    GLSL::disableVertexAttribArray(h_boneBuf4);
    GLSL::disableVertexAttribArray(h_numBoneInfluence);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    GLSL::checkError(GET_FILE_LINE);
}

void Cheb::draw(shared_ptr<MatrixStack> M, shared_ptr<Program> prog)
{
//    Matrix4f R = Matrix4f::Identity();
    animateCheb();
    
    M->pushMatrix();
    M->translate(currentTrans);
//    R.block<3,3>(0,0) = Eiq0.toRotationMatrix();
//    M->multMatrix(R);
    drawMesh(M, prog);
    M->popMatrix();
}

void Cheb::drawGPU(shared_ptr<MatrixStack> M, shared_ptr<Program> prog)
{
    M->pushMatrix();
    M->translate(currentTrans);
    //    R.block<3,3>(0,0) = Eiq0.toRotationMatrix();
    //    M->multMatrix(R);
    drawMeshGPU(M, prog);
    M->popMatrix();
}