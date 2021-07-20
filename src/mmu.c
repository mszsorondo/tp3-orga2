/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones del manejador de memoria
*/

#include "mmu.h"
#include "i386.h"
#include "sched.h"

#include "kassert.h"

#define _MMU_PAGE_BIT_PRESENT 0
#define _MMU_PAGE_BIT_READ_WRITE 1
#define _MMU_PAGE_BIT_US 2

#define _MMU_PAGE_US (0 << _MMU_PAGE_BIT_US)
#define _MMU_PAGE_RW (1 << _MMU_PAGE_BIT_READ_WRITE)
#define _MMU_PAGE_PRESENT (1 << _MMU_PAGE_BIT_PRESENT)

#define INICIO_PAGINAS_LIBRES_KERNEL 0x00100000
#define INICIO_PAGINAs_LIBRES_USER 0x400000

/* 
================================================================================
                 INICIALIZACION DE LOS SISTEMAS DEL MMU
================================================================================
*/


unsigned int proxima_pagina_libre_kernel;
unsigned int proxima_pagina_libre_user;

void mmu_init(void) {
    proxima_pagina_libre_kernel = INICIO_PAGINAS_LIBRES_KERNEL - 0x1000;
    proxima_pagina_libre_user = INICIO_PAGINAs_LIBRES_USER - 0x1000;
}

/* 
================================================================================
                          FUNCIONES DEL MMU
================================================================================
*/

//Obtenemos la proxima página libre del área del kernel. Estos es aumentar el la variable de la proxima pagina libre del kernel en 4kib
paddr_t mmu_next_free_kernel_page(void) {
  proxima_pagina_libre_kernel = proxima_pagina_libre_kernel + 0x1000;
  return proxima_pagina_libre_kernel;
}

//Obtenemos la proxima página libre del área del usario. Estos es aumentar el la variable de la proxima pagina libre del usario en 4kib

paddr_t mmu_next_free_user_page(void) {
  proxima_pagina_libre_user = proxima_pagina_libre_user + 0x1000;
  return proxima_pagina_libre_user;
}

//Inicializamos el directorio que mapea el kernel con identity mapping
paddr_t mmu_init_kernel_dir(void){

  page_directory_entry* pd = (page_directory_entry*)KERNEL_PAGE_DIR;
  page_table_entry* pt_0 = (page_table_entry*)KERNEL_PAGE_TABLE_0;

  //Ponemos todos los valores en 0
  for(int i = 0; i < 1024; i++){
    pd[i] = (page_directory_entry){0};
    pt_0[i] = (page_table_entry){0};
  }
  //Seteamos los atributos para el page directory
  pd[0].attrs = _MMU_PAGE_US | _MMU_PAGE_RW | _MMU_PAGE_PRESENT;
  pd[0].page_table_base = (uint32_t)pt_0 >> 12;

  //Hacemos el identity mapping del area del kernel para la tabla correpondiente.
  for(int i = 0; i < 1024; i++){
    pt_0[i].attrs = _MMU_PAGE_US | _MMU_PAGE_RW | _MMU_PAGE_PRESENT;
    pt_0[i].physical_adress_base = i;
  }
  return (uint32_t)pd;
}

//Función de mapeo de página. Recibe como parametros la tabla de paginas en donde vams a mapear
//las direcciones virtual y física a mapear y los atributos del mapeo.
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {
  
  //casteamos el cr3 como un puntero a pde
  page_directory_entry* cr3Aux = (page_directory_entry*)cr3;

  //Descomponemos la dirección virtual para obtener los índice de la PDT y la PT donde vamos a mapear
  uint16_t directoryIdx = virt >> 22;
  uint32_t pageIdx = (virt >> 12) & 0x3FF;

  //Chequeamos si la entrada que nos piden en la PDT está presente
  if(!(cr3Aux[directoryIdx].attrs & 0x001)){

    //Como no está presente tenemos que crear una nueva PT y asociarla a dicha entrada
    page_table_entry* newPT = (page_table_entry*) mmu_next_free_kernel_page();

    //le llenas las 1024 entradas a la PT
    for(int i = 0; i<1024 ; i++)
            newPT[i] = (page_table_entry){0};
    
    //Ponemos la dirección de la base de la tabla en la entrada de la PDT que nos piden
      cr3Aux[directoryIdx].page_table_base =(uint32_t)newPT>>12;
  }

  //Hacemos un or entre los atributos de la entrada en la PDT y los atributos que nos pasan por parámetros
  cr3Aux[directoryIdx].attrs |= (attrs | 0x001);

  //Obtenemos el puntero a la PT donde debemos mapear
  uint32_t base_tabla = (uint32_t)cr3Aux[directoryIdx].page_table_base;
  page_table_entry* point_base_tabla = (page_table_entry*)(base_tabla << 12);

  //Mapeamos en el indice de la PT que nos piden la dirección física que nos pasan por parámetro.
   //Hacemos un or entre los atributos de la entrada en la PT y los atributos que nos pasan por parámetro
  point_base_tabla[pageIdx].physical_adress_base = phy>>12; 
  point_base_tabla[pageIdx].attrs |= (attrs | 0x001);

  //falta tbl flush
  tlbflush();
  return;

}  
    
