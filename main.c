/** COPYRIGHT (C) 2017
 ** https://pixlark.github.io/
 *
 ** main.c
 * 
 * This file contains a 
 *
 */

#include <SDL2/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SORT_ARRAY_LEN 40

#define MAX_TICK_SAMPLES 100

void error_quit(char * message) {
	fprintf(stderr, "%s SDL_Error:\n\t%s\n", message, SDL_GetError());
	exit(1);
}

void draw_array_software(
	SDL_Surface * screen_surface,
	uint8_t * sort_array, int sa_len) {
	int bar_width = SCREEN_WIDTH / sa_len;
	for (int i = 0; i < sa_len; i++) {
		int bar_height = (SCREEN_HEIGHT * sort_array[i]) / 256;
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

void draw_array_hardware(
	SDL_Renderer * renderer,
	uint8_t * sort_array, int sa_len) {
	int bar_width = SCREEN_WIDTH / sa_len;
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	for (int i = 0; i < sa_len; i++) {
		int bar_height = (SCREEN_HEIGHT * sort_array[i]) / 256;
		SDL_Rect bar_rect;
		bar_rect.x = i * bar_width;
		bar_rect.w = bar_width;
		bar_rect.y = SCREEN_HEIGHT - bar_height;
		bar_rect.h = bar_height;
		SDL_RenderFillRect(renderer, &bar_rect);
	}
}

int run_visualization(char use_hardware) {
	SDL_Window * window;
	SDL_Surface * screen_surface;
	SDL_Renderer * renderer;
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
	if (use_hardware) {
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == 0) {
			error_quit("Trouble getting window renderer.");
		}
	} else {
		screen_surface = SDL_GetWindowSurface(window);
		if (screen_surface == 0) {
			error_quit("Trouble getting window surface.");
		}
	}
	uint8_t sort_array[SORT_ARRAY_LEN];
	for (int i = 0; i < SORT_ARRAY_LEN; i++) {
		sort_array[i] = rand() % 256;
	}

	/* FRAMERATE SETUP */
	int tick_index = 0;
	int tick_sum = 0;
	int tick_list[MAX_TICK_SAMPLES];
	int tick_print_rollover = 1000;
	int tick_print_clock = 0;
	for (int i = 0; i < MAX_TICK_SAMPLES; i++) {
		tick_list[i] = 0;
	}
	
	/* MAIN LOOP */
	uint8_t running = 0xFF;
	SDL_Event event;
	while (running) {
		int frame_start_time = SDL_GetTicks();
		/* PROCESS EVENTS */
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = 0x00;
			}
		}
		/* SORTING */
		int el1_i = rand() % SORT_ARRAY_LEN;
		int el2_i = rand() % SORT_ARRAY_LEN;
		uint8_t temp = sort_array[el1_i];
		sort_array[el1_i] = sort_array[el2_i];
		sort_array[el2_i] = temp;
		/* DRAWING */
		if (use_hardware) {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);
			draw_array_hardware(renderer, sort_array, SORT_ARRAY_LEN);
			SDL_RenderPresent(renderer);
		} else {
			SDL_FillRect(
				screen_surface, 0,
				SDL_MapRGB(screen_surface->format, 255, 255, 255)
			);
			draw_array_software(screen_surface, sort_array, SORT_ARRAY_LEN);
			SDL_UpdateWindowSurface(window);
		}
		/* FRAMERATE */
		int frame_time = SDL_GetTicks() - frame_start_time;
		tick_sum -= tick_list[tick_index];
		tick_sum += frame_time;
		tick_list[tick_index] = frame_time;
		if (++tick_index == MAX_TICK_SAMPLES) {
			tick_index = 0;
		}
		tick_print_clock += frame_time;
		if (tick_print_clock > tick_print_rollover) {
			tick_print_clock = 0;
			printf("%f\n", 1000 * (1/((double)tick_sum / MAX_TICK_SAMPLES)));
		}
	}
	
	/* CLEANUP */
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}

int main(int argc, char ** argv) {
	srand(time(0));
	char use_hardware = 0x00;
	char c;
	while ((c = getopt(argc, argv, "h")) != -1) {
		if (c == '?') {
			exit(1);
		} else if (c == 'h') {
			use_hardware = 0xFF;
		}
	}
	printf("Launching in %s mode.\n", use_hardware ? "hardware" : "software");
	run_visualization(use_hardware);
	return 0;
}
