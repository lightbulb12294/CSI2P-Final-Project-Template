#include "MonsterCenter.h"
#include "../Utils.h"
#include "../monsters/MonsterWolf.h"
#include "../monsters/MonsterCaveMan.h"
#include "../monsters/MonsterWolfKnight.h"
#include "../monsters/MonsterDemonNinja.h"
#include "TowerCenter.h"
#include "DataCenter.h"
#include "../Player.h"

using namespace std;

/**
 * @brief Handle all actions of monsters.
 * @details Update each monster, and detect if a monster reaches the end of the road path.
 * This also detects the collision of monsters and bullets shot by the towers, and will erase bullets and monsters (if HP downs to 0).
 * If a monster reaches the end, that means the monster will hit the player and erase itself.
*/
void
MonsterCenter::update() {
	// Update monsters first.
	for(auto &monster : monsters)
		monster->update();
	// If any bullet overlaps with any monster, we delete the bullet, reduce the HP of the monster, and delete the monster if necessary.
	DataCenter *DC = DataCenter::get_instance();
	TowerCenter *TC = TowerCenter::get_instance();
	for(int i=0; i<(int)(monsters.size()); ++i) {
		for(int j=0; j<(int)(TC->bullets.size()); ++j) {
			if(monsters[i]->shape->overlap(*(TC->bullets[j]->shape))) {
				monsters[i]->HP -= TC->bullets[j]->get_dmg();
				TC->bullets.erase(TC->bullets.begin()+j);
				--j;
				if(monsters[i]->HP <= 0) {
					// Monster gets killed. Player receives money.
					DC->player->coin += monsters[i]->get_money();
					monsters.erase(monsters.begin()+i);
					--i;
					// Since the current monsster is killed, we can directly proceed to next monster.
					break;
				}
			}
		}
	}
	// If any monster reaches the end, hurt the player and delete the monster.
	for(int i=0; i<(int)(monsters.size()); ++i) {
		if(monsters[i]->get_path().empty()) {
			monsters.erase(monsters.begin()+i);
			DC->player->HP--;
			--i;
		}
	}
}

void
MonsterCenter::draw() {
	for(auto &monster : monsters)
		monster->draw();
}

/**
 * @brief Create a Monster* instance by the type.
 * @param type the type of a monster.
 * @param path walk path of the monster. The path should be represented in road grid format.
 * @return The curresponding Monster* instance.
 * @see Level::grid_to_region(const Point &grid) const
*/
Monster*
MonsterCenter::create_monster(MonsterType type, const vector<Point> &path) {
	switch(type) {
		case MonsterType::WOLF: {
			return new MonsterWolf(path);
		}
		case MonsterType::CAVEMAN: {
			return new MonsterCaveMan(path);
		}
		case MonsterType::WOLFKNIGHT: {
			return new MonsterWolfKnight(path);
		}
		case MonsterType::DEMONNIJIA: {
			return new MonsterDemonNinja(path);
		}
		case MonsterType::MONSTERTYPE_MAX: {}
	}
	GAME_ASSERT(false, "monster type error.");
}
