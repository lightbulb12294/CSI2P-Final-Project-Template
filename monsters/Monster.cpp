#include "Monster.h"
#include "../data/DataCenter.h"
#include "../data/MonsterCenter.h"
#include "../data/ImageCenter.h"
#include <allegro5/allegro_primitives.h>
#include "../Level.h"
#include "../shapes/Circle.h"
#include "../shapes/Point.h"
#include "../shapes/Rectangle.h"

using namespace std;

// fixed settings
enum class Dir {
	UP, DOWN, LEFT, RIGHT
};
const char dir_path_prefix[][10] = {
	"UP", "DOWN", "LEFT", "RIGHT"
};

/**
 * @brief Given velocity of x and y direction, determine which direction the monster should face.
*/
Dir convert_dir(const Point &v) {
	if(v.y < 0 && abs(v.y) >= abs(v.x))
		return Dir::UP;
	if(v.y > 0 && abs(v.y) >= abs(v.x))
		return Dir::DOWN;
	if(v.x < 0 && abs(v.x) >= abs(v.y))
		return Dir::LEFT;
	if(v.x > 0 && abs(v.x) >= abs(v.y))
		return Dir::RIGHT;
	return Dir::RIGHT;
}

Monster::Monster(const vector<Point> &path, MonsterType type) {
	DataCenter *DC = DataCenter::get_instance();

	shape.reset(new Rectangle(0, 0, 0, 0));
	this->type = type;
	dir = Dir::RIGHT;
	bitmap_img_id = 0;
	bitmap_switch_counter = 0;
	for(auto &p : path)
		this->path.push(p);
	if(!path.empty()) {
		const Point &grid = this->path.front();
		const Rectangle &region = DC->level->grid_to_region(grid);
		// temporarily set the bounding box to the center (no area)
		shape.reset(new Rectangle(region.center_x(), region.center_y(), region.center_x(), region.center_y()));
		this->path.pop();
	}
}

/**
 * @brief Update the monster.
 * @details This update function updates the following things in order:
 *          * Move pose of the current facing direction (bitmap_img_id).
 *          * Current position (center of the hit box). The position is moved based on the center of the hit box (Rectangle). If the center of this monster reaches the center of the first point of path, the function will proceed to the next point of path.
 *          * Update the real bounding box by the center of the hit box calculated as above.
*/
void
Monster::update() {
	Rectangle *rec = static_cast<Rectangle*>(shape.get());
	DataCenter *DC = DataCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();

	if(bitmap_switch_counter) bitmap_switch_counter--;
	else {
		bitmap_img_id = (bitmap_img_id + 1) % (bitmap_img_ids[static_cast<int>(dir)].size());
		bitmap_switch_counter = bitmap_switch_freq;
	}
	double movement = v / DC->FPS;
	while(!path.empty() && movement > 0) {
		const Point &grid = this->path.front();
		const Rectangle &region = DC->level->grid_to_region(grid);
		const Point &next_goal = Point(region.center_x(), region.center_y());

		double d = Point::dist(Point(rec->center_x(), rec->center_y()), next_goal);
		Dir tmpdir;
		if(d < movement) {
			movement -= d;
			tmpdir = convert_dir(Point(next_goal.x-rec->center_x(), next_goal.y-rec->center_y()));
			rec->x1 = rec->x2 = next_goal.x;
			rec->y1 = rec->y2 = next_goal.y;
			path.pop();
		} else {
			double dx = (next_goal.x - rec->center_x()) / d * movement;
			double dy = (next_goal.y - rec->center_y()) / d * movement;
			tmpdir = convert_dir(Point(dx, dy));
			rec->x1 += dx;
			rec->x2 += dx;
			rec->y1 += dy;
			rec->y2 += dy;
			movement = 0;
		}
		if(tmpdir != dir) {
			dir = tmpdir;
			bitmap_img_id = 0;
		}
	}
	// update real bounding box for monster
	char buffer[50];
	sprintf(
		buffer, "%s/%s_%d.png",
		monster_imgs_root_path[static_cast<int>(type)],
		dir_path_prefix[static_cast<int>(dir)],
		bitmap_img_ids[static_cast<int>(dir)][bitmap_img_id]);
	ALLEGRO_BITMAP *bitmap = IC->get(buffer);
	const double &cx = rec->center_x();
	const double &cy = rec->center_y();
	const int &h = al_get_bitmap_width(bitmap) * 0.8;
	const int &w = al_get_bitmap_height(bitmap) * 0.8;
	rec->x1 = cx - w/2.;
	rec->y1 = cy - h/2.;
	rec->x2 = cx - w/2. + w;
	rec->y2 = cy - h/2. + h;
}

/**
 * @brief Draw the monster on the display.
*/
void
Monster::draw() {
	ImageCenter *IC = ImageCenter::get_instance();
	char buffer[50];
	sprintf(
		buffer, "%s/%s_%d.png",
		monster_imgs_root_path[static_cast<int>(type)],
		dir_path_prefix[static_cast<int>(dir)],
		bitmap_img_ids[static_cast<int>(dir)][bitmap_img_id]);
	ALLEGRO_BITMAP *bitmap = IC->get(buffer);
	al_draw_bitmap(
		bitmap,
		shape->center_x() - al_get_bitmap_width(bitmap)/2,
		shape->center_y() - al_get_bitmap_height(bitmap)/2, 0);
}
