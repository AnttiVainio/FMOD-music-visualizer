#version 140 //GLSL version 1.4 (OpenGL 3.1)

in vec2 f_color;

out vec4 color;

void main() {
	color = vec4(vec3(f_color.x), f_color.y);
}
