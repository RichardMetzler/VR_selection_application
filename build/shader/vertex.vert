#version 410 core

layout(location = 0) in vec3 viVertex;
layout(location = 1) in vec4 viColour;
layout(location = 2) in vec3 viNormal;
uniform vec4 viLightPos0;
uniform mat4 pvm;
uniform mat3 normalMatrix;
uniform mat4 vmMatrix;


smooth out vec3 normal;
smooth out vec4 colour;
out vec3[1] lightPosArr;
smooth out vec3 vertexEyeSpace;

void main() {
    colour = viColour;
   
    lightPosArr[0] = (vmMatrix * viLightPos0).xyz;
    normal = normalize(normalMatrix * viNormal);
    
    gl_Position = pvm * vec4(viVertex,1.);
    vec4 vertEyeSpace4 = vmMatrix * vec4(viVertex,1.);
    vertexEyeSpace = vec3(vertEyeSpace4);// / vertEyeSpace4.w;
        
}