//Función para desmapear páginas.
paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt) {
  
  page_directory_entry* cr3Aux = (page_directory_entry*) cr3;
  uint16_t directoryIdx = virt >> 22; // obtengo el offset en el directorio de paginas
  uint32_t pageIdx = (virt >> 12) & 0x3FF; // el offset en la tabla de paginas

  //Aquí guardaremos la dirección física a ser desmapeada.
  paddr_t fisica;

  //Obtenemos el puntero a la PT que corresponde
  uint32_t baseTabla = (uint32_t)cr3Aux[directoryIdx].page_table_base;
  page_table_entry* pBaseTabla = (page_table_entry*) (baseTabla << 12);
  //Ponemos el bit de presente en 0
  pBaseTabla[pageIdx].attrs = 0x000;

  //Guardamos y retornamos la dirección física que fue desmapeada.
  fisica = (paddr_t)(pBaseTabla->physical_adress_base <<12);
  return fisica;
}

//Función que crea PDT para tareas. Recibe como parámetros la dirección física donde comienza el área de la tarea.
paddr_t mmu_init_task_dir(paddr_t phy_start) {

  //Pedimos una página nueva que será donde hagamos el PDT.
  paddr_t task_dir = (paddr_t) mmu_next_free_kernel_page();

  //mapeamos el area del kernel con identity mapping
  for(uint32_t i = 0; i < 0x400000; i += PAGE_SIZE){
    mmu_map_page(task_dir, i, i, 2);
  }

  //Mapeamos el área que corresponde al código de la tarea al área virtual correspondiente para la misma
  mmu_map_page(task_dir, 0x08000000, phy_start, 4);
  mmu_map_page(task_dir, 0x08001000, (phy_start + 0x1000), 4);

  //Pedimos una nueva página del área de usario para mapear la pila de la tarea
  paddr_t task_stack = (paddr_t) mmu_next_free_user_page();
  
  mmu_map_page(task_dir, 0x08002000, task_stack, 7); 
  
  return task_dir;
}

/* 
================================================================================
                ASIGNACIÓN BAJO DEMANDA Y MEMORIA COMPARTIDA
================================================================================
*/


//Estos dos arreglos contendrán las direcciones físicas del área compartida que los equipos tienen mapeadas.
//La dirección virtual a la que están mapeadas estas direcciones se realciona con el índice en el arreglo de la siguiente forma
// virt_addr = 0x400000 + i*0x1000  -----> i = (virt_addr - 0x400000)/0x1000

paddr_t mappedA[4096] = {0};
paddr_t mappedB[4096] = {0};


//Función que mapea las página que los lemmings solicitan. Devuelve un booleano dependiendo de si el mapeo era válido o no.
//Para que sea válido debe estar pidiendo mapear un área dentro del rango asignado para la memoria compartida.
bool handlerPageFault(){
  
  bool res = false;
  //Guardamos el PDT y la direccion que generó el fallo
  uint32_t cr3 = rcr3();
  uint32_t cr2 = rcr2();
  //Obtenemos el índice que se corresponde la dirección que generó el fallo en el arreglo de direcciones mapeadas.
  uint32_t mappedIdx = (((cr2>>12)<<12) - 0x400000)/0x1000;
  paddr_t pyhs = 0;

  //Cheuqeamos que la dirección a mapear esté en el rango de direcciones válidas.
  if(cr2 >= 0x400000 && cr2 <= 0x13FFFFF){
    //Buscamos para el equipo correpondiente si está dirección ya fue mapeada por otro lemming.
    if(getCurrentTask()%2 == 0)
      pyhs = mappedA[mappedIdx];
    else
      pyhs = mappedB[mappedIdx];
    
    //Si el equipo había mapeado ya esta dirección entonces la mapeamos con la direccion fisica obtenida y devolvemos true
    if(pyhs != 0){
      mmu_map_page(cr3, cr2, pyhs, 0x7);
      res = true;
    }
    //Si no la mapeamos por primera vez ya agregamos la dirección física a la cual mapeamos en el arreglo del equipo que corresponda
    else{
      pyhs = mmu_next_free_user_page();
      mmu_map_page(cr3, cr2, pyhs, 0x7);
      if(getCurrentTask()%2 == 0)
        mappedA[mappedIdx] = pyhs;
      else
        mappedB[mappedIdx] = pyhs;
      res = true;
    }
  }
  return res;
}