#version 120
//Texture
//uniform sampler2D uTexUnit;
//uniform sampler2D uTexNorUnit;

varying vec3 color; // passed from the vertex shader
varying vec3 fragPos;
varying vec3 fragNor;
varying vec2 vTexCoord;

void main()
{
    vec3 lightPos = vec3(2.0, 3.0, 2.0); // in camera space
    vec3 lightPos2 = vec3(-2.0, 3.0, -2.0);
    vec3 n = normalize(fragNor);
    vec3 l = normalize(lightPos - fragPos);
    vec3 l2 = normalize(lightPos2 - fragPos);
    vec3 v = -normalize(fragPos);
    vec3 h = normalize(l + v);
    vec3 h2 = normalize(l2 + v);
    vec3 colorD = (max(dot(l, n), 0.0) + max(dot(l2, n), 0.0)) * color;
    vec3 colorS = (pow(max(dot(h, n), 0.0), 200) + pow(max(dot(h2, n), 0.0), 200)) * vec3(1.0, 1.0, 1.0f);
    vec3 finalcolor = colorD + colorS;
    gl_FragColor = vec4(color + finalcolor, 1.0);

    //vec4 texColor1 = texture2D(uTexUnit, vTexCoord);
    //color.r = min(color.r, 0.5);
    //color.g = min(color.g, 0.5);
    //color.b = min(color.b, 0.5);
    //gl_FragColor = texColor1 * 2.0 * vec4(color.r, color.g, color.b, 1.0);
}

