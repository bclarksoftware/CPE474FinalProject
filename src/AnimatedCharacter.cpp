//
//  AnimatedCharacter.cpp
//  FinalProject
//
//  Created by Brandon Clark on 3/12/16.
//
//

#include "AnimatedCharacter.hpp"
#include "MatrixStack.h"
#include "Program.h"

int startAnim = 0;
int endAnim = 31;

int framesPerSec = 24;

AnimatedCharacter::AnimatedCharacter(const string source, int inTexNum, float privScale, int daeToBe) {
    int i, j;

    Assimp::Importer importer;
    scene = (aiScene*)importer.ReadFile(source.c_str(), aiProcess_GenNormals);
    
    if(!scene)
    {
        cout << "couldn't read dae\n";
        cout << "reason: " << importer.GetErrorString() << "\n";
        root = NULL;
        return;
    }
    
    texInd = inTexNum;
    
    // TODO remove this magic
    meshInd = 0;
    lastAnim = -1;
    animChoice = -1;
    
    //cout << "root " << scene->mRootNode << "\n";
    root = scene->mRootNode;
    meshes = scene->mMeshes;
    
    /*cout << "scene " << scene << "\n";
     cout << "meshes " << meshes << "\n";
     cout << "num meshes " << scene->mNumMeshes << "\n";
     cout << "facees in mesh 1 " << meshes[meshInd]->mNumFaces << "\n";
     cout << "number of bones " << meshes[meshInd]->mNumBones << "\n";
     cout << "number of animations " << scene->mNumAnimations << "\n";
     cout << "duration of 1 " << scene->mAnimations[0]->mDuration << "\n";
     cout << "tps " << scene->mAnimations[0]->mTicksPerSecond << "\n";
     cout << "number of frames in 1 " << scene->mAnimations[0]->mChannels[0]->mNumRotationKeys << "\n";
     */
    
    //cout << "has tex? " << meshes[meshInd]->HasTextureCoords(0) << "\n";
    //cout << "num comps: " << meshes[meshInd]->mNumUVComponents[0] << "\n";
    
    numInd = meshes[meshInd]->mNumVertices;
    // recursivePrint(scene->mRootNode, 0, meshes);
    
    // positions
    positions = (float*) malloc(numInd * 3 * sizeof(float));
    memcpy(positions, meshes[meshInd]->mVertices, numInd * 3 * sizeof(float));
    
    glGenBuffers(1, &posBuf);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    glBufferData(GL_ARRAY_BUFFER, numInd * 3 * sizeof(float), positions, GL_STATIC_DRAW);
    
    
    // normals
    normals = (float*) malloc(meshes[meshInd]->mNumVertices * 3 * sizeof(float));
    memcpy(normals, meshes[meshInd]->mNormals, numInd * 3 * sizeof(float));
    
    glGenBuffers(1, &norBuf);
    glBindBuffer(GL_ARRAY_BUFFER, norBuf);
    glBufferData(GL_ARRAY_BUFFER, numInd * 3 * sizeof(float), normals, GL_STATIC_DRAW);
    
    
    // indices (face)
    indices = (unsigned int*) malloc(meshes[meshInd]->mNumFaces * 3 * sizeof(unsigned int));
    for (i = 0; i < meshes[meshInd]->mNumFaces; i++) {
        const aiFace* face = &(meshes[meshInd]->mFaces[i]);
        memcpy(&indices[i * 3], face->mIndices, 3 * sizeof(unsigned int));
    }
    
    glGenBuffers(1, &indBuf);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBuf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, meshes[meshInd]->mNumFaces * 3 * sizeof(int), indices, GL_STATIC_DRAW);
    
    
    // texture
    // 2 might be wrong
    texture = (float*) calloc(numInd, 2 * sizeof(float));
    for(i = 0; i < meshes[meshInd]->mNumVertices; i++) {
        texture[2*i] = meshes[meshInd]->mTextureCoords[0][i].x;
        texture[2*i + 1] = meshes[meshInd]->mTextureCoords[0][i].y;
    }
    
    glGenBuffers(1, &texBuf);
    glBindBuffer(GL_ARRAY_BUFFER, texBuf);
    glBufferData(GL_ARRAY_BUFFER, numInd * 2 * sizeof(float), texture, GL_STATIC_DRAW);
    
    
    // numBones, boneId, boneWeight
    // boneCount
    numBones = (unsigned int*) calloc(sizeof(unsigned int), numInd);
    boneId = (unsigned int*) calloc(sizeof(unsigned int) * 4, numInd);
    boneId2 = (unsigned int*) calloc(sizeof(unsigned int) * 4, numInd);
    boneWeight = (float*) calloc(sizeof(float) * 4, numInd);
    boneWeight2 = (float*) calloc(sizeof(float) * 4, numInd);
    aiVertexWeight boneVertex;
    
    // for every bone
    for(i = 0; i < meshes[meshInd]->mNumBones; i++) {
        // mark which vertices this bone affects
        for(j = 0; j < meshes[meshInd]->mBones[i]->mNumWeights; j++) {
            boneVertex = meshes[meshInd]->mBones[i]->mWeights[j];
            
            if (numBones[boneVertex.mVertexId] < 4) {
                boneId[(boneVertex.mVertexId * 4) + numBones[boneVertex.mVertexId]] = i;
                boneWeight[(boneVertex.mVertexId * 4) + numBones[boneVertex.mVertexId]] = boneVertex.mWeight;
            } else {
                boneId2[(boneVertex.mVertexId * 4) + numBones[boneVertex.mVertexId] - 4] = i;
                boneWeight2[(boneVertex.mVertexId * 4) + numBones[boneVertex.mVertexId] - 4] = boneVertex.mWeight;
            }
            
            numBones[boneVertex.mVertexId]++;
        }
    }
    
    // fill the bone Id buffer
    glGenBuffers(1, &boneIdBuf);
    glBindBuffer(GL_ARRAY_BUFFER, boneIdBuf);
    glBufferData(GL_ARRAY_BUFFER, numInd * 4 * sizeof(unsigned int), boneId, GL_STATIC_DRAW);
    
    // fill the bone Weights buffer
    glGenBuffers(1, &boneWeightBuf);
    glBindBuffer(GL_ARRAY_BUFFER, boneWeightBuf);
    glBufferData(GL_ARRAY_BUFFER, numInd * 4 * sizeof(float), boneWeight, GL_STATIC_DRAW);
    
    // fill the bone Id buffer
    glGenBuffers(1, &boneIdBuf2);
    glBindBuffer(GL_ARRAY_BUFFER, boneIdBuf2);
    glBufferData(GL_ARRAY_BUFFER, numInd * 4 * sizeof(unsigned int), boneId2, GL_STATIC_DRAW);
    
    // fill the bone Weights buffer
    glGenBuffers(1, &boneWeightBuf2);
    glBindBuffer(GL_ARRAY_BUFFER, boneWeightBuf2);
    glBufferData(GL_ARRAY_BUFFER, numInd * 4 * sizeof(float), boneWeight2, GL_STATIC_DRAW);
    
    // make the model buffer
    // will need to update that buffers data pretty frequently...
    boneCount = meshes[meshInd]->mNumBones;
    floatModel = (float*) calloc(sizeof(float) * 16, boneCount);
    cout << "floatModel at " << floatModel << "\n";
    cout << "boneCount " << boneCount << "\n";
    
    cout << "finished loading " << source << "\n\n";
}

