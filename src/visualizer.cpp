/** visualizer.cpp **/

#include <iostream>
#include <cstring>
#include <cmath>
#include <GL/glew.h>
#include <GL/glfw.h>
#include "visualizer.hpp"

#define FPS 60.0 //Frames per second

#define MOTION_BLUR_AMOUNT 0.25f //Amount of "motion blur" in range from 0 to 1

//Defines the way the bars are drawn
//in type 1 multiple bars are combined into one or one bar is broken into multiple bars so that all the drawn parts have same width
//in type 2 all the existing bars are drawn with a variable width
#define BAR_TYPE 1

#define SMOOTH_SPEC //Does some smoothing to the spectrum itself
#define SMOOTH_BARS //Does some smoothing to the bars, does basically the same as SMOOTH_SPEC when BAR_TYPE is 2

//Returns values linearly from y1 to y2 when x has values from x1 to x2
inline float mix(const float x1, const float x2, const float y1, const float y2, const float x) {
	return (y1 - y2) / (x1 - x2) * (x - x1) + y1;
}

visualizer_c::visualizer_c(const char *song_name):
	sound_system(song_name) {}

/*
	A lot of the actual initialization is done here before the loop is started to keep things simple
	First figures out how to draw the bars using logarithmic scale as FMOD gives spectrum data in linear scale
	Then in the loop it mainly figures out what to draw
*/
void visualizer_c::run() {
	//Some drawing information
	const float bars_color[VERTEX_ARRAY_SIZE * 2]    = {1.0, 0.1, 1.0, 1.0, 1.0, 0.1, 1.0, 1.0};
	const float bg_colors[VERTEX_ARRAY_SIZE * 2]     = {1.0, 0.3, 1.0, 0.3, 1.0, 0.0, 1.0, 0.0};
	const float square_colors[VERTEX_ARRAY_SIZE * 2] = {0.05, 1.0, 0.05, 1.0, 0.05, 1.0, 0.05, 1.0};
	const float fade_colors[VERTEX_ARRAY_SIZE * 2]   = {0.0, 1.0f - MOTION_BLUR_AMOUNT, 0.0, 1.0f - MOTION_BLUR_AMOUNT, 0.0, 1.0f - MOTION_BLUR_AMOUNT, 0.0, 1.0f - MOTION_BLUR_AMOUNT};
	const float full_screen_vertices[VERTEX_ARRAY_SIZE * 2] = {
		-1, -1,
		 1, -1,
		-1,  1,
		 1,  1};

	//Rather useless defines
	#define SPECTRUMRANGE ((float)OUTPUTRATE / 2.0f) // 24000.0 Hz
	#define BINSIZE (SPECTRUMRANGE / (float)SPECTRUMSIZE) // 5.8594 Hz

	//We do not show the full spectrum, instead just the interesting part
	#define SPECTRUM_START 6 // 41.0156 Hz  (7 * BINSIZE)
	#define SPECTRUM_END 2560 // 15000.0 Hz  (2560 * BINSIZE)
	//Storages for the left and right spectrums
	float spectrumL[SPECTRUMSIZE];
	float spectrumR[SPECTRUMSIZE];

	//Bars 1 have constant width
	//This figures out how to combine or divide the bars on the linear scale
	//  so that they use logarithmic scale instead
	#if BAR_TYPE == 1
		#define BAR_MULT 1.022 //Affects the amount of bars
		int bar_amount = 0; //The amount of bars
		float i = BAR_MULT - 1;
		float start = 0;
		while(start + i <= SPECTRUMSIZE - 1) {
			if(start >= SPECTRUM_START && start + i <= SPECTRUM_END) bar_amount++;
			start+= i;
			i*= BAR_MULT;
		}

		int *bar_start = new int[bar_amount]; //Start of full frequencies
		int *bar_end = new int[bar_amount]; //End of full frequencies
		int *bar_first = new int[bar_amount]; //First non-full frequency
		float *bar_first_mult = new float[bar_amount]; //Mult for first non-full frequency
		int *bar_last = new int[bar_amount]; //Last non-full frequency
		float *bar_last_mult = new float[bar_amount]; //Mult for last non-full frequency

		i = BAR_MULT - 1;
		start = 0;
		while(start < SPECTRUM_START) { //Skip some frequencies
			start+= i;
			i*= BAR_MULT;
		}
		for(int j = 0; j < bar_amount; j++) {
			const float end = start + i;
			bar_start[j] = ceil(start);
			bar_end[j] = floor(end);
			bar_first[j] = floor(start);
			bar_first_mult[j] = bar_start[j] == bar_first[j] ? 0.0 : 1.0 - start + floor(start);
			bar_last[j] = floor(end);
			bar_last_mult[j] = end - floor(end);
			if(bar_first[j] == bar_last[j]) {
				bar_first_mult[j] = end - start;
				bar_last_mult[j] = 0.0;
			}
			start+= i;
			i*= BAR_MULT;
		}
	#endif

	//Bars 2 have variable width
	//This figures widths for the bars so that are on a logarithmic scale
	#if BAR_TYPE == 2
		float bar_size[SPECTRUMSIZE - 1];
		float total_size = 0;
		for(int i = 0; i < SPECTRUMSIZE - 1; i++) {
			bar_size[i] = log(i + 2) - log(i + 1);
			if(i >= SPECTRUM_START && i < SPECTRUM_END) total_size+= bar_size[i];
		}
		for(int i = 0; i < SPECTRUMSIZE - 1; i++) bar_size[i]*= 2.0 / total_size;
	#endif

	//Start playing the song
	sound_system.play_music();

	//Some variables for the motion blur of the squares
	float prev_bass = 0;
	float prev_left = 0;
	float prev_right = 0;
	double time = glfwGetTime();

	//The actual loop starts here
	while(!glfwGetKey(GLFW_KEY_ESC) && glfwGetWindowParam(GLFW_OPENED)) {
		//Get spectrum
		sound_system.get_spectrum(spectrumL, spectrumR);
		float bass_sum = 0;
		float left_sum = 0;
		float right_sum = 0;
		float sound_sum = 0;

		//Smooth the actual spectrum
		#ifdef SMOOTH_SPEC
			float temp_spectrumL[SPECTRUMSIZE];
			float temp_spectrumR[SPECTRUMSIZE];
			memcpy(temp_spectrumL, spectrumL, sizeof(float) * SPECTRUMSIZE);
			memcpy(temp_spectrumR, spectrumR, sizeof(float) * SPECTRUMSIZE);
			for(int i = SPECTRUM_START; i < SPECTRUM_END; i++) {
				spectrumL[i]
					= 0.1 * (temp_spectrumL[i - 2] + temp_spectrumL[i + 2])
					+ 0.2 * (temp_spectrumL[i - 1] + temp_spectrumL[i + 1])
					+ 0.4 * temp_spectrumL[i];
				spectrumR[i]
					= 0.1 * (temp_spectrumR[i - 2] + temp_spectrumR[i + 2])
					+ 0.2 * (temp_spectrumR[i - 1] + temp_spectrumR[i + 1])
					+ 0.4 * temp_spectrumR[i];
			}
		#endif

		//Draw some black color with some alpha over the previous frame
		//This produces some "motion blur"
		graphics.draw_arrays(full_screen_vertices, fade_colors);

		//Calculate the size for the middle bass square
		for(int i = 0; i < SPECTRUMSIZE / 128; i++) {
			bass_sum+= (spectrumL[i] + spectrumR[i]) * ((float)SPECTRUMSIZE / 128.0 - (float)i);
		}
		bass_sum/= 150.0;

		//Calculate the sizes for the left and right squares
		for(int i = 0; i < SPECTRUMSIZE - 1; i++) {
			const float mult = sqrt(i);
			left_sum+= spectrumL[i] * mult;
			right_sum+= spectrumR[i] * mult;
			sound_sum+= spectrumL[i] + spectrumR[i];
		}
		left_sum/= 800.0;
		right_sum/= 800.0;

		/*
			Next calculate and draw the bars
		*/
		#if BAR_TYPE == 1 //Bars with constant width
			float *bar1_heights = new float[bar_amount];
			//Calculate the heights for the bars
			for(int i = 0; i < bar_amount; i++) {
				float sumL = spectrumL[bar_first[i]] * bar_first_mult[i] + spectrumL[bar_last[i]] * bar_last_mult[i];
				float sumR = spectrumR[bar_first[i]] * bar_first_mult[i] + spectrumR[bar_last[i]] * bar_last_mult[i];

				for(int j = bar_start[i]; j < bar_last[i]; j++) {
					sumL+= spectrumL[j - 1];
					sumR+= spectrumR[j - 1];
				}

				bar1_heights[i] = std::max((sumL + sumR) * 5.0 - 0.04, 0.0) + 0.015;
			}
			//Draw the bars
			for(int i = 0; i < bar_amount; i++) {
				const float x = -1.0 + (float)i / (float)bar_amount * 2.0;
				const float x2 = -1.0 + float(i + 1) / (float)bar_amount * 2.0;

				//Smooth the bars here
				#ifdef SMOOTH_BARS
					const float height
						= 0.038 * (bar1_heights[std::max(i - 2, 0)] + bar1_heights[std::min(i + 2, bar_amount - 1)])
						+ 0.154 * (bar1_heights[std::max(i - 1, 0)] + bar1_heights[std::min(i + 1, bar_amount - 1)])
						+ 0.615 * bar1_heights[i];
				#else
					const float height = bar1_heights[i];
				#endif

				//Here we actually draw
				const float vertices[VERTEX_ARRAY_SIZE * 2] = {
					x,  -1.0f,
					x,  -1.0f + height,
					x2, -1.0f,
					x2, -1.0f + height};
				graphics.draw_arrays(vertices, bars_color);
			}
			delete [] bar1_heights;
		#endif

		#if BAR_TYPE == 2 //Bars with variable width
			float pos = -1;
			for(int i = SPECTRUM_START; i < SPECTRUM_END; i++) {
				//Smooth the bars first
				#ifdef SMOOTH_BARS
					const float height = std::max((
						  (0.038 * (spectrumL[i - 2] + spectrumL[i + 2])
							+ 0.154 * (spectrumL[i - 1] + spectrumL[i + 1])
							+ 0.615 * spectrumL[i])
						+ (0.038 * (spectrumR[i - 2] + spectrumR[i + 2])
							+ 0.154 * (spectrumR[i - 1] + spectrumR[i + 1])
							+ 0.615 * spectrumR[i])
						) / bar_size[i] * 0.05 - 0.04, 0.0) + 0.015;
				#else
					const float height = std::max((spectrumL[i] + spectrumR[i]) / bar_size[i] * 0.05 - 0.04, 0.0) + 0.015;
				#endif
				const float x2 = pos + bar_size[i];

				//Here we actually draw
				const float vertices[VERTEX_ARRAY_SIZE * 2] = {
					pos, -1.0f,
					pos, -1.0f + height,
					x2,  -1.0f,
					x2,  -1.0f + height};
				graphics.draw_arrays(vertices, bars_color);

				pos+= bar_size[i];
			}
		#endif

		//Draw the background fade at the top of the window
		const float y = 1.0 - sound_sum / 10.0;
		const float bg_vertices[VERTEX_ARRAY_SIZE * 2] = {
			-1, 1,
			 1, 1,
			-1, y,
			 1, y};
		graphics.draw_arrays(bg_vertices, bg_colors);

		//Draw the squares with some actual motion blur
		glBlendFunc(GL_ONE, GL_ONE); //Additive rendering
		for(int i = 0; i < 20; i++) {
			float size = mix(0, 10, bass_sum, prev_bass, i);
			const float vertices1[VERTEX_ARRAY_SIZE * 2] = {
				-size * 0.56f, 0.1f - size,
				 size * 0.56f, 0.1f - size,
				-size * 0.56f, 0.1f + size,
				 size * 0.56f, 0.1f + size};
			graphics.draw_arrays(vertices1, square_colors);

			size = mix(0, 10, left_sum, prev_left, i);
			const float vertices2[VERTEX_ARRAY_SIZE * 2] = {
				-0.6f - size * 0.56f, 0.5f - size,
				-0.6f + size * 0.56f, 0.5f - size,
				-0.6f - size * 0.56f, 0.5f + size,
				-0.6f + size * 0.56f, 0.5f + size};
			graphics.draw_arrays(vertices2, square_colors);

			size = mix(0, 10, right_sum, prev_right, i);
			const float vertices3[VERTEX_ARRAY_SIZE * 2] = {
				0.6f - size * 0.56f, 0.5f - size,
				0.6f + size * 0.56f, 0.5f - size,
				0.6f - size * 0.56f, 0.5f + size,
				0.6f + size * 0.56f, 0.5f + size};
			graphics.draw_arrays(vertices3, square_colors);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //"normal" rendering

		//Do the "motion blur" and swap the screen
		graphics.draw_framebuffer();
		glfwSwapBuffers();

		//Save values for the next frame
		prev_bass = bass_sum;
		prev_left = left_sum;
		prev_right = right_sum;

		sound_system.update();

		//Handle frames per second
		time+= 1.0 / FPS;
		const double sleep = time - glfwGetTime();
		if(sleep > 0.0) glfwSleep(sleep);
	}

	#if BAR_TYPE == 1
		delete [] bar_start;
		delete [] bar_end;
		delete [] bar_first;
		delete [] bar_first_mult;
		delete [] bar_last;
		delete [] bar_last_mult;
	#endif
}
