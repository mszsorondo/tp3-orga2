/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones de impresion por pantalla.
*/

#include "screen.h"
#include "mapa.h"
#include "tss.h"
#include "i386.h"

int taskClocks[10] = {0,0,0,0,0,0,0,0,0,0};
char *reloj[4] = {"|", "/", "-", "\\"};


//Cuando se detecta una excepción imprime en pantalla cual es.
void imprimir_excepcion(int codigo){
  //Como tienen distinta longitud las excepciones borro primero lo que hubiera escrito antes  
  print(".........................",0,49, C_BG_BLACK | C_FG_BLACK);
  print("EXCEPCION ", 0, 49, 0x0A);
  print_dec(codigo, 2, 10, 49, 0x0A);
  print(" DETECTADA", 12, 49, 0x0A);
}

void print(const char* text, uint32_t x, uint32_t y, uint16_t attr) {
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO; // magia
  int32_t i;
  for (i = 0; text[i] != 0; i++) {
    p[y][x].c = (uint8_t)text[i];
    p[y][x].a = (uint8_t)attr;
    x++;
    if (x == VIDEO_COLS) {
      x = 0;
      y++;
    }
  }
}

void print_dec(uint32_t numero, uint32_t size, uint32_t x, uint32_t y,
               uint16_t attr) {
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO; // magia
  uint32_t i;
  uint8_t letras[16] = "0123456789";

  for (i = 0; i < size; i++) {
    uint32_t resto = numero % 10;
    numero = numero / 10;
    p[y][x + size - i - 1].c = letras[resto];
    p[y][x + size - i - 1].a = attr;
  }
}

void print_hex(uint32_t numero, int32_t size, uint32_t x, uint32_t y,
               uint16_t attr) {
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO; // magia
  int32_t i;
  uint8_t hexa[8];
  uint8_t letras[16] = "0123456789ABCDEF";
  hexa[0] = letras[(numero & 0x0000000F) >> 0];
  hexa[1] = letras[(numero & 0x000000F0) >> 4];
  hexa[2] = letras[(numero & 0x00000F00) >> 8];
  hexa[3] = letras[(numero & 0x0000F000) >> 12];
  hexa[4] = letras[(numero & 0x000F0000) >> 16];
  hexa[5] = letras[(numero & 0x00F00000) >> 20];
  hexa[6] = letras[(numero & 0x0F000000) >> 24];
  hexa[7] = letras[(numero & 0xF0000000) >> 28];
  for (i = 0; i < size; i++) {
    p[y][x + size - i - 1].c = hexa[i];
    p[y][x + size - i - 1].a = attr;
  }
}

//Imprime el proximo reloj de la tarea cuyo id es el pasado por parametro.
void next_task_clock(int taskOffset){
  int base = 0;
  int tarea = (taskOffset>>3) - 15;
  if(tarea % 2 == 0){
    base = 21;
  }
  else{
    base = 41;
  }

  if(taskClocks[tarea] % 4 == 0)
    print("|", base+getTeamOffSet(tarea)*4, 42, C_BG_BLACK | C_FG_WHITE);
  else if(taskClocks[tarea] % 4  == 1)
    print("/", base+getTeamOffSet(tarea)*4, 42, C_BG_BLACK | C_FG_WHITE);
  else if(taskClocks[tarea] % 4  == 2)
    print("-", base+getTeamOffSet(tarea)*4, 42, C_BG_BLACK | C_FG_WHITE);
  else if(taskClocks[tarea] % 4  == 3)
    print("\\", base+getTeamOffSet(tarea)*4, 42, C_BG_BLACK | C_FG_WHITE);

  taskClocks[tarea]++;
}

//Nos devuelve el offset de la ubicación del reloj en pantalla para la tarea pasada por parametro
int getTeamOffSet(int indice){
  int res = 0;
  switch(indice){
    case 0:
      res = 0;
      break;
    case 1:
      res = 0;
      break;
    case 2:
      res = 1;
      break;
    case 3:
      res = 1;
      break;
    case 4:
      res = 2;
      break;
    case 5:
      res = 2;
      break;
    case 6:
      res = 3;
      break;
    case 7:
      res = 3;
      break;
    case 8:
      res = 4;
      break;
    case 9:
      res = 4;
      break;
  }
  return res;
}

void screen_draw_box(uint32_t fInit, uint32_t cInit, uint32_t fSize,
                     uint32_t cSize, uint8_t character, uint8_t attr) {
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;
  uint32_t f;
  uint32_t c;
  for (f = fInit; f < fInit + fSize; f++) {
    for (c = cInit; c < cInit + cSize; c++) {
      p[f][c].c = character;
      p[f][c].a = attr;
    }
  }
}

