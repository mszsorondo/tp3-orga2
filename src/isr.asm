; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================
;
; Definicion de rutinas de atencion de interrupciones

%define TASK_CODE_VIRTUAL 0x08000000
%define TASK_STACK_BASE   0x08003000

%include "print.mac"
extern imprimir_excepcion
extern printScanCode
extern movLemming
extern boom
extern game_tick
BITS 32

sched_task_offset:     dd 0xFFFFFFFF
sched_task_selector:   dw 0xFFFF

offset:     dd 0
selector:   dw 0
result:     dd 0

;; PIC
extern pic_finish1

;; Sched
extern sched_next_task
extern desalojar
extern handlerModoDebug
extern modoDebug
extern modoDebugOnOff
extern next_task_clock
extern imprimir_excepcion
extern boom
extern bridge
extern init_debug_interface
extern getModoDebug
extern hasErrorCode
extern killLemming
extern getDebugging
extern setDebugging
extern handlerPageFault

extern getCurrentTask
;;
;; Definición de MACROS
;; -------------------------------------------------------------------------- ;;
%macro ISR 1
global _isr%1

_isr%1:
   pushad

   xchg bx, bx
   mov eax, %1
   push eax
   call imprimir_excepcion
   add esp, 4
   ;^^ imprimimos la excepción en pantalla.

   call getCurrentTask
   ;^^ obtenemos el índice de la tarea que genero la falta. Este se corresponde con el lemming asociado a dicha tarea.
   

   push eax
   call killLemming
   add esp, 4
   ;^^ matamos al lemming que produjo la excepción.

   call getModoDebug
   ;^^ chequeamos si el modo debug está prendido. De ser así comenzaremos a recuperar los datos para imprimir la pantalla del debugger.
   cmp eax, 1
   jne .sigue

   call setDebugging
   ;^^ seteamos la variable debuggin del scheduler en true para indicar que estamos debuggeando, es decir la pantalla del debugger está activa

   mov eax, %1
   push eax
   call hasErrorCode
   add esp, 4
   mov [result], eax
   cmp eax, 1
   je .hayErrorCode
   jne .noHayErrorCode
   ;^^ Chequeamos si al excepción genera error code o no. Si genera errocode tendremos que usar otros offsets para recuperar valores en
   ; la pila de lvl 0, además de tener que escribir el error code y sacarlo de la pila antes del iret.

.hayErrorCode:
   mov eax, [esp+8*4] ; error code
   mov ebx, [esp+9*4] ; eip
   mov ecx, [esp+11*4] ; eflags
   mov edx, [esp+12*4] ; esp lvl 3
;^^ guardamos todo lo info que corresponde con la tarea en el nivel 3

   push edx
   push eax
   push ebx
   push ecx 
;^^ pasamos como parametros el eip, eflags y el esp.
;^^además, pasamos como parametro el puntero a la pila, esto se debe a que puede que no se haya pusheado nada en esa pila y accederla directamente generaría un page fault.
;en el C encargado del print se va a verificar si estamos en esta situación o no, y en caso de no estarlo se van a resolver la cantidad de valores a escribir.

   jmp .sigueDebug

.noHayErrorCode:
   ;Como no hay error code pasamos DEAD, y el inicializador de la interfaz de debug escribira guiones en el lugar del err.
   mov eax, 0xDEAD  
   mov ebx, [esp+8*4]   ;eip
   mov ecx, [esp+10*4]  ;eflags
   mov edx, [esp+11*4]  ;esp lvl 3


   ;^^ Los tres pimeros datos que estan pusheados en la pila de nivel 3 y lo pusheamos para pasarlo por parametro
   push edx
   push eax
   push ebx
   push ecx
;^^además, pasamos como parametro el puntero a la pila, esto se debe a que puede que no se haya pusheado nada en esa pila y accederla directamente generaría un page fault.
;en el C encargado del print se va a verificar si estamos en esta situación o no, y en caso de no estarlo se van a resolver la cantidad de valores a escribir.
.sigueDebug:
   push cs 
   push ds 
   push es 
   push fs 
   push gs
   push ss 
;^^le pasamos los codigos de segmento al inicializador de la interfaz de debug

   mov eax, %1
   push eax
;^^le pasamos el codigo de la excepcion al inicializador de la interfaz de debug

   call init_debug_interface
   add esp, 13*4
   ;^^ una vez pasado todos lo datos llamamos a la función que inicializa toda la interfaz del debugging.


.sigue:
   mov [selector], word 0x70
   jmp far [offset]
;;^^ saltamos a la tarea idle
   .fin:
   popad

   push eax             ;como usamos eax para guardar result lo pusheamos para preservarlo.
   mov eax, [result]
   cmp eax, 1
   jne .final
   pop eax
   add esp, 4
   jmp .irett
   ;;^^Chequeamos result, un lugar en memoria que tiene guardado el resultado de la funcion hasErrorCode, de manera que podamos ver si
   ;debemos quitar el error code de la pila no.
   
.final:
   pop eax
   
.irett:
   iret

%endmacro

