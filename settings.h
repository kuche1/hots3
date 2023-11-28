
#ifndef SETTINGS_H
#define SETTINGS_H

// debug

// #define DEBUG

// networking

#define PORT 6969
#define LISTEN 5

// game parameters

#define PLAYERS_REQUIRED 6

#define NUMBER_OF_BOT_PLAYERS 5
#define BOT_REACTION_TIME_MS 400 // 1e3 is 1 second
// b-line or random movement
#define BOT_WILLPOWER     1
#define BOT_SCHIZOPHRENIA 5
// human wave or encirclement
#define BOT_HUMAN_WAVE_NUMERATOR  2
#define BOT_HUMAN_WAVE_DENOMINTOR 4

// map parameters

#define MAP_Y 20
#define MAP_X 40

#define SPAWN_AREA_Y 8
#define SPAWN_AREA_X 16

#define PLAYERS_MAX 20 // includes minions

#define MINION_SPAWN_INTERVAL_MS 2700 // 1e3 is 1 second

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
