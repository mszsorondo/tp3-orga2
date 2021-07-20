/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Declaracion de funciones del scheduler.
*/

#ifndef __SCHED_H__
#define __SCHED_H__

#include "types.h"
#include "tss.h"

void sched_init();
uint16_t sched_next_task();
uint16_t sched_next_task_aux(uint8_t task);
uint8_t getCurrentTask();
void desalojar();
void modoDebugOnOff();
void modoDebug();
void setTaskActive(int i);
int getDebugging();
void endGame();
int getLemming();
void currentTaskMenosDos();

typedef struct task{
  bool isActive;      //Indica si la tarea está viva
  uint8_t taskIdx;    //Indice en la gdt de la tarea
}__attribute__((__packed__)) task_t;

typedef struct sched
{
  bool modoDebug;       //Indica si el modo debug esta activo 
  task_t tasks[10];     //Arreglo con la información de las tareas
  uint8_t currentTask;  //El indice en el arreglo de las tareas de la tarea actual
  uint8_t prevTask;     //El indice en el arreglo de las tareas de la tarea previa
  int debugging;        //Indica si esta la pantalla del debuggin prendida y el juego debe ser pausado.
}__attribute__((__packed__)) sched_t;





#endif //  __SCHED_H__
