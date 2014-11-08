#version 140 //GLSL version 1.4 (OpenGL 3.1)

//GLSL version 3.3 (OpenGL 3.3) feature
//Required for layout(location = 0)
#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 color;

out vec2 f_color;

void main() {
	f_color = color;
	gl_Position = vec4(position, 0.0, 1.0);
}
