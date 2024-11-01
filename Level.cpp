#include "Level.h"
#include <string>
#include "Utils.h"
#include "data/DataCenter.h"
#include "data/MonsterCenter.h"
#include <allegro5/allegro_primitives.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"

using namespace std;

// fixed settings
const char level_path_format[] = "./assets/level/LEVEL%d.txt";
const vector<int> grid_size = {
	40, 40, 40, 40
};
const int monster_spawn_rate = 90;

void
Level::init() {
	level = -1;
	grid_w = -1;
	grid_h = -1;
	monster_spawn_counter = 0;
}

/**
 * @brief Loads level data from input file. The input file is required to follow the format.
 * @param lvl level index. The path format is a fixed setting in code.
 * @details The content of the input file should be formatted as follows:
 *          * Total number of monsters.
 *          * Number of each different number of monsters. The order and number follows the definition of MonsterType.
 *          * Indefinite number of Point (x, y), represented in grid format.
 * @see level_path_format
 * @see MonsterType
*/
void
Level::load_level(int lvl) {
	DataCenter *DC = DataCenter::get_instance();

	char buffer[50];
	sprintf(buffer, level_path_format, lvl);
	FILE *f = fopen(buffer, "r");
	GAME_ASSERT(f != nullptr, "cannot find level.");
	level = lvl;
	grid_w = DC->game_field_length / grid_size[lvl];
	grid_h = DC->game_field_length / grid_size[lvl];
	num_of_monsters.clear();
	road_path.clear();

	int num;
	// read total number of monsters & number of each monsters
	fscanf(f, "%d", &num);
	for(int i=0; i<static_cast<int>(MonsterType::MONSTERTYPE_MAX); ++i) {
		fscanf(f, "%d", &num);
		num_of_monsters.emplace_back(num);
	}

	// read road path
	while(fscanf(f, "%d", &num) != EOF) {
		int w = num % grid_w;
		int h = num / grid_h;
		road_path.emplace_back(w, h);
	}
	debug_log("<Level> load level %d.\n", lvl);
}

/**
 * @brief Updates monster_spawn_counter and create monster if needed.
*/
void
Level::update() {
	if(monster_spawn_counter) {
		monster_spawn_counter--;
		return;
	}
	DataCenter *DC = DataCenter::get_instance();
	MonsterCenter *MC = MonsterCenter::get_instance();

	for(int i=0; i<(int)(num_of_monsters.size()); ++i) {
		if(num_of_monsters[i] == 0) continue;
		MC->monsters.emplace_back(MC->create_monster(static_cast<MonsterType>(i), DC->level->get_road_path()));
		num_of_monsters[i]--;
		break;
	}
	monster_spawn_counter = monster_spawn_rate;
}

void
Level::draw() {
	if(level == -1) return;
	for(auto &[i, j] : road_path) {
		int x1 = i * grid_size[level];
		int y1 = j * grid_size[level];
		int x2 = x1 + grid_size[level];
		int y2 = y1 + grid_size[level];
		al_draw_filled_rectangle(x1, y1, x2, y2, al_map_rgb(255, 244, 173));
	}
}

bool
Level::is_onroad(const Rectangle &region) {
	for(auto &grid : road_path) {
		if(grid_to_region(grid).overlap(region))
			return true;
	}
	return false;
}

Rectangle
Level::grid_to_region(const Point &grid) const {
	int x1 = grid.x * grid_size[level];
	int y1 = grid.y * grid_size[level];
	int x2 = x1 + grid_size[level];
	int y2 = y1 + grid_size[level];
	return Rectangle(x1, y1, x2, y2);
}
