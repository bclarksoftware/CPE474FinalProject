#version 120

attribute vec4 vertPos;
attribute vec3 vertNor;
attribute vec2 vertTex;

uniform mat4 P;
uniform mat4 MV;
varying vec3 fragPos;
varying vec3 fragNor;
varying vec3 color;

void main()
{
    vec4 posCam = MV * vertPos;
    gl_Position = P * posCam;
    fragPos = posCam.xyz;
    fragNor = (MV * vec4(vertNor, 0.0)).xyz;
    color = vec3(0.0, 0.0, 1.0);
}