;; Rutina de atención de las EXCEPCIONES
;; -------------------------------------------------------------------------- ;;
ISR 0
ISR 1
ISR 2
ISR 3
ISR 4
ISR 5
ISR 6
ISR 7
ISR 8
ISR 9
ISR 10
ISR 11
ISR 12
ISR 13
ISR 15
ISR 16
ISR 17
ISR 18
ISR 19
ISR 20

;Rutina de atención del page fault
global _isr14
_isr14:
   pushad

   mov eax, 14
   push eax
   call imprimir_excepcion
   add esp, 4
   ;^^ imprimimos la excepción en pantalla.
   
   call handlerPageFault
   cmp eax, 1
   je .fin
   ;^^ llamamos a la función que se encarga de manejar el tema del mapeo bajo demanda. Esta función además va a indicar si el mapeo
   ;fue exitoso o no. De no serlo desalojamos el lemming t saltamos a la tarea idle.

   call getCurrentTask
   ;^^ obtenemos el índice de la tarea que genero la falta. Este se corresponde con el lemming asociado a dicha tarea.
   
   push eax
   call killLemming
   add esp, 4
   ;^^ Matamos el lemming

   mov [selector], word 0x70
   jmp far [offset]


.fin:
   popad
   add esp, 4
   iret

;; Rutina de atención del RELOJ
global _isr32
_isr32:
   pushad
   call pic_finish1
   
   call getDebugging
   cmp eax, 1
   jge .fin
   ;^^ Nos fijamos si estamos debuggeando en este momento. Si es así entonces no hacemos nada.

   call next_clock

   call game_tick
   ;^^ Ejecutamos toda la lógica del juego que se produce al caer un tick de reloj.
   cmp eax, 1
   ;^^ game tick devuelve 1 si: se cumplen las condiciones tal que hay una tarea para matar, y esa tarea es exactamente la current
   jne .next_task
   ;^^ si no es así, continuamos la lógica de la interrupción normalmente

   mov [esp + 8*4], dword TASK_CODE_VIRTUAL
   mov [esp + 11*4], dword TASK_STACK_BASE
   ;^^ antes de hacer el task switch, alteramos los valores de la pila de tal manera que el snap que se guarde en la tss tenga los errores correctos.
   ;en particular, se pisa la dirección de retorno con la dirección que se corresponde con el inicio del código de la tarea
   ;también se acomoda el stack.

   .next_task:
   call sched_next_task
   ;^^ Obtenemos la proxima tarea a ser ejecutada.

   str cx
   cmp ax, cx
   je .fin
   ;^^ Nos fijamos que la tarea obtenida no sea la misma que ya estamos ejecutando.

   mov [selector], ax
   push eax
   call next_task_clock
   ;^^ Cambiamos el reloj de la tarea antes de cambiar
   add esp, 4
   jmp far [offset]
   ;^^ task switch
   

   
   .fin:
   popad
   iret
;; -------------------------------------------------------------------------- ;;

extern handlerModoDebug
;; Rutina de atención del TECLADO
global _isr33
_isr33:
   pushad
   in al, 0x60
   
   ;si el codigo es 15 && !mododebug => prendemos modo debug

   ;else el codigo es 15 && mododebug && debugging => cerramos información y continuamos
   cmp eax, 0x15
   jne .fin

   call modoDebugOnOff
   ;^^ esta función prende el mododebug si no estaba prendido y si está prendido y estamos debuggeando, apaga la pantalla del debugger y
   ;reanuda la ejecución del juego.
   .fin:
   call pic_finish1
   popad
iret
;; -------------------------------------------------------------------------- ;;
;; Rutinas de atención de las SYSCALLS
global _isr88
_isr88:
   pushad

   push eax
   call movLemming
   ;^^ Llamamos a la función que se encarga de realizar el movimiento del lemming
   add esp, 4
   mov [esp + 28], eax
   ;^^ Movemos a el offset 7 del esp el resultado de la función movLemming. Allí está guardado el valor de eax del pushad.
   ;De esta forma cuando hagamos popad, eax se "restaura" con el valor del resultado del movimiento.

   mov [selector], word 0x70
   jmp far [offset]
   ;^^Saltamos a la tarea idle.
   
   popad
   iret


global _isr98
_isr98:
   pushad

   call boom
   ;^^Llamamos a la función que se encargará de realizar la explosión. 

   mov [selector], word 0x70
   jmp far [offset]
   ;^^Saltamos a la tarea idle.

   popad
   iret

global _isr108
_isr108:
   pushad

   push eax
   call bridge
   add esp, 4
   ;^^ Llamamos a la función que se encargará de hacer el puente.

   mov [selector], word 0x70
   jmp far [offset]
   ;^^Saltamos a la tarea idle.

   popad
   iret
;; -------------------------------------------------------------------------- ;;
;; Funciones Auxiliares
;; -------------------------------------------------------------------------- ;;
isrNumber:           dd 0x00000000
isrClock:            db '|/-\'
next_clock:
        pushad
        inc DWORD [isrNumber]
        mov ebx, [isrNumber]
        cmp ebx, 0x4
        jl .ok
                mov DWORD [isrNumber], 0x0
                mov ebx, 0
        .ok:
                add ebx, isrClock
                print_text_pm ebx, 1, 0x0f, 49, 79
                popad
        ret