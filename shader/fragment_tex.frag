#version 330 core
//set floats to high precision
precision highp float; 

in  vec3 ex_Color;
in  vec2 ex_tCoord;

out vec4 gl_FragColor;

uniform sampler2D texture0;

void main(void) {
    // Pass through our original color with full opacity.
    //gl_FragColor = (vec4(ex_Color,1.0) +texture(texture0,ex_tCoord))/2.;
	gl_FragColor = texture(texture0,ex_tCoord);
	//gl_FragColor = vec4(ex_tCoord.x,ex_tCoord.y,0.,1.);
}