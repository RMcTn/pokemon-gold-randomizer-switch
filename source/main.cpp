#include <cstdio>
#include <cstring>
#include <time.h>
#include <unistd.h>
#include <string>
#include <dirent.h>

#include "rom.h"
#include "randomization_options.h"

#include <switch.h>

const int MAX_MENU_ITEMS = 11;

const int KEYBOARD_BUFFER_SIZE = 500;

const std::string GBC_FILE_EXTENSION = ".gbc";

const std::string menu_items[MAX_MENU_ITEMS] = {
	"Randomize intro pokemon", "Randomize starter pokemon", "Randomize evolutions", "Randomize wild pokemon", 
	"Randomize trainers", "Randomize gift pokemon", "Randomize static pokemon",
	"Randomize game corner pokemon", "Randomize static items", "Enable shiny mode", "Randomize pokemon colour palettes"
};
bool selected_options[MAX_MENU_ITEMS] = {true};
std::vector<RandomizationOptions> convert_to_randomization_options();

const std::string DEFAULT_ROM_FILENAME = "roms/gbc/Pokemon - Gold Version (UE) [C][!].gbc";

void randomize_rom(std::string rom_filename_to_load, std::string filename_to_save) {
	// Go through selected options and enable that for the randomizing
	Rom rom = Rom();
	if (rom.load(rom_filename_to_load)) {
		printf("Loaded successfully\n");
	} else {
		printf("Load was unsuccessful\n");
		return;
	}


	auto randomization_options = convert_to_randomization_options();
	rom.run(randomization_options);

	if (filename_to_save.size() <= 4) {
		// Can't possibly be a filename with the .gbc extension, so add it to the end
		// Possible the user gives us something like ".gbc" or "....", not handling these for now 
		filename_to_save.append(GBC_FILE_EXTENSION);
	}
	if (filename_to_save.substr(filename_to_save.size() - GBC_FILE_EXTENSION.size()) != GBC_FILE_EXTENSION) {
		filename_to_save.append(GBC_FILE_EXTENSION);
	}
	printf("Filename is %s\n", filename_to_save.c_str());
	if (rom.save(filename_to_save)) {
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

enum SelectMode {
	OptionSelectMode,
	FileSelectMode,
};

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

	Result rc;
	SwkbdConfig keyboard;
	char keyboard_buffer[KEYBOARD_BUFFER_SIZE] = {0};
	rc = swkbdCreate(&keyboard, 0);
	printf("swkbdCreate(): 0x%x\n", rc);

	if (R_SUCCEEDED(rc)) {
		swkbdConfigMakePresetDefault(&keyboard);
	} 
	const std::string keyboard_guide_text = "Filename to save (Max " + std::to_string(KEYBOARD_BUFFER_SIZE) + " characters)";

	std::vector<std::string> dir_entries;

	SelectMode current_select_mode = OptionSelectMode;

	std::string rom_filename_to_randomize = DEFAULT_ROM_FILENAME;

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

		if (buttons_pressed & HidNpadButton_Up) {
			menu_cursor -= 1;
		}

		if (current_select_mode == FileSelectMode) {
			if (buttons_pressed & HidNpadButton_A) {
				rom_filename_to_randomize = dir_entries[menu_cursor];
				current_select_mode = OptionSelectMode;
				menu_cursor = 0;
				continue;
			}
			if (menu_cursor < 0) menu_cursor = dir_entries.size() - 1;
			if (menu_cursor >= dir_entries.size()) menu_cursor = 0;
			printf("\x1b[1;1HMenu cursor: %i\n", menu_cursor);
			printf("File: %s\n", dir_entries[menu_cursor].c_str());
			printf("A to select ROM to randomize. Up and Down (D PAD) to scroll\n");
		}


		if (current_select_mode == OptionSelectMode) {
			if (buttons_pressed & HidNpadButton_Y) {
				menu_cursor = 0;
				current_select_mode = FileSelectMode;
				DIR* dir;
				struct dirent* ent;
				// TODO allow option to select DIRs/Go back up a DIR
				dir = opendir(""); // Open current-working-directory.
				if (dir==NULL) {
					printf("Failed to open dir.\n");
				} else {
					printf("Dir-listing for '':\n");
					dir_entries.clear();
					while ((ent = readdir(dir))) {
						std::string entry_name(ent->d_name);
						// d_namlen doesn't look like it's available on switch's dirent

						if (entry_name.size() < GBC_FILE_EXTENSION.size()) continue;

						if (entry_name.substr(entry_name.size() - GBC_FILE_EXTENSION.size()) != GBC_FILE_EXTENSION) continue;

						dir_entries.push_back(entry_name);
					}
					closedir(dir);
				}

			}

			if (buttons_pressed & HidNpadButton_A) {
				selected_options[menu_cursor] = !selected_options[menu_cursor];
			}

			if (buttons_pressed & HidNpadButton_B) {
				std::string filename_to_save_to;
				swkbdConfigSetGuideText(&keyboard, keyboard_guide_text.c_str());
				rc = swkbdShow(&keyboard, keyboard_buffer, sizeof(keyboard_buffer));
				if (R_SUCCEEDED(rc)) {
					filename_to_save_to = std::string(keyboard_buffer);
				}
				randomize_rom(rom_filename_to_randomize, filename_to_save_to);
				printf("Finished randomizer\n");
				consoleUpdate(NULL);
				sleep(2);
				exit_requested = 1;
				continue;
			}

			if (menu_cursor < 0) menu_cursor = MAX_MENU_ITEMS - 1;
			if (menu_cursor >= MAX_MENU_ITEMS) menu_cursor = 0;
			printf("\x1b[1;1HMenu cursor: %i\n", menu_cursor);
			printf("Selected option: %s. Enabled: %d\n", menu_items[menu_cursor].c_str(), selected_options[menu_cursor]);
			printf("B to randomize. A to toggle option. Up and Down (D PAD) to scroll\n");
			printf("Y to select ROM to randomize\n");
			printf("Current ROM to randomize: %s\n", rom_filename_to_randomize.c_str());
		}

		consoleUpdate(NULL);
	}
	swkbdClose(&keyboard);
	consoleExit(NULL);

	socketExit();
	return 0;
}

