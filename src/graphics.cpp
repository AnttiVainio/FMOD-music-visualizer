/** graphics.cpp **/

#include <GL/glew.h>
#include <string>
#include "graphics.hpp"
#include "main.hpp"

/*
	The used texture shader can be specified here
	There are a few shaders already available:
		"normal" - no special effect
		"invert" - inverts the colors
		"rgb" - separates red green and blue channels by moving them horizontally
	Custom shaders can be created by for example placing example.frag in the shaders-directory
	  this shader could then be used by setting this define to "example"
*/
#define SHADER_NAME "normal"

graphics_c::graphics_c():
	texture_shader("src/shaders/normal.vert", (std::string("src/shaders/") + SHADER_NAME + ".frag").c_str()),
	color_shader("src/shaders/color.vert", "src/shaders/color.frag") {

	//Basic texture coordinates
	const float full_screen_tex_coords[VERTEX_ARRAY_SIZE * 2] = {
		0, 0,
		1, 0,
		0, 1,
		1, 1};

	//Init vertex array object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Init some OpenGL buffers
	glGenBuffers(1, &vertex_buffer);
	glGenBuffers(1, &color_buffer);
	glGenBuffers(1, &tex_coord_buffer);

	//Bind and init buffer for vertex data
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0); //The first argument 0 is defined as "layout(location = 0)" in shader code
    glBufferData(GL_ARRAY_BUFFER, sizeof(float[VERTEX_ARRAY_SIZE * 2]), NULL, GL_STREAM_DRAW);

	//Bind and init buffer for color data
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); //The first argument 1 is defined as "layout(location = 1)" in shader code
    glBufferData(GL_ARRAY_BUFFER, sizeof(float[VERTEX_ARRAY_SIZE * 2]), NULL, GL_STREAM_DRAW);

	//Bind and init buffer for texture coordinate data
	//Texture coordinates remain same all the time so we can already set the data
    glBindBuffer(GL_ARRAY_BUFFER, tex_coord_buffer);
	glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0); //The first argument 2 is defined as "layout(location = 2)" in shader code
    glBufferData(GL_ARRAY_BUFFER, sizeof(float[VERTEX_ARRAY_SIZE * 2]), full_screen_tex_coords, GL_STATIC_DRAW);

	//Init texture for the framebuffer
    glGenTextures(1, &framebuffer_tex);
	glBindTexture(GL_TEXTURE_2D, framebuffer_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL);

	//Init framebuffer object
	//By default everything is now drawn into this buffer
	glGenFramebuffersEXT(1, &framebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, framebuffer_tex, 0);
}

graphics_c::~graphics_c() {
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &color_buffer);
	glDeleteBuffers(1, &tex_coord_buffer);

	glDeleteTextures(1, &framebuffer_tex);
	glDeleteFramebuffersEXT(1, &framebuffer);
}

//Draw using the given vertices and their colors
//Expects 4 vertices
void graphics_c::draw_arrays(const float *vertices, const float* colors) const {
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float[VERTEX_ARRAY_SIZE * 2]), vertices);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float[VERTEX_ARRAY_SIZE * 2]), colors);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, VERTEX_ARRAY_SIZE);
}

//By default everything is first drawn into the framebuffer object here
//This function draws the content of the framebuffer object to the main framebuffer that is actually visible on screen
//This way some "motion blur" can be produced
//This function should be called before swapping the screen to actually see the updates
void graphics_c::draw_framebuffer() const {
	//Enable main framebuffer and use texturing shader
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	texture_shader();

	//Draw the content of our framebuffer object
	//The framebuffer texture was already bound in the initialization
	const float full_screen_vertices[VERTEX_ARRAY_SIZE * 2] = {
		-1, -1,
		 1, -1,
		-1,  1,
		 1,  1};
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float[VERTEX_ARRAY_SIZE * 2]), full_screen_vertices);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, VERTEX_ARRAY_SIZE);

	//Again draw to the framebuffer object without texturing
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
	color_shader();
}