AnimatedCharacter::~AnimatedCharacter()
{
    
}

void AnimatedCharacter::recursivePrint(const aiNode* node, int level, aiMesh** meshes) {
    int i, j;
    
    for (i = 0; i < level; i++) {
        printf("  -");
    }
    
    //cout << "mesh " << node->mMeshes[0];
    cout << "name " << node->mName.data;
    //cout << ", face count " << meshes[node->mMeshes[0]]->mNumFaces;
    //cout << ", bones? " << meshes[node->mMeshes[0]]->HasBones();
    //cout << ", normals? " << meshes[node->mMeshes[0]]->HasNormals();
    //cout << ", positions? " << meshes[node->mMeshes[0]]->HasPositions();
    cout << "\n";
    
    for (i = 0; i < node->mNumChildren; i++) {
        recursivePrint(node->mChildren[i], level + 1, meshes);
    }
}

void AnimatedCharacter::updateBones(float time) {
    recursiveUpdate(scene->mRootNode, time);
}

void AnimatedCharacter::recursiveUpdate(const aiNode* toUpdate, float time) {
    aiMatrix4x4t<float> parent, us; // default to identity
    int i, j;
    
    if (toUpdate != nullptr && scene != nullptr && scene->mNumAnimations > 0)
    {
        int updateId = findBone(toUpdate);
        
        const aiAnimation* anim = scene->mAnimations[0];
        const aiNodeAnim* nodeAnim = findNodeAnim(anim, toUpdate);
        float timeOffset = 0;
        
        if(isAnimating()) {
            timeOffset = ((float) startAnim) / ((float) framesPerSec);
        } else if (lastAnim != -1) {
            // based on last animations end frame
            timeOffset = ((float) endAnim) / ((float) framesPerSec);
        }
        
        us = toUpdate->mTransformation;
        if(nodeAnim != NULL) {
            // interpolate from where we are in the animation
            aiMatrix4x4t<float> sMat, pMat, rMat;
            aiVector3D scale = intScale(time - animStart + timeOffset, nodeAnim);
            aiVector3D pos = intTrans(time - animStart + timeOffset, nodeAnim);
            aiQuaternion rot = intRot(time - animStart + timeOffset, nodeAnim);
            
            sMat = aiMatrix4x4t<float>::Scaling(scale, sMat);
            pMat = aiMatrix4x4t<float>::Translation(pos, pMat);
            rMat = aiMatrix4x4t<float>(rot.GetMatrix());
            
            us = pMat * rMat * sMat;
        }
        
        // TODO should store this data, wasted lookups
        const aiNode* temp = toUpdate->mParent;
        while(temp != NULL) {
            const aiNodeAnim* pAnim = findNodeAnim(anim, temp);
            if(pAnim != NULL) {
                // use int parent
                aiMatrix4x4t<float> sMatP, pMatP, rMatP;
                aiVector3D scaleP = intScale(time - animStart + timeOffset, pAnim);
                aiVector3D posP = intTrans(time - animStart + timeOffset, pAnim);
                aiQuaternion rotP = intRot(time - animStart + timeOffset, pAnim);
                
                sMatP = aiMatrix4x4t<float>::Scaling(scaleP, sMatP);
                pMatP = aiMatrix4x4t<float>::Translation(posP, pMatP);
                rMatP = aiMatrix4x4t<float>(rotP.GetMatrix());
                
                parent = pMatP * rMatP * sMatP * parent;
            } else {
                parent = temp->mTransformation * parent;
            }
            temp = temp->mParent;
        }
        
        us = parent * us;
        
        
        if(updateId != -1) {
            us = us * meshes[meshInd]->mBones[updateId]->mOffsetMatrix;
            us = us.Transpose();
            
            for(i = 0; i < 4; i++) {
                for(j = 0; j < 4; j++) {
                    floatModel[(updateId * 16) + (i * 4) + j] = us[i][j];
                }
            }
        }
        
        for(i = 0; i < toUpdate->mNumChildren; i++) {
            recursiveUpdate(toUpdate->mChildren[i], time);
        }
    }
}

