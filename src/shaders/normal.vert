#version 140 //GLSL version 1.4 (OpenGL 3.1)

//GLSL version 3.3 (OpenGL 3.3) feature
//Required for layout(location = 0)
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec2 position;
layout(location = 2) in vec2 tex_coord;

out vec2 f_tex_coord;

void main() {
	f_tex_coord = tex_coord;
	gl_Position = vec4(position, 0.0, 1.0);
}
