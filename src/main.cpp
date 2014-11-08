/** main.cpp **/

#include <iostream>
#include <cstdlib>
#include <GL/glew.h>
#include <GL/glfw.h>
#include "main.hpp"
#include "visualizer.hpp"

/*

	This program is supposed to visualize the spectrum of music
	  with bars representing volumes of different frequencies
	  and some squares representing volumes of bass and left and right channels
	The program uses OpenGL 3.1 for graphics and FMOD for playing music and analyzing the spectrum
	As FMOD does the actual analysis,
	  this program does no FFT or anything else regarding spectrum analysis
	  instead solely focusing on visualizing the data
	There is still one important thing to notice:
	  FMOD gives the spectrum using linear scale
	  whereas sound spectrum is better visualized using logarithmic scale,
	  so this program still needs to do that conversion

	You can also play other songs than the one song that comes with this program
	This can be done by giving the file name/path as the first command line parameter
	On Windows this can also be done by dragging a music file, that is in this same folder, on the executable file of this program

*/

inline void print_error(const char *message) {
	std::cerr << "ERROR: " << message << std::endl;
	std::cin.ignore();
	exit(1);
}

int main(int argc, char **argv) {
	//Check program arguments and set played music file accordingly
	if(argc < 2) {
		std::cout << "No music file specified. Playing default song:" << std::endl;
		std::cout << "  Horizon by Geoplex" << std::endl;
		std::cout << "  Get original version from http://www.newgrounds.com/audio/listen/520387" << std::endl;
		std::cout << "You can play other songs by giving their file name/path as the first command line parameter." << std::endl;
		std::cout << std::endl;
	}
	const char *music_file = argc < 2 ? "520387_Horizon_short.mp3" : argv[1];

	//Init GLFW and open window
	if(glfwInit() == GL_FALSE) print_error("Couldn't initialize GLFW!");
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE);
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 0);
	if(glfwOpenWindow(WINDOW_WIDTH, WINDOW_HEIGHT, 8, 8, 8, 8, 0, 0, GLFW_WINDOW) == GL_FALSE) print_error("Couldn't open window!");

	//Set program window in the middle of the screen and some other settings
	GLFWvidmode desktop_resolution;
	glfwGetDesktopMode(&desktop_resolution);
	glfwSetWindowPos((desktop_resolution.Width - WINDOW_WIDTH) / 2, (desktop_resolution.Height - WINDOW_HEIGHT) / 3);
	glfwSetWindowTitle("FMOD music spectrum visualizer");
	glfwSwapInterval(1); //vsync
	glfwEnable(GLFW_MOUSE_CURSOR);

	//Check some values
	std::cout << "Using OpenGL version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "Rendering with: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << (glfwGetWindowParam(GLFW_ACCELERATED) ? "Hardware accelerated window succesfully opened!" : "Couldn't get hardware acceleration!") << std::endl;
	std::cout << "Color bits: " << (glfwGetWindowParam(GLFW_RED_BITS) + glfwGetWindowParam(GLFW_GREEN_BITS) + glfwGetWindowParam(GLFW_BLUE_BITS) + glfwGetWindowParam(GLFW_ALPHA_BITS)) << " / 32" << std::endl;
	std::cout << "Depth bits: " << glfwGetWindowParam(GLFW_DEPTH_BITS) << " / 0" << std::endl;
	std::cout << "Stencil bits: " << glfwGetWindowParam(GLFW_STENCIL_BITS) << " / 0" << std::endl;
	std::cout << "Multisampling: " << glfwGetWindowParam(GLFW_FSAA_SAMPLES) << " / 0" << std::endl;

	//Init GLEW and check OpenGL version support
	if(glewInit() != GLEW_OK) print_error("Couldn't initialize GLEW!");
	if(!glewIsSupported("GL_VERSION_3_1")) std::cerr << "WARNING: OpenGL 3.1 not supported!" << std::endl;

	//OpenGL settings
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	//Wrapped inside this block so that visualizer gets automatically deleted
	{
		visualizer_c visualizer(music_file);
		visualizer.run();
	}

	glfwTerminate();
	return 0;
}
