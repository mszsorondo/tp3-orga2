#include "syscall.h"

// Chequear que el resultado de una operacion sea esperado.
// En caso de que falle, se generara un breakpoint.
#define CHECK_EQ(A, B)                 \
  do {                                 \
    if ((A) != (B)) {                  \
      __asm volatile("xchg %bx, %bx"); \
    }                                  \
  }                                    \
  while (0)

// Hacer que el Lemming produzca una excepcion.
#define SEPPUKKU(...) do { __asm volatile("ud2"); } while(0)

void task(void) {
  for (int i = 0; i < 7; i++) {
    CHECK_EQ(syscall_move(RIGHT), SUCCESS);
  }
  CHECK_EQ(syscall_move(RIGHT), WALL);
  CHECK_EQ(syscall_move(DOWN), BORDER);
  CHECK_EQ(syscall_move(UP), SUCCESS);
  CHECK_EQ(syscall_move(RIGHT), LAKE);

  for (int i = 0; i < 20; i++) {
    CHECK_EQ(syscall_move(UP), SUCCESS);
  }
  // A partir de algun momento me tengo que topar con un lemming del equipo opuesto.
  for (int i = 0; i < 50; i++) {
    move_result_e res = syscall_move(RIGHT);
    if (res == LEMMING) {
      // La otra tarea se tiene que poder ejecutar y toparse conmigo
      syscall_move(RIGHT);
      syscall_move(RIGHT);
      syscall_move(RIGHT);
      syscall_move(RIGHT);

      SEPPUKKU();
    }
  }

  // Si llegamos hasta aca es porque no encontramos ningun lemming en el camino. Es un error.
  __asm volatile("xchg %bx, %bx");
}
