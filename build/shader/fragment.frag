# version 410 core

layout( location = 0 ) out vec4 FragColor;

smooth in vec3 normal;
uniform vec3 userPos = vec3(0.,0.,0.);
smooth in vec4 colour;
in vec3[1] lightPosArr;
smooth in vec3 vertexEyeSpace;
 
uniform vec4 matDiffuse = vec4(0.8f,0.8f,0.8f,1.0);
uniform vec4 matAmbient = vec4(0.4f,0.4f,0.4f,1.0);
uniform vec4 matSpecular = vec4(0.0f, 0.0f, 0.0f, 1.0f);
uniform float matShininess = 25.f;
 
void main() {
    vec4 combinedColour = ( colour) ;
    vec3 diffuseColour = (matDiffuse * combinedColour).xyz;
    vec3 ambientColour = (matAmbient * combinedColour).xyz;
    vec3 specularColour = (matSpecular * combinedColour).xyz;

    vec3 normal = (gl_FrontFacing) ? normal : -normal;
    vec3 colour = vec3(0);
    vec3  viewDirection  = normalize( userPos - vertexEyeSpace );
    for(int i = 0; i < 1; ++i){
        vec3 lightDir = normalize(lightPosArr[i] - vertexEyeSpace);
        float lambertian = max(abs(dot(lightDir,normal)), 0.0);
        if(lambertian > 0.){
            colour = lambertian * diffuseColour;
            if (matShininess > 0.){
                vec3  reflection = normalize(reflect(-lightDir, normal));                                                  
                float fRDotV = max( 0.0,  dot( reflection, viewDirection ) );
                float specular = (pow(fRDotV, matShininess));
                colour = colour + specular * matSpecular.xyz;
            }
        }
    }
    FragColor.xyz =  ambientColour + colour;
    FragColor.a = 1.;
}