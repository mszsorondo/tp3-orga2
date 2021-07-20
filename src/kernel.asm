; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
; ==============================================================================

%include "print.mac"

global start

extern GDT_DESC
extern IDT_DESC
extern idt_init
extern pic_enable
extern pic_reset
extern mmu_init
extern mmu_init_kernel_dir
extern mmu_map_page
extern mmu_unmap_page
extern mmu_init_task_dir
extern print
extern tss_init
extern tss_create
extern sched_init
extern game_init

%define CS_LVL_0 0x0040
%define DS_LVL_0 0x0050
%define ESP_INIT 0x25000
%define VM_LVL_0 0x0060
%define TSS_INITIAL 0x0068
%define TSS_IDLE 0x0070
%define LEMMING_1 0x0078

BITS 16
;; Saltear seccion de datos

jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

Amalin db 'Amalin'
Amalin_len equ $ - Amalin
Betarote db 'Betarote'
Betarote_len equ $ - Betarote
Lemmings db '00  01  02  03  04'
Lemmings_len equ $ - Lemmings
Dead db 'x   x   x   x   x   x   x   x   x   x'
Dead_len equ $ - Dead

vacio db ''
;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:
    ; Deshabilitar interrupciones
    cli

    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    ; Imprimir mensaje de bienvenida
    print_text_rm start_rm_msg, start_rm_len, 0x07, 0, 0
    
    
    
    ; Habilitar A20
    call A20_disable
    call A20_check
    call A20_enable
    call A20_check

    
    ; Cargar la GDT
    lgdt [GDT_DESC]
    
    ; Setear el bit PE del registro CR0
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    ; Saltar a modo protegido
    jmp CS_LVL_0:modo_prot 
    
BITS 32    
modo_prot:
    ; Establecer selectores de segmentos
    
    mov ax, DS_LVL_0
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Establecer la base de la pila
    mov esp, ESP_INIT
    mov ebp, esp
    
    ; Imprimir mensaje de bienvenida
    print_text_pm start_pm_msg, start_pm_len, 0x07, 0, 0

    ; Inicializar pantalla
    mov ecx, 0
    mov ax, VM_LVL_0
    mov es, ax
    .ciclo_pantalla:
        cmp ecx, 0x1900
        je .equipos
        
        mov word [es:ecx], 0x1100
        add ecx, 2
        jmp .ciclo_pantalla
    
    ;la 10 columna desde la izquierda, pero ademas la 3 fila desde arriba
    .equipos:; ec+6400
    add ecx, 0x1F2 ;498 bytes
    mov edx, 0
    mov ebx, 4

    ; Inicializar el manejador de memoria
    call mmu_init
 
    ; Inicializar el directorio de paginas
    call mmu_init_kernel_dir

    ; Cargar directorio de paginas
    mov cr3, eax

    ; Habilitar paginacion
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ;mapear paginas


    ; Inicializar tss
    ; Inicializar tss de la tarea Idle
    call tss_init

    ; Inicializar el scheduler
    call sched_init
    
    ;Inicializamos las estructuras del juego
    call game_init
    
    ; Inicializar la IDT
    call idt_init
    
    ; Cargar IDT
    lidt [IDT_DESC]

    ; Configurar controlador de interrupciones
    call pic_reset
    call pic_enable

    ;Habilitar Interrupciones
    sti

    ; Cargar tarea inicial
    mov ax, TSS_INITIAL
    ltr ax
    xchg bx, bx 
    
    ;Saltar a la primera tarea: Idle
    jmp TSS_IDLE:0
    

    ; Ciclar infinitamente (por si algo sale mal...)
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    .inf:
    jmp .inf

;; -------------------------------------------------------------------------- ;;

%include "a20.asm"