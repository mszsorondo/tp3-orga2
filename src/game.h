/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Declaracion de las rutinas asociadas al juego.
*/

#ifndef __GAME_H__
#define __GAME_H__
#include "types.h"

bool game_tick(void);
void game_init(void);
void lemmings_init(void);


typedef enum direction_e {
  DIR_UP = 0,
  DIR_RIGHT = 1,
  DIR_DOWN = 2,
  DIR_LEFT = 3
} direction_e;

typedef enum move_result_e {
  MOVE_RESULT_SUCCESS = 0,
  MOVE_RESULT_WALL = 1,
  MOVE_RESULT_LAKE = 2,
  MOVE_RESULT_BORDER = 3,
  MOVE_RESULT_LEMMING = 4
} move_result_e;

//Estructura que contiene los datos de un lemmings, vida, edad, posición, su equipo y sobre que tipo de terreno está parado.
typedef struct str_lemming{
  uint8_t health;
  uint16_t age;
  uint8_t position_x;
  uint8_t position_y;
  char terrain;
  char team;
} lemming_t;

move_result_e movLemming(direction_e dir);
uint16_t getTerrainAttributes(char ca);
void boom();
void bridge(direction_e dir);
void lemmingCreate(int equipo);
int alive(int equipo);
void killLemming(int lemming);
void checkWinCondition();
#endif //  __GAME_H__
