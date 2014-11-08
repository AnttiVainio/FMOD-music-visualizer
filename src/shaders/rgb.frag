#version 140 //GLSL version 1.4 (OpenGL 3.1)

in vec2 f_tex_coord;

out vec4 color;

uniform sampler2D texture1; //Initialized to 0 by default (no need to set using glUniform)

void main() {
	//move red and blue channels horizontally
	color = vec4(
		texture2D(texture1, f_tex_coord + vec2(-0.01, 0.0)).r,
		texture2D(texture1, f_tex_coord + vec2( 0.0,  0.0)).g,
		texture2D(texture1, f_tex_coord + vec2( 0.01, 0.0)).b,
		1.0);
}
