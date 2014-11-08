/** sound_system.hpp **/

#ifndef SOUND_SYSTEM_HPP
#define SOUND_SYSTEM_HPP

#define OUTPUTRATE 48000
#define SPECTRUMSIZE 4096 //Defines the accuracy of the analyzed spectrum

/// NOTE: if compiling FMOD gives you an error, try uncommenting the following line
//#define REDEFINE_FMOD_STDCALL

//FMOD include
#ifdef REDEFINE_FMOD_STDCALL
	#define _stdcall __stdcall
#endif
	#include <fmod.h>
#ifdef REDEFINE_FMOD_STDCALL
	#undef _stdcall
#endif

/*
	The class for initializing FMOD and playing and analyzing music
*/
class sound_system_c {
	private:
		sound_system_c(const sound_system_c &obj); //Copy constructor
		sound_system_c &operator=(const sound_system_c &obj); //Assign operator

		FMOD_SYSTEM *fmod_system;
		FMOD_SOUND *music;
		FMOD_CHANNEL *channel;

	public:
		sound_system_c(const char *song_name);
		~sound_system_c();
		void play_music();
		void get_spectrum(float *spectrumL, float *spectrumR) const;
		void update() const;
};

#endif