int AnimatedCharacter::findBone(const aiNode* toFind) {
    int i;
    
    if (toFind != nullptr)
    {
        if (meshes != nullptr && meshes[meshInd] != nullptr)
        {
            for(i = 0; i < meshes[meshInd]->mNumBones; i++) {
                if(toFind->mName == meshes[meshInd]->mBones[i]->mName) {
                    return i;
                }
            }
        }
    }
    
    return -1;
}

const aiNodeAnim* AnimatedCharacter::findNodeAnim(const aiAnimation* anim, const aiNode* toFind) {
    int i;
    
    if (anim != nullptr && toFind != nullptr)
    {
        for(i = 0; i < anim->mNumChannels; i++) {
            if(anim->mChannels[i]->mNodeName == toFind->mName) {
                return anim->mChannels[i];
            }
        }
    }
    
    return NULL;
}

aiQuaternion AnimatedCharacter::intRot(float time, const aiNodeAnim* nodeAnim) {
    int i = 0; // int between i and i+1
    int key;
    
    for(i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
        if(time < (float) nodeAnim->mRotationKeys[i + 1].mTime) {
            key = i;
            break;
        }
    }
    
    const aiQuaternion startRot = nodeAnim->mRotationKeys[key].mValue;
    const aiQuaternion endRot = nodeAnim->mRotationKeys[key + 1].mValue;
    float dt = nodeAnim->mRotationKeys[key + 1].mTime -
    nodeAnim->mRotationKeys[key].mTime;
    float factor = (time - nodeAnim->mRotationKeys[key].mTime) / dt;
    
    aiQuaternion ret;
    aiQuaternion::Interpolate(ret, startRot, endRot, factor);
    
    return ret;
}

// note: our animations don't change scale yet
aiVector3D AnimatedCharacter::intScale(float time, const aiNodeAnim* nodeAnim) {
    // TODO, add this if animations want it
    return aiVector3D(1.0, 1.0, 1.0);
}

