/** shader.hpp **/

#ifndef SHADER_HPP
#define SHADER_HPP

#include <vector>
#include <GL/glew.h>

/*
	This class is a nice wrapper for OpenGLs shaders
	Supports multiple vertex and fragment shaders for a single shader program
*/
class shader {
	private:
		shader(const shader &obj); //Copy constructor
		shader &operator=(const shader &obj); //Assign operator

		//Lists of vertex and fragment shaders
		const GLuint program;
		std::vector<GLuint> vertex_shaders;
		std::vector<GLuint> fragment_shaders;

	public:
		shader(const char *vprog, const char *fprog);
		~shader();
		void add_vertex_shader(const char *vprog);
		void add_fragment_shader(const char *fprog);
		void use() const;
		void operator()() const;
};

#endif
