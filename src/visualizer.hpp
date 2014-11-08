/** visualizer.hpp **/

#ifndef VISUALIZER_HPP
#define VISUALIZER_HPP

#include "graphics.hpp"
#include "sound_system.hpp"

/*
	This class is sort of the main loop of the program
	It mainly figures out what to draw
*/
class visualizer_c {
	private:
		visualizer_c(const visualizer_c &obj); //Copy constructor
		visualizer_c &operator=(const visualizer_c &obj); //Assign operator

		graphics_c graphics;
		sound_system_c sound_system;

	public:
		visualizer_c(const char *song_name);
		void run();
};

#endif
