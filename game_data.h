#ifndef GAME_DATA_H
#define GAME_DATA_H
#include <string>
#include <vector>
#include <cstdint>
#include "schneider_lang.h"

enum GameState { S_INTRO1, S_INTRO2, S_MENU, S_MAP, S_TAVERN };

struct Hero { 
    _87 name, race, className;
    _43 x, y, hp, hpMax, gold, xp, lvl;
    _43 str, dex, con, intui, wis, cha, cou;
    _43 slotHead, slotBody, slotMain, slotOff;
    _43 inv[20];
};

extern _43 state;
extern Hero p;
extern _43 world[50][50];
#endif