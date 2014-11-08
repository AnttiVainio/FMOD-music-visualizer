/** graphics.hpp **/

#ifndef GRAPHICS_HPP
#define GRAPHICS_HPP

#include "shader.hpp"

//We will only draw rectangles and they consist of 4 vertices
#define VERTEX_ARRAY_SIZE 4

/*
	This class handles all the actual drawing using vertex array objects for drawing
	Also uses framebuffer objects for "motion blur"
*/
class graphics_c {
	private:
		graphics_c(const graphics_c &obj); //Copy constructor
		graphics_c &operator=(const graphics_c &obj); //Assign operator

		//Shaders
		shader texture_shader;
		shader color_shader;

		//Buffers
		GLuint vao, vertex_buffer, color_buffer, tex_coord_buffer;
		GLuint framebuffer_tex, framebuffer;

	public:
		graphics_c();
		~graphics_c();
		void draw_arrays(const float *vertices, const float* colors) const;
		void draw_framebuffer() const;
};

#endif
