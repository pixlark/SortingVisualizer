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

#define SORT_ARRAY_LEN 80

#define MAX_TICK_SAMPLES 100

enum Sorts {
	BUBBLE,
	SELECTION,
	INSERTION
};

typedef struct {
	u32 switches;
	u32 position;
	u32 size;
} SV_Bubble;

typedef struct {
	u32 smallest_index;
	u32 next_check;
	u32 position;
} SV_Selection;

typedef struct {
	u32 moving_index;
	u32 position;
} SV_Insertion;

union SortVars {
	SV_Bubble sv_bubble;
	SV_Selection sv_selection;
	SV_Insertion sv_insertion;
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
	for (int i = 0; i < sa_len; i++) {
		SDL_SetRenderDrawColor(renderer, highlights[i].r, highlights[i].g, highlights[i].b, 255);
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
		/*
		 * BUBBLE SORT
		 */
		case BUBBLE: {
			SV_Bubble * sv = &(sort_vars->sv_bubble);
			// Initialize if first loop
			if (first_loop) {
				sv->switches = 0;
				sv->position = 0;
				sv->size = sort_array_len;
			}
			// If we've compared up to the end of the list
			if (sv->position + 1 >= sv->size) {
				if (sv->switches == 0) {
					return ~0;
				} else {
					sv->switches = 0;
					sv->position = 0;
					sv->size--;
					return 0;
				}
			}
			// If we need to swap
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
        /*
		 * SELECTION SORT
		 */
		case SELECTION: {
			SV_Selection * sv = &(sort_vars->sv_selection);
			// Initialize if first loop
			if (first_loop) {
				sv->smallest_index = 0;
				sv->position = 0;
				sv->next_check = 1;
			}
			// Check for completion
			if (sv->position >= sort_array_len - 1) {
				return ~0;
			}
			// If we've checked all the elements
			if (sv->next_check >= sort_array_len) {
				u32 temp = sort_array[sv->smallest_index];
				sort_array[sv->smallest_index] = sort_array[sv->position];
				sort_array[sv->position] = temp;
				sv->position++;
				sv->smallest_index = sv->position;
				sv->next_check = sv->position + 1;
				return 0;
			}
			// Check next element
			highlights[sv->next_check].r = 0;
			highlights[sv->next_check].g = 0;
			highlights[sv->next_check].b = 255;
			if (sort_array[sv->next_check] < sort_array[sv->smallest_index]) {
				sv->smallest_index = sv->next_check;
			}
			highlights[sv->smallest_index].r = 0;
			highlights[sv->smallest_index].g = 255;
			highlights[sv->smallest_index].b = 0;
			sv->next_check++;
			return 0;
		} break;
        /*
		 * INSERTION SORT
		 */
		case INSERTION: {
			SV_Insertion * sv = &(sort_vars->sv_insertion);
			// Initialize if first loop
			if (first_loop) {
				sv->moving_index = 1;
				sv->position = 1;
			}
			// Check for completion
			if (sv->position >= sort_array_len) {
				return ~0;
			}
			// Compare
			if (sv->moving_index == 0 || sort_array[sv->moving_index] > sort_array[sv->moving_index - 1]) {
				highlights[sv->moving_index].r = 0;
				highlights[sv->moving_index].g = 255;
				highlights[sv->moving_index].b = 0;
				sv->position++;
				sv->moving_index = sv->position;
			} else {
				highlights[sv->moving_index].r = 0;
				highlights[sv->moving_index].g = 0;
				highlights[sv->moving_index].b = 255;
				u32 temp = sort_array[sv->moving_index];
				sort_array[sv->moving_index] = sort_array[sv->moving_index - 1];
				sort_array[sv->moving_index - 1] = temp;
				sv->moving_index--;
			}
			return 0;
		} break;
	}
}

int run_visualization(enum Sorts algorithm, u32 delay, u8 use_hardware, u8 echo_fps) {
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

	/* LOG FILE */
	FILE * log = fopen("profile.log", "a");
	time_t log_time = time(0);
	struct tm log_time_tm = *localtime(&log_time);
	char time_buffer[100];
	sprintf(time_buffer, "%02d:%02d:%02d : %s mode\n",
			log_time_tm.tm_hour, log_time_tm.tm_min, log_time_tm.tm_sec,
			use_hardware ? "hardware" : "software");
	fwrite(time_buffer, sizeof(char), strlen(time_buffer), log);
	
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
			sorted = sort_step(algorithm, &sort_vars, sort_array, SORT_ARRAY_LEN, highlights, first_loop);
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
			char * fps_str = malloc(sizeof(char) * 12);
			sprintf(fps_str, "%f\n", 1000 * (1/((double)tick_sum / MAX_TICK_SAMPLES)));
			if (echo_fps) {
				printf("%s", fps_str);
			}
			fwrite(fps_str, sizeof(char), strlen(fps_str), log);
			free(fps_str);
		}
		first_loop = 0;
		if (delay != 0) {
			SDL_Delay(delay);
		}
	}
	
	/* CLEANUP */
	SDL_DestroyWindow(window);
	SDL_Quit();
	
	return 0;
}

int main(int argc, char ** argv) {
	srand(time(0));
	u8 use_hardware = 0;
	enum Sorts algorithm;
	u8 algorithm_specified = 0;
	u32 delay = 0;
	u8 echo_fps = 0;
	
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-a") == 0) {
			algorithm_specified = ~0;
			char * alg_name = argv[i + 1];
			i++;
			for (int ci = 0;;ci++) {
				if (alg_name[ci] == '\0') break;
				alg_name[ci] = tolower(alg_name[ci]);
			}
			if (strcmp(alg_name, "bubble") == 0) {
				algorithm = BUBBLE;
			} else if (strcmp(alg_name, "selection") == 0) {
				algorithm = SELECTION;
			} else if (strcmp(alg_name, "insertion") == 0) {
				algorithm = INSERTION;
			} else {
				printf("Invalid algorithm name.\n");
				exit(2);
			}
		} else if (strcmp(argv[i], "-h") == 0) {
			use_hardware = ~0;
		} else if (strcmp(argv[i], "-d") == 0) {
			char * delay_str = argv[i + 1];
			i++;
			delay = atoi(delay_str);
		} else if (strcmp(argv[i], "-fps") == 0) {
			echo_fps = ~0;
		}
	}

	if (!algorithm_specified) {
		printf("Specify an algorithm with -a.\n");
		exit(2);
	}
	
	printf("Launching in %s mode.\n", use_hardware ? "hardware" : "software");
	run_visualization(algorithm, delay, use_hardware, echo_fps);
	return 0;
}
