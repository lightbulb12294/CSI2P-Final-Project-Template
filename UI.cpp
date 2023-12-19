#include "UI.h"
#include "data/DataCenter.h"
#include "data/ImageCenter.h"
#include "data/FontCenter.h"
#include "data/TowerCenter.h"
#include <algorithm>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>
#include "shapes/Point.h"
#include "shapes/Rectangle.h"
#include "Player.h"
#include "Level.h"

// fixed settings
const char love_img_path[] = "./assets/image/love.png";
const int love_img_padding = 5;
const int tower_img_left_padding = 30;
const int tower_img_top_padding = 30;

void
UI::init() {
	DataCenter *DC = DataCenter::get_instance();
	ImageCenter *IC = ImageCenter::get_instance();
	love = IC->get(love_img_path);
	int tl_x = DC->game_field_length + tower_img_left_padding;
	int tl_y = tower_img_top_padding;
	int max_height = 0;
	// arrange tower shop
	for(int i=0; i<(int)(TowerType::TOWERTYPE_MAX); ++i) {
		ALLEGRO_BITMAP *bitmap = IC->get(tower_menu_img_path[i]);
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		if(tl_x + w > DC->window_width) {
			tl_x = DC->game_field_length + tower_img_left_padding;
			tl_y += max_height + tower_img_top_padding;
			max_height = 0;
		}
		tower_items.emplace_back(bitmap, Point(tl_x, tl_y), tower_price[i]);
		tl_x += w + tower_img_left_padding;
		max_height = std::max(max_height, h);
	}
	puts("UI state: change to HALT");
	state = STATE::HALT;
	on_item = -1;
}

void
UI::update() {
	DataCenter *DC = DataCenter::get_instance();
	TowerCenter *TC = TowerCenter::get_instance();
	const Point &mouse = DC->mouse;

	switch(state) {
		case STATE::HALT: {
			for(size_t i=0; i<tower_items.size(); ++i) {
				auto &[bitmap, p, price] = tower_items[i];
				int w = al_get_bitmap_width(bitmap);
				int h = al_get_bitmap_height(bitmap);
				// hover on a shop tower item
				if(mouse.overlap(Rectangle(p.x, p.y, p.x+w, p.y+h))) {
					on_item = i;
					puts("UI state: change to HOVER");
					state = STATE::HOVER;
					break;
				}
			}
			break;
		} case STATE::HOVER: {
			auto &[bitmap, p, price] = tower_items[on_item];
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			if(!mouse.overlap(Rectangle(p.x, p.y, p.x+w, p.y+h))) {
				on_item = -1;
				puts("UI state: change to HALT");
				state = STATE::HALT;
				break;
			}
			// click mouse left button
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
				// no money
				if(price > DC->player->coin) {
					puts("UI: no money.");
					break;
				}
				puts("UI state: change to SELECT");
				state = STATE::SELECT;
			}
			break;
		} case STATE::SELECT: {
			// click mouse left button: place
			if(DC->mouse_state[1] && !DC->prev_mouse_state[1]) {
				puts("UI state: change to PLACE");
				state = STATE::PLACE;
			}
			// click mouse right button: cancel
			if(DC->mouse_state[2] && !DC->prev_mouse_state[2]) {
				on_item = -1;
				puts("UI state: change to HALT");
				state = STATE::HALT;
			}
			break;
		} case STATE::PLACE: {
			// check placement legality
			ALLEGRO_BITMAP *bitmap = TC->get_bitmap(static_cast<TowerType>(on_item));
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			Rectangle place_region(mouse.x-w/2, mouse.y-h/2, DC->mouse.x+w/2, DC->mouse.y+h/2);
			bool place = true;
			// tower cannot be placed on the road
			place &= (!DC->level->is_onroad(place_region));
			// tower cannot intersect with other towers
			for(auto &tower : TC->towers) {
				place &= (!place_region.overlap(tower->get_region()));
			}
			if(!place) {
				puts("UI: Tower place failed.");
			} else {
				TC->towers.emplace_back(TC->create_tower(static_cast<TowerType>(on_item), mouse));
				DC->player->coin -= std::get<2>(tower_items[on_item]);
			}
			puts("UI state: change to HALT");
			state = STATE::HALT;
			break;
		}
	}
}

void
UI::draw() {
	DataCenter *DC = DataCenter::get_instance();
	FontCenter *FC = FontCenter::get_instance();
	TowerCenter *TC = TowerCenter::get_instance();
	const Point &mouse = DC->mouse;
	// draw HP
	const int &game_field_length = DC->game_field_length;
	const int &player_HP = DC->player->HP;
	int love_width = al_get_bitmap_width(love);
	for(int i=1; i<=player_HP; ++i) {
		al_draw_bitmap(love, game_field_length - (love_width + love_img_padding) * i, love_img_padding, 0);
	}
	// draw coin
	const int &player_coin = DC->player->coin;
	al_draw_textf(
		FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
		game_field_length+love_img_padding, love_img_padding,
		ALLEGRO_ALIGN_LEFT, "coin: %5d", player_coin);
	// draw tower shop items
	for(auto &[bitmap, p, price] : tower_items) {
		int w = al_get_bitmap_width(bitmap);
		int h = al_get_bitmap_height(bitmap);
		al_draw_bitmap(bitmap, p.x, p.y, 0);
		al_draw_rectangle(
			p.x-1, p.y-1,
			p.x+w+1, p.y+h+1,
			al_map_rgb(0, 0, 0), 1);
		al_draw_textf(
			FC->courier_new[FontSize::MEDIUM], al_map_rgb(0, 0, 0),
			p.x+w/2, p.y+h,
			ALLEGRO_ALIGN_CENTRE, "%d", price);
	}

	static Tower *selected_tower = nullptr;
	switch(state) {
		case STATE::HALT: {
			if(selected_tower != nullptr) {
				delete selected_tower;
				selected_tower = nullptr;
			}
			break;
		} case STATE::HOVER: {
			auto &[bitmap, p, price] = tower_items[on_item];
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			al_draw_filled_rectangle(p.x, p.y, p.x+w, p.y+h, al_map_rgba(50, 50, 50, 64));
			break;
		}
		case STATE::SELECT: {
			if(selected_tower == nullptr) {
				selected_tower = TC->create_tower(static_cast<TowerType>(on_item), mouse);
			} else {
				selected_tower->shape.x = mouse.x;
				selected_tower->shape.y = mouse.y;
			}
		}
		case STATE::PLACE: {
			ALLEGRO_BITMAP *bitmap = TC->get_bitmap(static_cast<TowerType>(on_item));
			al_draw_filled_circle(mouse.x, mouse.y, selected_tower->shape.r, al_map_rgba(255, 0, 0, 32));
			int w = al_get_bitmap_width(bitmap);
			int h = al_get_bitmap_height(bitmap);
			al_draw_bitmap(bitmap, mouse.x - w / 2, mouse.y - h / 2, 0);
			break;
		}
	}
}
