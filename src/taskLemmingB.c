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
    CHECK_EQ(syscall_move(LEFT), SUCCESS);
  }
  CHECK_EQ(syscall_move(LEFT), WALL);
  CHECK_EQ(syscall_move(UP), BORDER);
  CHECK_EQ(syscall_move(DOWN), SUCCESS);
  CHECK_EQ(syscall_move(LEFT), LAKE);

  for (int i = 0; i < 17; i++) {
    CHECK_EQ(syscall_move(DOWN), SUCCESS);
  }

  // A partir de algun momento me tengo que topar con un lemming del equipo opuesto.
  for (int i = 0; i < 50; i++) {
    move_result_e res = syscall_move(LEFT);
    if (res == LEMMING) {
      // La otra tarea se tiene que poder ejecutar y toparse conmigo
      syscall_move(LEFT);
      syscall_move(LEFT);
      syscall_move(LEFT);
      syscall_move(LEFT);

      SEPPUKKU();
    }
  }

  // Si llegamos hasta aca es porque no encontramos ningun lemming en el camino. Es un error.
  __asm volatile("xchg %bx, %bx");
}
