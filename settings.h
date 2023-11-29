
#ifndef SETTINGS_H
#define SETTINGS_H

// networking

#define PORT 32793
#define LISTEN 5

// game parameters

#define PLAYERS_REQUIRED 4
#define NUMBER_OF_BOT_PLAYERS 3

#define LEVEL_ON_SPAWN 5
#define XP_ON_SPAWN 1
#define XP_FOR_LEVEL_UP 8
#define LEVEL_UP_HEALTH_RESTORED_NUMERATOR   1
#define LEVEL_UP_HEALTH_RESTORED_DENOMINATOR 3

#define RESPAWN_TIME_MS 20e3
#define MINION_SPAWN_INTERVAL_MS 2800

#define HEALTH_STATES 8 // clarity vs bandwidth

#define PLAYERS_MAX 26 // includes minions and towers // TODO? rename to entities

// bot parameters

// reactions
#define BOT_REACTION_TIME_MS 400
#define MINION_REACTION_TIME_MS 800
#define TOWER_REACTION_TIME_MS 1600
// b-line or random movement
#define BOT_WILLPOWER     4
#define BOT_SCHIZOPHRENIA 5
#define MINION_WILLPOWER     1
#define MINION_SCHIZOPHRENIA 1
#define TOWER_WILLPOWER   1
#define TOWER_SCHIZOPHRENIA 1
// human wave or encirclement
#define BOT_HUMAN_WAVE_NUMERATOR  2
#define BOT_HUMAN_WAVE_DENOMINTOR 4
#define MINION_HUMAN_WAVE_NUMERATOR  5
#define MINION_HUMAN_WAVE_DENOMINTOR 6
#define TOWER_HUMAN_WAVE_NUMERATOR  1
#define TOWER_HUMAN_WAVE_DENOMINTOR 1

// map parameters

#define MAP_Y 20
#define MAP_X 40

#define SPAWN_AREA_Y 8
#define SPAWN_AREA_X 16
#define MINION_SPAWN_AREA_Y 2
#define MINION_SPAWN_AREA_X 4

#define TOWERS_PER_TEAM 1
#define TOWER_SPAWN_Y 3
#define TOWER_SPAWN_X 6

// keybinds

// keybind movement
#define KEY_MOVE_LEFT 'a'
#define KEY_MOVE_RIGHT 'd'
#define KEY_MOVE_UP 'w'
#define KEY_MOVE_DOWN 's'
// basic attack
#define KEY_BASIC_ATTACK 'j'
// heal ability
#define KEY_HEAL_ABILITY 'k'

#endif
