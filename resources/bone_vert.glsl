#version 120
attribute vec4 vertPos; // in object space
attribute vec3 vertNor; // in object space
attribute vec2 vertTex;

attribute vec4 boneIds;
attribute vec4 boneIds2;
attribute vec4 boneWeights;
attribute vec4 boneWeights2;

uniform mat4 P;
uniform mat4 MV;

// bone adjustments
uniform mat4 bones[100];

varying vec3 color; // Pass to fragment shader
varying vec3 fragPos;
varying vec3 fragNor;
varying vec2 vTexCoord;

void main()
{
    //vec4 norm = MV * vec4(vertNor, 0.0);
    //normal = norm.xyz;
    
	vTexCoord = vertTex;
    mat4 boneTrans = bones[int(boneIds[0])] * boneWeights[0];
    boneTrans += bones[int(boneIds[1])] * boneWeights[1];
    boneTrans += bones[int(boneIds[2])] * boneWeights[2];
    boneTrans += bones[int(boneIds[3])] * boneWeights[3];
    boneTrans += bones[int(boneIds2[0])] * boneWeights2[0];
    boneTrans += bones[int(boneIds2[1])] * boneWeights2[1];
    boneTrans += bones[int(boneIds2[2])] * boneWeights2[2];
    boneTrans += bones[int(boneIds2[3])] * boneWeights2[3];

    vec4 posCam = MV * boneTrans * vertPos;
    fragPos = posCam.xyz;
    fragNor = (MV * vec4(vertNor, 0.0)).xyz;
    gl_Position = P * MV * boneTrans * vertPos;
    
    color = vec3(0.0, 0.0, 1.0);
}