aiVector3D AnimatedCharacter::intTrans(float time, const aiNodeAnim* nodeAnim) {
    int i = 0; // int between i and i+1
    int key;
    
    for(i = 0; i < nodeAnim->mNumPositionKeys - 1; i++) {
        if(time < (float) nodeAnim->mPositionKeys[i + 1].mTime) {
            key = i;
            break;
        }
    }
    
    const aiVector3D startPos = nodeAnim->mPositionKeys[key].mValue;
    const aiVector3D endPos = nodeAnim->mPositionKeys[key + 1].mValue;
    float dt = nodeAnim->mPositionKeys[key + 1].mTime -
    nodeAnim->mPositionKeys[key].mTime;
    float factor = (time - nodeAnim->mPositionKeys[key].mTime) / dt;
    
    aiVector3D ret;
    ret += startPos * (1.0f - factor);
    ret += endPos * (factor);
    // aiVector3D::Interpolate(ret, startPos, endPos, factor);
    
    return ret;
}


void AnimatedCharacter::startAnimation(string animation) {
    animStart = lastTime;
    
    animChoice = 0;

    int numFrames = endAnim - startAnim;
        
    int skippedFrames = 0;
    if(lastAnim != animChoice) {
        skippedFrames = rand() % (numFrames);
    }
    
    // frames / fps
    animStart = animStart - (((float) skippedFrames) / ((float) framesPerSec));
    endTime = (((float) numFrames) / ((float) framesPerSec)) + animStart;

    lastAnim = animChoice;
}

bool AnimatedCharacter::isAnimating()
{
    return animChoice != -1;
}

void AnimatedCharacter::drawChar(shared_ptr<MatrixStack> MV, const shared_ptr<Program> p, float time)
{
    if(root == NULL) {
        return;
    }
    
    // prepare to start an animation
    lastTime = time;
    if (time > endTime) {
        animChoice = -1;
    }
    if (!isAnimating()) {
        animStart = lastTime;
    }
    
    // indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indBuf);
    
    // position
    int h_vertPos = p->getAttribute("vertPos");
    GLSL::enableVertexAttribArray(h_vertPos);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf);
    glVertexAttribPointer(h_vertPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    // normals
    int h_vertNor = p->getAttribute("vertNor");
    GLSL::enableVertexAttribArray(h_vertNor);
    glBindBuffer(GL_ARRAY_BUFFER, norBuf);
    glVertexAttribPointer(h_vertNor, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texInd);
    
    int h_aTexCoord = p->getAttribute("vertTex");
    GLSL::enableVertexAttribArray(h_aTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, texBuf);
    glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
    
    // model transform
    MV->rotate(rotate, Eigen::Vector3f(0.0, 1.0, 0.0));
    MV->scale(scale);
    glUniformMatrix4fv(p->getUniform("MV"), 1, GL_FALSE, MV->topMatrix().data());
    
    // bone id
    int h_boneIds = p->getAttribute("boneIds");
    GLSL::enableVertexAttribArray(h_boneIds);
    glBindBuffer(GL_ARRAY_BUFFER, boneIdBuf);
    glVertexAttribPointer(h_boneIds, 4, GL_INT, GL_FALSE, 0, 0);
    
    int h_boneIds2 = p->getAttribute("boneIds2");
    GLSL::enableVertexAttribArray(h_boneIds2);
    glBindBuffer(GL_ARRAY_BUFFER, boneIdBuf2);
    glVertexAttribPointer(h_boneIds2, 4, GL_INT, GL_FALSE, 0, 0);
    
    // bone weight
    int h_boneWeights = p->getAttribute("boneWeights");
    GLSL::enableVertexAttribArray(h_boneWeights);
    glBindBuffer(GL_ARRAY_BUFFER, boneWeightBuf);
    glVertexAttribPointer(h_boneWeights, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    int h_boneWeights2 = p->getAttribute("boneWeights2");
    GLSL::enableVertexAttribArray(h_boneWeights2);
    glBindBuffer(GL_ARRAY_BUFFER, boneWeightBuf2);
    glVertexAttribPointer(h_boneWeights2, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    // bone models
    updateBones(time);
    
    glUniformMatrix4fv(p->getUniform("bones"), boneCount, (GLboolean) false, floatModel);
    
    // actual draw call
    glDrawElements(GL_TRIANGLES, numInd, GL_UNSIGNED_INT, 0);
    
    GLSL::disableVertexAttribArray(h_vertPos); // position
    GLSL::disableVertexAttribArray(h_vertNor); // normals
    GLSL::disableVertexAttribArray(h_aTexCoord); // texture
    GLSL::disableVertexAttribArray(h_boneIds); // bone ids
    GLSL::disableVertexAttribArray(h_boneIds2);
    GLSL::disableVertexAttribArray(h_boneWeights); // bone weights
    GLSL::disableVertexAttribArray(h_boneWeights2);
    
    glDisable(GL_TEXTURE_2D);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
