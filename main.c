/** COPYRIGHT (C) 2017
 ** https://pixlark.github.io/
 *
 ** main.c
 * 
 * This file contains a 
 *
 */

#include <SDL2/SDL.h>
#include <pixint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

void error_quit(char * message) {
	fprintf(stderr, "%s SDL_Error:\n\t%s\n", message, SDL_GetError());
	exit(1);
}

void draw_array(
	SDL_Surface * screen_surface,
	u8 * sort_array, u16 sa_len) {
	u16 bar_width = SCREEN_WIDTH / sa_len;
	for (u16 i = 0; i < sa_len; i++) {
		u16 bar_height = (SCREEN_HEIGHT * sort_array[i]) / 256;
		SDL_Surface * bar_surface = SDL_CreateRGBSurface(
			0, bar_width, bar_height, 32, 0, 0, 0, 0);
		SDL_FillRect(bar_surface, 0,
			SDL_MapRGB(bar_surface->format, 255, 0, 0));
		SDL_Rect bar_rect;
		bar_rect.x = i * bar_width;
		bar_rect.w = bar_width;
		bar_rect.y = SCREEN_HEIGHT - bar_height;
		bar_rect.h = bar_height;
		SDL_BlitSurface(bar_surface, 0, screen_surface, &bar_rect);
		SDL_FreeSurface(bar_surface);
	}
}

int main() {
	/* INITIALIZATION */
	SDL_Window * window;
	SDL_Surface * screen_surface;
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		error_quit("Trouble initializing.");
	}
	window = SDL_CreateWindow(
		"Sorting Visualizer",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN
	);
	if (window == 0) {
		error_quit("Trouble creating the window.");
	}
	screen_surface = SDL_GetWindowSurface(window);
	if (screen_surface == 0) {
		error_quit("Trouble getting window surface.");
	}

	srand(time(0));
	
	#define SORT_ARRAY_LEN 160
	u8 sort_array[SORT_ARRAY_LEN];
	for (int i = 0; i < SORT_ARRAY_LEN; i++) {
		sort_array[i] = rand() % 256;
	}
	
	/* MAIN LOOP */
	u8 running = 0xFF;
	SDL_Event event;
	while (running) {
		/* PROCESS EVENTS */
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = 0x00;
			}
		}
		/* SORTING */
		u16 el1_i = rand() % SORT_ARRAY_LEN;
		u16 el2_i = rand() % SORT_ARRAY_LEN;
		u8 temp = sort_array[el1_i];
		sort_array[el1_i] = sort_array[el2_i];
		sort_array[el2_i] = temp;
		/* DRAWING */
		SDL_FillRect(
			screen_surface, 0,
			SDL_MapRGB(screen_surface->format, 255, 255, 255)
		);
		draw_array(screen_surface, sort_array, SORT_ARRAY_LEN);
		SDL_UpdateWindowSurface(window);
	}
	
	/* CLEANUP */
	free(sort_array);
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}