void printScanCode(uint8_t code){
    if(code <= 0x0B)
      print_hex(code, 2, 78, 0, C_BG_BLACK | C_FG_LIGHT_GREEN);
}

//Imprime el mapa del juego en la pantalla.
void imprimir_mapa(){
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;
  uint8_t attr = 0x00; 

  for(int i = 0; i < VIDEO_FILS-10; i++){
    for(int j = 0; j < VIDEO_COLS; j++){
      if(mapa_original[i][j] == 'P'){
        attr = 0x46;
      }
      if(mapa_original[i][j]== 'L'){
        attr = 0x11;
      }
      if(mapa_original[i][j]== '.'){
        attr = 0x22;
      }

      p[i][j].a = attr;
      p[i][j].c = mapa_original[i][j];
      }
    }
}



//Esta estructura no servirá para hacer un debugg
ca(*backup)[VIDEO_COLS]=(ca(*)[VIDEO_COLS])VIDEO;
char map[40][80];

//Backupeamos la pantalla guardandola en map.
void backupPantalla(){
  ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;
  for(int i = 0; i < VIDEO_FILS-10; i++){
        for(int j = 0; j < VIDEO_COLS; j++){
          map[i][j] = p[i][j].c;
        }
  }
}

//Escribimos la interfaz del debuuger del juego.
void init_debug_interface(int code, int32_t ss, int32_t gs, int32_t fs, int32_t es, int32_t ds, int32_t cs, int32_t eflags, int32_t eip, int32_t err, 
int32_t* stacklvl3, int32_t edi, int32_t esi, int32_t ebp, int32_t esp, int32_t ebx, int32_t edx, 
int32_t ecx, int32_t eax){     

    //Hacemos un backup de la pantalla
    backupPantalla();

    //Dibujamos al area negra que contiene la info del debugger
    screen_draw_box(0,20,40,41,(uint8_t)' ', C_BG_BLACK | C_FG_BLACK);
    
    //Obtenemos el string asociado al tipo de error ocurrido y lo imprimos
    char* errorString = getErrorString(code);
    print(errorString, 21, 1, C_BG_BLACK | C_FG_LIGHT_GREEN);

    //Esto es para poder hacer un for loop y no escribir 14 lineas
    int32_t aux[15] = {eax, ebx, ecx, edx, esi, edi, ebp, esp, eip, cs, ds, es, fs, gs, ss};

    //Escribimos los registros y sus valores en pantalla
    for(int i = 0; i < 14; i++){
      print(getRegisterString(i), 22, 5+2*i, C_BG_BLACK | C_FG_WHITE);
      print_hex(aux[i], 8, 26, 5+2*i, C_BG_BLACK | C_FG_LIGHT_GREEN);
    }
    //Escribimos los eflags
    print("eflags", 22, 38, C_BG_BLACK | C_FG_WHITE);
    print_hex(eflags, 8, 29, 38, C_BG_BLACK|C_FG_LIGHT_GREEN);

    //Escrbimos los cr 
    print("cr0", 48, 6, C_BG_BLACK | C_FG_WHITE);
    print_hex(rcr0(),8,52,6,C_BG_BLACK|C_FG_LIGHT_GREEN);
    print("cr2", 48, 8, C_BG_BLACK | C_FG_WHITE);
    print_hex(rcr2(),8,52,8,C_BG_BLACK|C_FG_LIGHT_GREEN);
    print("cr3", 48, 10, C_BG_BLACK | C_FG_WHITE);
    print_hex(rcr3(),8,52,10,C_BG_BLACK|C_FG_LIGHT_GREEN);
    print("cr4", 48, 12, C_BG_BLACK | C_FG_WHITE);
    print_hex(rcr0(),8,52,12,C_BG_BLACK|C_FG_LIGHT_GREEN);
    //Si recibimos como codigo de error 0xDEAD entonces no hay código de error y escribimos guiones.
    print("err", 48, 14, C_BG_BLACK | C_FG_WHITE);
    if(err == 0xDEAD)
      print("--------", 52, 14, C_BG_BLACK|C_FG_LIGHT_GREEN);
    else 
    //Si no escribimos el código de error
      print_hex(err,8,52,14,C_BG_BLACK|C_FG_LIGHT_GREEN);

    //Escribimos los valores que levantamos del stack
    int stackelements = (0x08003000-(int)stacklvl3)/4;
    print("stack", 42 ,18, C_BG_BLACK | C_FG_WHITE);
    print("--------", 42, 19, C_BG_BLACK|C_FG_LIGHT_GREEN);
    print("--------", 42, 20, C_BG_BLACK|C_FG_LIGHT_GREEN);
    print("--------", 42, 21, C_BG_BLACK|C_FG_LIGHT_GREEN);
    for(int i = 0; i < 3 && i < stackelements; i++)
    print_hex(stacklvl3[i], 8, 42, 19+i, C_BG_BLACK|C_FG_LIGHT_GREEN);
   

 }

