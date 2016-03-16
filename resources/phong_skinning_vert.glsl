#version 120

attribute vec4 vertPos;
attribute vec3 vertNor;
attribute vec2 vertTex;

attribute vec4 weights0;
attribute vec4 weights1;
attribute vec4 weights2;
attribute vec4 weights3;
attribute vec2 weights4;
attribute vec4 bones0;
attribute vec4 bones1;
attribute vec4 bones2;
attribute vec4 bones3;
attribute float numBoneInfluences;

uniform mat4 bindPose[18];
uniform mat4 animTrans[18];

uniform mat4 P;
uniform mat4 MV;
varying vec3 fragPos;
varying vec3 fragNor;
varying vec3 color;

void main()
{
    vec4 sumPos = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 sumNor = vec4(0.0, 0.0, 0.0, 0.0);

    for (int jNdx = 0; jNdx < int(numBoneInfluences); ++jNdx)
    {
        int j;
        if (int(numBoneInfluences) < 4)
        {
            j = int(bones0[jNdx]);
        }
        else if (int(numBoneInfluences) < 8)
        {
            j = int(bones1[jNdx - 4]);
        }
        else if (int(numBoneInfluences) < 12)
        {
            j = int(bones2[jNdx - 8]);
        }
        else
        {
            j = int(bones3[jNdx - 12]);
        }
        
        vec4 tempPos = bindPose[j] * vertPos;
        vec4 tempNor = bindPose[j] * vec4(vertNor, 0.0);
        
        tempPos = animTrans[j] * tempPos;
        tempNor = animTrans[j] * tempNor;
        
        if (j < 4)
        {
            sumPos = sumPos + weights0[j] * tempPos;
            sumNor = sumNor + weights0[j] * tempNor;
        }
        else if (j < 8)
        {
            sumPos = sumPos + weights1[j - 4] * tempPos;
            sumNor = sumNor + weights1[j - 4] * tempNor;
        }
        else if (j < 12)
        {
            sumPos = sumPos + weights2[j - 8] * tempPos;
            sumNor = sumNor + weights2[j - 8] * tempNor;
        }
        else if (j < 16)
        {
            sumPos = sumPos + weights3[j - 12] * tempPos;
            sumNor = sumNor + weights3[j - 12] * tempNor;
        }
        else
        {
            sumPos = sumPos + weights4[j - 16] * tempPos;
            sumNor = sumNor + weights4[j - 16] * tempNor;
        }
    }
    
    vec4 posCam = MV * sumPos; //vertPos;
    gl_Position = P * posCam;
    fragPos = posCam.xyz;
    fragNor = (MV * vec4(sumNor.xyz, 0.0)).xyz; //vec4(vertNor, 0.0)).xyz;
    color = vec3(0.0, 0.0, 1.0);
}