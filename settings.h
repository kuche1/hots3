
#ifndef SETTINGS_H
#define SETTINGS_H

// networking

#define PORT 32793
#define LISTEN 5
#define SEND_MAX_ATTEMPTS 5 // this does not take into account the size of the data

// anti hacking

#define ACTION_INTERVAL_MS 50

// game parameters

#define PLAYERS_MAX 50 // includes minions and towers, more like entities_max

#define PLAYERS_REQUIRED 8 // more like heroes_required

#define RESPAWN_TIME_MS 20e3
#define MINION_SPAWN_INTERVAL_MS 2100

#define HEALTH_STATES 8 // clarity vs bandwidth (and style)

// levels

#define LEVEL_ON_SPAWN 7
#define XP_FOR_LEVEL_UP 10
#define KILL_REWARD_XP 1

#define MINIONS_AND_TOWERS_CAN_LEVEL_UP 1
#define TOWERS_RESTORE_HP_ON_LEVEL_UP 0

#define LEVEL_UP_HEALTH_RESTORED_NUMERATOR    7
#define LEVEL_UP_HEALTH_RESTORED_DENOMINATOR 32

#define LEVELS_LOST_ON_DEATH 0
#define XP_IS_LOST_ON_DEATH 1 // yes or no

// bot parameters

// reactions
#define BOT_REACTION_TIME_MS 300
#define MINION_REACTION_TIME_MS 700
#define TOWER_REACTION_TIME_MS 1400
// b-line or random movement
#define BOT_WILLPOWER     4
#define BOT_SCHIZOPHRENIA 5
#define MINION_WILLPOWER      9
#define MINION_SCHIZOPHRENIA 10
#define TOWER_WILLPOWER     1
#define TOWER_SCHIZOPHRENIA 1
// human wave or encirclement
#define BOT_HUMAN_WAVE_NUMERATOR  2
#define BOT_HUMAN_WAVE_DENOMINTOR 4
#define MINION_HUMAN_WAVE_NUMERATOR  5
#define MINION_HUMAN_WAVE_DENOMINTOR 6
#define TOWER_HUMAN_WAVE_NUMERATOR  1
#define TOWER_HUMAN_WAVE_DENOMINTOR 1

// map parameters

#define MAP_Y 24
#define MAP_X (MAP_Y * 2)

#define SPAWN_AREA_Y 8
#define SPAWN_AREA_X (SPAWN_AREA_Y * 2)
#define MINION_SPAWN_AREA_Y 3
#define MINION_SPAWN_AREA_X (MINION_SPAWN_AREA_Y * 2)

#define TOWERS_PER_TEAM 7
#define TOWER_SPAWN_Y 3
#define TOWER_SPAWN_X (TOWER_SPAWN_Y * 2)

// UI

#define UI_LEVEL_Y (MAP_Y+1)
#define UI_XP_Y (UI_LEVEL_Y+1)
#define UI_HELP_Y (UI_XP_Y+1)

// keybinds

// keybind movement
#define KEY_MOVE_LEFT 'a'
#define KEY_MOVE_RIGHT 'd'
#define KEY_MOVE_UP 'w'
#define KEY_MOVE_DOWN 's'
// basic attack
#define KEY_BASIC_ATTACK_1 'j'
#define KEY_BASIC_ATTACK_2 'h'
// heal ability
#define KEY_HEAL_ABILITY 'k'

#endif