//Nos devuelve el sting que indica el tipo de error en base a su codigo.
char* getErrorString(int ecode){
  char* res;
  switch(ecode){
    case 0:
      res = "Divide Error #DE [0]";
      break;
    case 1:
      res = "Debug Exception #DB [1]";
      break;
    case 2:
      res = "NMI Interrupt - [2]";
      break;
    case 3:
      res = "Breakpoint #BP [3]";
      break;
    case 4:
      res = "Overflow #OF [4]";
      break;
    case 5:
      res = "Bound Range Exceeded #BR [5]";
      break;
    case 6:
      res = "Invalid Opcode #UD [6]";
      break;
    case 7:
      res = "Device not available #NM [7]";
      break;
    case 8:
      res = "Double Fault #DF [8]";
      break;
    case 9:
      res = "Coprocessor Segment Overrun - [9]";
      break;
    case 10:
      res = "Invalid TSS #TS [10]";
      break;
    case 11:
      res = "Segment not present #NP [11]";
      break;
    case 12:
      res = "Stack Segment Fault #SS [12]";
      break;
    case 13:
      res = "General Protection Fault #GP [13]";
      break;
    case 14:
      res = "Page Fault #PF [14]";
      break;
    case 15:
      res = "Intel Reserved - [15]";
      break;
    case 16:
      res = "Math Fault #MF [16]";
      break;
    case 17:
      res = "Alignment Check #AC [17]";
      break;
    case 18:
      res = "Machine Check #MC [18]";
      break;
    case 19:
      res = "SIMD Floating Point Error #XM [19]";
      break;
    default:
      res = "Not present exception";
      break;
  }

  return res;
}

//Nos dice si para el la excepción de código pasado por parametro tenemos error code.
bool hasErrorCode(int32_t code){

  bool res = false;
  if(code == 8 || code == 0xA || code == 0xB || code == 0xC || code == 0xD || code == 0xE || code == 0x11 || code == 0x1E){
    res = true;
  }  
  return res;
}

//Recuperamos la pantalla con los datos guardado en map. Esto se usa la apagar la pantalla.
void setOldScreen(){
    ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;
    for(int i = 0; i < VIDEO_FILS-10; i++){
        for(int j = 0; j < VIDEO_COLS; j++){
          p[i][j].c = map[i][j];
          p[i][j].a = getTerrainAttributes1(map[i][j]);
        }
  }
}

//Nos devuelve los atributos para el tipo de terrane pasado por parametro.
uint16_t getTerrainAttributes1(char terrain){
    uint16_t res = 0;
    switch(terrain)
    {
        case 'A':
          res = C_BG_RED | C_FG_WHITE;
          break;
        case 'B':
          res = C_BG_BLUE | C_FG_WHITE;
          break;
        case 'P':
          res = 0x46;
          break;
        case 'L':
          res = 0x11;
          break;
        case '+':
            res = C_BG_BLUE | C_FG_WHITE;
            break;
        case '.':
            res = C_BG_GREEN | C_FG_GREEN;
            break;
        case 'x':
            res = C_BG_RED | C_FG_GREEN;
            break;
    }
    return res;
}

//Nos devuelve el string asociado a un registros dependiendo deun valor númerico.
char* getRegisterString(int index){
  char* res = "";
  switch(index){
    case 0:
      res =  "eax";
      break;
    case 1:
      res = "ebx";
      break;
    case 2:
      res = "ecx";
      break;
    case 3:
      res = "edx";
      break;
    case 4:
      res =  "esi";
      break;
    case 5:
      res =  "edi";
      break;
    case 6:
      res = "ebp";
      break;
    case 7:
      res =  "esp";
      break;
    case 8:
      res = "eip";
      break;
    case 9:
      res = " cs";
      break;
    case 10:
      res = " ds";
      break;
    case 11:
      res = " es";
      break;
    case 12:
      res = " fs";
      break;
    case 13:
      res = " gs";
      break;
    case 14:
      res = " ss";
      break;
  }
  return res;
}
