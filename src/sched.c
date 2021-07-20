/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones del scheduler
*/

#include "sched.h"
#include "tss.h"
#include "screen.h"

sched_t scheduler;

//Inicializamos la estructura del scheduler
void sched_init(void){
  scheduler.modoDebug = true;
  scheduler.debugging = 0;
  scheduler.currentTask = 10;   //Indice en tasks de la tarea actual. 10 simboliza IDLE
  scheduler.prevTask = 10;      //Indice en tasks de la tarea anterior. 10 simboliza IDLE
  for(uint8_t i = 0; i<10; i++){
    //Como las tareas en la gdt estan apartir del índice 15 entonces el indice en tasks sumado 15 es el índice en la gdt
    scheduler.tasks[i].taskIdx = i+15;    
    scheduler.tasks[i].isActive = false;
  }
}

//Función que índica cual es la proxima tarea a ejecutar.
uint16_t sched_next_task(void) {

  uint16_t res;
    //La idea es buscar la próxima tarea a partir de saber cual es la tarea anterior. Pero estos genera problemas en la alternacia de los
    //equipos cuando la tarea anterior y la actual son del mismo equipo, lo cual sucede cuando un equipo se queda sin lemmings mientras
    //el todavía tiene lemmings activos. Esto es por que tenemos como precondición al buscar la tarea que sigue que la tarea anterior a la
    //actual sea del otro equipo.

    //Como la tarea actual es el índice en arreglo de tareas y los Amalin tiene tareas de índice par y los Betarote impares, cuando la
    //paridad de la tarea actual y la previa son iguales entonces cuando agarremos la tarea previa estamos buscando tareas del mismo
    //equipo cuando antes deberia chequear si tenemos alguna disponible del equipo contrario para cambiar. 

    //Para solucionar esto creamos este ternario. Si la tarea anterior y la previa tienen la misma paridad, entonces vamos a querer que
    //aux sea un índice de paridad contraria, entonces sumamos un numero impar a la tarea previa lo que soluciona el problema.

    //Le sumamos dos a la tarea previa por que nos interas buscar a partir del índice de la tarea siguiente a la previa.
    uint8_t aux = scheduler.prevTask%2 == scheduler.currentTask%2 ? scheduler.prevTask+3 : scheduler.prevTask+2;
    res = sched_next_task_aux(aux);

    //Si no encontramos una tarea para el otro equipo a la cual cambiar, entonces ahora si buscamos en el equipo actual.
    if(res == GDT_IDX_IDLE_TSS)
      res = sched_next_task_aux(scheduler.currentTask+2); 
  
  //Actualizamos la tarea previa y la actual.
  scheduler.prevTask = scheduler.currentTask;
  if(res == GDT_IDX_IDLE_TSS)
  scheduler.currentTask = 10;   //restamos 15 por que la auxiliar devuelve el índice en la gdt.
  else
  scheduler.currentTask = res -15;
  //Devolvemos el offset de la tarea en la gdt que es igual a su indice por 8 (osea shiteado 3)
  return res<<3;
}

//Esta función busca la proxima tarea activa a partir de id(indice en el arreglo de tareas) de la tarea pasado por parametro.
//Siempre devolvera el índice en la gdt de la tarea encontrada. Este se corresponde con el id de la misma sumado en 15.
//Si no encuentra nada devuelve el indice de la idle en la gdt
uint16_t sched_next_task_aux(uint8_t task){

  bool encontrado = false;
  uint8_t i = task;
    
    //Buscamos de la tarea pasada por parametro en adelante
    while(!encontrado && i < 10){
      encontrado = scheduler.tasks[i].isActive;
      
      if(encontrado)
        return scheduler.tasks[i].taskIdx;
    
      i+=2;
    }
    
    i=task%2;

    //Si no encontramos nada, buscamos de nuevo desde el principio del array de tareas
    while(!encontrado && i < 10){
      encontrado = scheduler.tasks[i].isActive;
      if(encontrado)
        return scheduler.tasks[i].taskIdx;
      i+=2;
    }
  //Si todavia no encontramos nada devolvemos el indice de la gdt de la idle
  return GDT_IDX_IDLE_TSS;
}

//Devolvemos la tarea que se está corriendo actualmente
uint8_t getCurrentTask(){
  return scheduler.currentTask;
}

//Ponemos la tarea con el indice pasado por parametro como activa
void setTaskActive(int indice){
  scheduler.tasks[indice].isActive = true;
}

//Ponemos la tarea con el indice pasado por parametro como inactiva y escribimos una x en su reloj 
void desalojar(int indice){
  char* dead = "x";
  scheduler.tasks[indice].isActive = false;
  //Misma matemática que usamos para escribir los relojes al iniciar el juego.
  print(dead, 21+indice*4, 42, 0x0F);
}

//Nos dice si el modo debug está activado o no
bool getModoDebug(){
  return scheduler.modoDebug;
}

//Nos indica si estamos con la pantalla del debug activa y el juego parado
int getDebugging(){
  return scheduler.debugging;
}

//Ponemos el indicador del debugging en 1
void setDebugging(){
  scheduler.debugging = 1;
}

//Aprovechamos la lógica del debuggin y ponemos la variable ne 2 indicando que el juego esta finalizado
//De esta manera congelamos el juego pero sin prender la pantalla del debugging
void endGame(){
  scheduler.debugging = 2;
}

//Esta función se encarga de prender el modo debug y apagar el debuggin cuando corresponda
void modoDebugOnOff(){
  //Si el modo debug está prendido entonces revisamos si tenemos que apagar la pantalla del debugger
    if(scheduler.modoDebug){
      //Si la pantalla del debugger está activa, restauramos la pantalla del juego y apagamos el debugging
      if(scheduler.debugging == 1){
        scheduler.debugging = 0;
        setOldScreen();
      }
    }
    //Si no prendemos el modo debugg
    else {
        scheduler.modoDebug = true;
    } 
}

void currentTaskMenosDos(){
  scheduler.currentTask -= 2;
}