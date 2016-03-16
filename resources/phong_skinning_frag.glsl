#version 120

varying vec3 fragPos; // in camera space
varying vec3 fragNor; // in camera space
varying vec3 color;

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
    gl_FragColor = vec4(finalcolor, 1.0);
}