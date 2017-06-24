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
#include <cpixprint.h>
#include <cpixlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define SORT_ARRAY_LEN 160

#define MAX_TICK_SAMPLES 100

enum Sorts {
	BUBBLE
};

typedef struct {
	u32 switches;
	u32 position;
} SV_Bubble;

union SortVars {
	SV_Bubble sv_bubble;
};

void error_quit(u8 * message) {
	fprintf(stderr, "%s SDL_Error:\n\t%s\n", message, SDL_GetError());
	exit(1);
}

void draw_array_software(
	SDL_Surface * screen_surface,
	u8 * sort_array, int sa_len, SDL_Color * highlights) {
	int bar_width = SCREEN_WIDTH / sa_len;
	for (int i = 0; i < sa_len; i++) {
		int bar_height = (SCREEN_HEIGHT * sort_array[i]) / 256;
		SDL_Surface * bar_surface = SDL_CreateRGBSurface(
			0, bar_width, bar_height, 32, 0, 0, 0, 0);
		SDL_FillRect(bar_surface, 0,
			SDL_MapRGB(bar_surface->format,
				highlights[i].r, highlights[i].g, highlights[i].b));
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
	u8 * sort_array, int sa_len, SDL_Color * highlights) {
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

u8 sort_step(enum Sorts sort, union SortVars * sort_vars, u8 * sort_array, u32 sort_array_len, SDL_Color * highlights, u8 first_loop) {
	switch (sort) {
		case BUBBLE: {
			SV_Bubble * sv = &(sort_vars->sv_bubble);
			if (first_loop) {
				sv->switches = 0;
				sv->position = 0;
			}
			if (sv->position + 1 >= sort_array_len) {
				if (sv->switches == 0) {
					return ~0;
				} else {
					sv->switches = 0;
					sv->position = 0;
					return 0;
				}
			}
			if (sort_array[sv->position] > sort_array[sv->position + 1]) {
				u8 temp = sort_array[sv->position];
				sort_array[sv->position] = sort_array[sv->position + 1];
				sort_array[sv->position + 1] = temp;
				sv->switches++;
				highlights[sv->position + 1].r = 0;
				highlights[sv->position + 1].g = 255;
				highlights[sv->position + 1].b = 0;
			}
			sv->position++;
			return 0;
		} break;
	}
}

int run_visualization(u8 use_hardware) {
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
	u8 sort_array[SORT_ARRAY_LEN];
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
	u8 running = ~0;
	SDL_Event event;
	union SortVars sort_vars;
	u8 first_loop = ~0;
	u8 sorted = 0;
	while (running) {
		int frame_start_time = SDL_GetTicks();
		/* PROCESS EVENTS */
		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT) {
				running = 0;
			}
		}
		/* SORTING */
		SDL_Color * highlights = malloc(sizeof(SDL_Color) * SORT_ARRAY_LEN);
		for (int i = 0; i < SORT_ARRAY_LEN; i++) {
			highlights[i].r = 255;
			highlights[i].g = 0;
			highlights[i].b = 0;
			highlights[i].a = 255;
		}
		if (!sorted) {
			sorted = sort_step(BUBBLE, &sort_vars, sort_array, SORT_ARRAY_LEN, highlights, first_loop);
		}
		/* DRAWING */
		if (use_hardware) {
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);
			draw_array_hardware(renderer, sort_array, SORT_ARRAY_LEN, highlights);
			SDL_RenderPresent(renderer);
		} else {
			SDL_FillRect(
				screen_surface, 0,
				SDL_MapRGB(screen_surface->format, 255, 255, 255)
			);
			draw_array_software(screen_surface, sort_array, SORT_ARRAY_LEN, highlights);
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
		first_loop = 0;
	}
	
	/* CLEANUP */
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}

int main(int argc, char ** argv) {
	srand(time(0));
	u8 use_hardware = 0;
	char c;
	while ((c = getopt(argc, argv, "h")) != -1) {
		if (c == '?') {
			exit(1);
		} else if (c == 'h') {
			use_hardware = ~0;
		}
	}
	printf("Launching in %s mode.\n", use_hardware ? "hardware" : "software");
	run_visualization(use_hardware);
	return 0;
}
