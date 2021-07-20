/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Declaracion de funciones del scheduler
*/

#ifndef __SCREEN_H__
#define __SCREEN_H__

/* Definicion de la pantalla */
#define VIDEO_FILS 50
#define VIDEO_COLS 80

#include "colors.h"
#include "defines.h"
#include "stdint.h"
#include "tss.h"

/* Estructura de para acceder a memoria de video */
typedef struct ca_s {
  uint8_t c;
  uint8_t a;
} ca;


void print(const char* text, uint32_t x, uint32_t y, uint16_t attr);
void print_dec(uint32_t numero, uint32_t size, uint32_t x, uint32_t y,
               uint16_t attr);
void print_hex(uint32_t numero, int32_t size, uint32_t x, uint32_t y,
               uint16_t attr);

void screen_draw_box(uint32_t fInit, uint32_t cInit, uint32_t fSize,
                     uint32_t cSize, uint8_t character, uint8_t attr);

void imprimir_mapa(void);

void print_scan_code(uint8_t code);

void escribirDebug(int code, tss_t* tss);

char* getErrorString(int ecode);

char* getRegisterString(int indice);

int getRegisterValue(int indice, tss_t* tssActual);

int getTeamOffSet(int indice);

void next_task_clock(int taskOffset);

void print_excepcion(int codigo);

bool hasErrorCode(int32_t code);

void setOldScreen();

void init_debug_interface(int code, int32_t ss, int32_t gs, int32_t fs, int32_t es, int32_t ds, int32_t cs, int32_t eflags, int32_t eip, int32_t err, 
int32_t* stacklvl3, int32_t edi, int32_t esi, int32_t ebp, int32_t esp, int32_t ebx, int32_t edx, 
int32_t ecx, int32_t eax);

uint16_t getTerrainAttributes1(char tarrain);
#endif //  __SCREEN_H__