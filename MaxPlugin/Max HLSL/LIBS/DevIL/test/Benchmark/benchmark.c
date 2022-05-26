//-----------------------------------------------------------------------------
//
// ImageLib Benchmark Source
// Copyright (C) 2000 by Denton Woods
// Last modified:  08/21/2001 <--Y2K Compliant! =]
//
// Filename: testil/benchmark/benchmark.c
//
// Description:  Performs benchmarking of DevIL and ILU.
//					This requires the Simple DirectMedia Layer library,
//					available at http://www.libsdl.org
//
//-----------------------------------------------------------------------------


#include <il/il.h>
#include <il/ilu.h>
#include <SDL.h>
#include <stdlib.h>


#ifdef _WIN32
	#pragma comment(lib, "sdl.lib")
	#pragma comment(lib, "sdlmain.lib")
	#ifdef  _DEBUG
		#pragma comment(linker, "/NODEFAULTLIB:msvcrt.lib")
	#endif//_DEBUG
#endif


int main(int argc, char **argv)
{
	ILuint		id, Error;
	ILuint		i;
	ILdouble	avgtime, curtime, last_elapsed, cur_elapsed;

	if (argc < 2) {
		printf("Please specify a filename.\n");
		return 1;
	}

	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
		ilGetInteger(ILU_VERSION_NUM) < ILU_VERSION) {
		printf("DevIL version is different...exiting!\n");
		return 2;
	}

	ilInit();


	ilGenImages(1, &id);
	ilBindImage(id);

	if (SDL_Init(SDL_INIT_TIMER) < 0) {
		fprintf(stderr, "Couldn't load SDL: %s\n", SDL_GetError());
		return 3;
	}
	atexit(SDL_Quit);


	last_elapsed = cur_elapsed = SDL_GetTicks();

	
	ilHint(IL_MEM_SPEED_HINT, IL_FASTEST);
	printf("Using IL_FASTEST\n");
	avgtime = 0.0;

	for (i = 0; i < 10; i++) {
		ilLoadImage(argv[1]);
		cur_elapsed = SDL_GetTicks();
		curtime = cur_elapsed - last_elapsed;
		last_elapsed = cur_elapsed;
		avgtime += curtime;
		printf("%g\n", curtime);
	}
	
	printf("Average time:  %g\n", avgtime / 10.0);


	ilHint(IL_MEM_SPEED_HINT, IL_LESS_MEM);
	printf("Using IL_LESS_MEM\n");
	avgtime = 0.0;


	last_elapsed = cur_elapsed = SDL_GetTicks();

	for (i = 0; i < 10; i++) {
		ilLoadImage(argv[1]);
		cur_elapsed = SDL_GetTicks();
		curtime = cur_elapsed - last_elapsed;
		last_elapsed = cur_elapsed;
		avgtime += curtime;
		printf("%g\n", curtime);
	}
	printf("Average time:  %g\n", avgtime / 10.0);


	ilDeleteImages(1, &id);

	while ((Error = ilGetError())) {
		printf("Error: %s\n", iluErrorString(Error));
	}

	return 0;
}
