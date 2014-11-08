/** shader.cpp **/

#include "shader.hpp"
#include <iostream>
#include <fstream>
#include <cstring>

enum {
	PROGRAM, SHADER
};

//Function for printing shader and shader program logs
//Used to reveal possible bugs in shader code
void print_shader_log(const GLuint obj, const unsigned char type) {
	int log_length;
	if(type == SHADER) glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &log_length);
	else glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_length);

	if(log_length > 0) {
		int chars_written;
		char *_log = new char[log_length];

		if(type == SHADER) glGetShaderInfoLog(obj, log_length, &chars_written, _log);
		else glGetProgramInfoLog(obj, log_length, &chars_written, _log);

		if(type == PROGRAM) std::cout << "Shader program info log:\n" << _log << std::endl;
		else std::cout << _log << std::endl;

		delete [] _log;
	}
}

//This function loads a shader from a file and turns it into an OpenGL shader
GLuint load_shader(const char *path, const GLenum type) {
	//Load from file
	std::ifstream file(path, std::ios::in | std::ios::binary | std::ios::ate);
	const unsigned int size = file.tellg();

	if(!file.good() || size <= 2) {
		file.close();
		std::cerr << "Couldn't load a shader from " << path << "!" << std::endl;
		return 0;
	}

	file.seekg(0, std::ios::beg);
	char *source = new char[size];
	file.read(source, size);
	file.close();

	//Create shader
	const GLuint shader = glCreateShader(type);
	const char *_source = source;
	const GLint _size = size;
	glShaderSource(shader, 1, &_source, &_size);
	delete [] source;
	glCompileShader(shader);

	std::cout << "Log for " << path << ":" << std::endl;
	print_shader_log(shader, SHADER);
	return shader;
}

//Initializes the shader from the given vertex and fragment shader file paths
shader::shader(const char *vprog, const char *fprog): program(glCreateProgram()) {
	add_fragment_shader(fprog);
	add_vertex_shader(vprog);
}

shader::~shader() {
	for(std::vector<GLuint>::const_iterator i = vertex_shaders.begin(); i != vertex_shaders.end(); i++) {
		glDetachShader(program, *i);
		glDeleteShader(*i);
	}
	for(std::vector<GLuint>::const_iterator i = fragment_shaders.begin(); i != fragment_shaders.end(); i++) {
		glDetachShader(program, *i);
		glDeleteShader(*i);
	}
	glDeleteProgram(program);
}

//Add more vertex shader files
void shader::add_vertex_shader(const char *vprog) {
	vertex_shaders.push_back(load_shader(vprog, GL_VERTEX_SHADER));
	glAttachShader(program, vertex_shaders.back());

	if(!vertex_shaders.empty() && !fragment_shaders.empty()) {
		glLinkProgram(program);
		glUseProgram(program);
		print_shader_log(program, PROGRAM);
	}
}

//Add more fragment shader files
void shader::add_fragment_shader(const char *fprog) {
	fragment_shaders.push_back(load_shader(fprog, GL_FRAGMENT_SHADER));
	glAttachShader(program, fragment_shaders.back());

	if(!vertex_shaders.empty() && !fragment_shaders.empty()) {
		glLinkProgram(program);
		glUseProgram(program);
		print_shader_log(program, PROGRAM);
	}
}

//Enable the shader
void shader::use() const {
	glUseProgram(program);
}
void shader::operator()() const {
	glUseProgram(program);
}
