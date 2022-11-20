/* Mini SDL Demo
 * featuring SDL2 + SDL2_mixer + SDL2_image + SDL2_ttf
 * on Nintendo Switch using libnx
 *
 * Copyright 2018 carsten1ns
 *           2020 WinterMute
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <cstdio>
#include <time.h>
#include <unistd.h>
#include <string>

#include "rom.h"
#include "randomization_options.h"

#include <switch.h>


// some switch buttons
#define JOY_A     0
#define JOY_B     1
#define JOY_X     2
#define JOY_Y     3
#define JOY_PLUS  10
#define JOY_MINUS 11
#define JOY_LEFT  12
#define JOY_UP    13
#define JOY_RIGHT 14
#define JOY_DOWN  15

#define SCREEN_W 1280
#define SCREEN_H 720

const int MAX_MENU_ITEMS = 11;

const std::string menu_items[MAX_MENU_ITEMS] = {
	"Randomize intro pokemon", "Randomize starter pokemon", "Randomize evolutions", "Randomize wild pokemon", 
	"Randomize trainers", "Randomize gift pokemon", "Randomize static pokemon",
	"Randomize game corner pokemon", "Randomize static items", "Enable shiny mode", "Randomize pokemon colour palettes"
};
bool selected_options[MAX_MENU_ITEMS] = {true};

std::vector<RandomizationOptions> convert_to_randomization_options();

void randomize_rom() {
	// Go through selected options and enable that for the randomizing
	Rom rom = Rom();
	if (rom.load("roms/gbc/Pokemon - Gold Version (UE) [C][!].gbc")) {
		printf("Loaded successfully\n");
	} else {
		printf("Load was unsuccessful\n");
		return;
	}


	auto randomization_options = convert_to_randomization_options();
	rom.run(randomization_options);
	// TODO add filename to save to
	if (rom.save()) {
		printf("Save was successful\n");
	} else {
		printf("Save was unsuccessful\n");
	}
}

std::vector<RandomizationOptions> convert_to_randomization_options() {
	// Simple enough, I guess
	std::vector<RandomizationOptions> randomization_options;
	randomization_options.reserve(MAX_MENU_ITEMS);
	if (selected_options[0]) {
		randomization_options.push_back(RANDOMIZE_INTRO_POKEMON);
	}
	if (selected_options[1]) {
		randomization_options.push_back(RANDOMIZE_STARTER_POKEMON);
	}
	if (selected_options[2]) {
		randomization_options.push_back(RANDOMIZE_EVOLUTIONS);
	}
	if (selected_options[3]) {
		randomization_options.push_back(RANDOMIZE_WILD_POKEMON);
	}
	if (selected_options[4]) {
		randomization_options.push_back(RANDOMIZE_TRAINERS);
	}
	if (selected_options[5]) {
		randomization_options.push_back(RANDOMIZE_GIFT_POKEMON);
	}
	if (selected_options[6]) {
		randomization_options.push_back(RANDOMIZE_STATIC_POKEMON);
	}
	if (selected_options[7]) {
		randomization_options.push_back(RANDOMIZE_GAME_CORNER_POKEMON);
	}
	if (selected_options[8]) {
		randomization_options.push_back(RANDOMIZE_STATIC_ITEMS);
	}
	if (selected_options[9]) {
		randomization_options.push_back(ENABLE_SHINY_MODE);
	}
	if (selected_options[10]) {
		randomization_options.push_back(RANDOMIZE_POKEMON_PALLETES);
	}

	return randomization_options;
}

int main(int argc, char** argv) {

	socketInitializeDefault();
	nxlinkStdio();

	consoleInit(NULL);
	padConfigureInput(1, HidNpadStyleSet_NpadStandard);

	PadState pad;
	padInitializeDefault(&pad);

	int exit_requested = 0;

	int menu_cursor = 0;


	for (int i = 0; i < MAX_MENU_ITEMS; i++) {
		selected_options[i] = true;
	}


	while (!exit_requested && appletMainLoop()) {
		consoleClear();

		padUpdate(&pad);

		u64 buttons_pressed = padGetButtonsDown(&pad);

		if (buttons_pressed & HidNpadButton_Plus) {
			exit_requested = 1;
			continue;
		}

		if (buttons_pressed & HidNpadButton_Down) {

			menu_cursor += 1;

		}

		if (buttons_pressed & HidNpadButton_A) {
			selected_options[menu_cursor] = !selected_options[menu_cursor];
		}

		if (buttons_pressed & HidNpadButton_B) {
			printf("About to call it\n");
			randomize_rom();
			printf("Finished randomize\n");
			consoleUpdate(NULL);
			sleep(2);
			exit_requested = 1;
			continue;
		}

		if (buttons_pressed & HidNpadButton_Up) {

			menu_cursor -= 1;

		}

		if (menu_cursor < 0) menu_cursor = MAX_MENU_ITEMS - 1;
		if (menu_cursor >= MAX_MENU_ITEMS) menu_cursor = 0;
		printf("\x1b[1;1HMenu cursor: %i\n", menu_cursor);
		printf("Selected option: %s. Enabled: %d\n", menu_items[menu_cursor].c_str(), selected_options[menu_cursor]);
		printf("B to randomize. A to toggle option. Up and Down (D PAD) to scroll\n");
		consoleUpdate(NULL);
	}

	consoleExit(NULL);

	socketExit();
	return 0;
}

