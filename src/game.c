/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
*/

#include "game.h"
#include "mapa.h"
#include "types.h"
#include "screen.h"
#include "sched.h"
#include "i386.h"
#include "sched.h"
int len(int number);


/* 
================================================================================
                 INICIALIZACION DE LOS SISTEMAS DEL JUEGO
================================================================================
*/

//Contador para los ticks de reloj que indicaran cuando deben crearse o matarse lemmings
uint32_t spawn_ticks = 0;
uint32_t kill_ticks = 0;

//Contadores de lemmings creados para cada equipo
uint16_t spawnedA = 0;
uint16_t spawnedB = 0;

//Array de tipo lemming_t que contendra la información de los lemmings. Los lemmings con indice par serán Amalin y los impares Betarote
lemming_t lemmings[10];


//Inicializamos todo lo necesario para correr la lógica del juego
void game_init(void) {


    //Inicializamos el mapa
    imprimir_mapa();
    
    //Escribimos el nombre de los equipos
    char* Betarote = "Betarote";
    char* Amalin = "Amalin";

    print(Amalin, 9, 41, 0x0C);
    print(Betarote, 62, 41, 0x09);

    //Escribimos lo interfaz de los relojes de los lemmings. Como están todos muertos su reloj marca X
    char* dead = "x";

    for(uint32_t i=0; i<10; i++){
        //Los primeros 5 serán los relojes de Amalin y luego los de Betarote
        //Hacemos una matematica simple para la ubicación en pantalla
        //Los relojes comienzan en 21 y cada reloj "está longitud de reloj" lugares a la derecha de esta base.
        if(i<5){
        print_dec(i, 2, 21+i*4, 41, C_BG_BLACK | C_FG_LIGHT_RED);
        print(dead, 21+i*4, 42, C_BG_BLACK | C_FG_WHITE);
        }else{
        print_dec(i%5, 2, 21+i*4, 41, C_BG_BLACK | C_FG_LIGHT_BLUE);
        print(dead, 21+i*4, 42, C_BG_BLACK | C_FG_WHITE);
        }
    }
    //Dibujamos las cajas donde escribiremos el puntaje de los equipos
    screen_draw_box(43, 9, 3, 10, 0, C_BG_RED | C_FG_RED);
    screen_draw_box(43, 62, 3, 10, 0, C_BG_BLUE | C_FG_BLUE);
    
    //escribimos el puntaje incializado con 0 en las cajas de cada equipo
    uint8_t score = 0;
    print_dec(score, 6, 11, 44, C_BG_RED | C_FG_WHITE);
    print_dec(score, 6, 64, 44,  C_BG_BLUE | C_FG_WHITE);

    //Inicializamos la información de los lemmings en el array de lemmings
    lemmings_init();
}

void lemmings_init(){
    //Recorremos el array incializando cada estructura lemming dentro del array de lemmings con los datos pertinentes para los
    //lemmings de cada equipo
    for(int i = 0; i < 10; i++){
        if(i%2 ==0){
            lemmings[i].age=0;
            lemmings[i].position_x = 0;
            lemmings[i].position_y = 39;
            lemmings[i].terrain = '.';
            lemmings[i].team = 'A';
            lemmings[i].health = 0;
        }else{
            lemmings[i].age = 0;
            lemmings[i].position_x = 79;
            lemmings[i].position_y = 0;
            lemmings[i].terrain = '.';
            lemmings[i].team = 'B';
            lemmings[i].health = 0;
        }
    }
}

/* 
================================================================================
                 FUNCIONES DEL SISTEMA DEL JUEGO
================================================================================
*/

//Esta función se encarga de actualizar la edad de los lemmings, aumentar los contadores que indican el momento de crear y matar lemmings,
//crear y matar lemming cuando corresponda y chequear que si se cumple la condición de victoria para algun equipo.
bool game_tick() {
    //Aumentamos el contador de clock para crea y matar lemmings
    spawn_ticks++;
    kill_ticks++;

    //Chequeamos si ganó alguno de los dos equipos
    checkWinCondition();

    //Aumentamos la edad de todos los lemmings en 1
    for(uint8_t i = 0; i < 10; i++){
        if(lemmings[i].health != 0)
            lemmings[i].age++;
    }

    //Chequeamos si llegamos al un ciclo de reloj donde debamos crear lemmings.
    if(spawn_ticks%401 == 0){
        if(alive(0) < 5){
            lemmingCreate(0);
        }
        if(alive(1)<5){
            lemmingCreate(1);
        }
    }

   
    //Chequeamos si llegamos al un ciclo de reloj donde debamos matar lemmings viejos.
    if(kill_ticks%2001 == 0){
        uint8_t oldLemmingA=0;
        uint8_t oldLemmingB=1;
        //Buscamos el lemming más viejo de cada equipo.
        for(uint8_t i = 0; i < 10; i+=2){
            if(lemmings[i].age > lemmings[oldLemmingA].age && lemmings[i].health != 0)
                oldLemmingA = i;
            if(lemmings[i+1].age > lemmings[oldLemmingB].age && lemmings[i+1].health != 0)
                oldLemmingB = i+1;
        }
        //Si los lemmings vivos de cada equipo son 5 entonces matamos el más viejo y luego lo revivimos
        if(alive(0) == 5){
            killLemming(oldLemmingA);
            lemmingCreate(0);
            return (getCurrentTask() == oldLemmingA || getCurrentTask() == oldLemmingB);
        }
        if(alive(1)== 5){
            killLemming(oldLemmingB);
            lemmingCreate(1);
            return (getCurrentTask() == oldLemmingA || getCurrentTask() == oldLemmingB);
        }
            
    }    

    return false;

}

//Función que crea a los lemmings. Recibe el equipo como parametro. 0 = Amalin, 1 = Betarote 
void lemmingCreate(int equipo){
    //Guardamos el puntero a la pantalla ya que vamos a tener que poner el nuevo lemming en pantalla
    ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;

    //Buscamos para el equipo que corresponda  el primer lemming muerto para devolverlo a la vida. 
    for(int i = equipo; i < 10; i+=2){
        if(lemmings[i].health == 0)
        {
            //Ponemos la posición del punto de creación para el equipo que corresponda en vars auxiliares
            int auxX = equipo % 2 == 0 ? 0 : 79;
            int auxY = equipo % 2 == 0 ? 39 : 0;

            //Si el punto de creación está ocupado entonces no hacemos nada. Falla la creación
            if(p[auxY][auxX].c == 'A' || p[auxY][auxX].c == 'B')
                return;

            //Creamos la tss para el nuevo lemmings. Recordar que el índice del lemming se corresponde con el de su tarea.
            tss_create(i, equipo % 2 == 0 ? 0x18000 : 0x1A000);
            //Una vez creada la tarea la ponemos como activa.
            setTaskActive(i);
            
            //Ponemos la info del nuevo lemming en el arreglo de lemmings
            lemmings[i].age = 0;
            lemmings[i].position_x = auxX;
            lemmings[i].position_y = auxY;
            lemmings[i].terrain = '.';
            lemmings[i].team = equipo % 2 == 0 ? 'A' : 'B';
            lemmings[i].health = 1;
            
            //Imprimimos el lemming
            print(equipo % 2 == 0 ? "A" : "B", lemmings[i].position_x, lemmings[i].position_y, 
            equipo%2 == 0 ? (C_BG_RED | C_FG_WHITE) : (C_BG_BLUE | C_FG_WHITE));

            //Actualizamos la cantidad de lemmings creados para el equio que corresponda
            if(equipo % 2 == 0){
                spawnedA++;
                print_dec(spawnedA, 6, 11, 44, C_BG_RED | C_FG_WHITE);
            }
               
            else{
                spawnedB++;
                print_dec(spawnedB, 6, 64, 44, C_BG_BLUE | C_FG_WHITE);
            }
            return;
        }
    }
}

//Función que mata lemmings. Recibe el indice del lemming que debe ser asesinado.
void killLemming(int lemming){
    
    //Guardamos el terreno sobre el cual estaba el lemming para sobrescribir en la pantalla.
    //Como print no acepta chars lo transformamos en string
    char terrain = lemmings[lemming].terrain;
    char* terrainAux = "";
    terrainAux[0] =  lemmings[lemming].terrain;
    terrainAux[1] = '\0';

    //Vida en 0
    lemmings[lemming].health = 0;

    //Quitamos de la pantalla
    print(terrainAux, lemmings[lemming].position_x, lemmings[lemming].position_y, getTerrainAttributes(terrain));

    //Quitasmos del scheduler
    desalojar(lemming);
}


//Chequeamos la condición de victoria para los dos equipos.
void checkWinCondition(){
    //Guardamos el puntero a la pantalla
    ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;

    //Chequeamos en la primera y última columna de la pantalla para ver si hay lemmings del equipo B y A respectivamente
    //Si lo hay escribimos un mensaje proclamando al ganador. WINNER WINNER CHICKEN DINNER
    for(int i = 0; i < 40; i++){
        if(p[i][0].c == 'B' || p[i][79].c == 'A'){
            print("GANO EL EQUIPO: ", 40, 20, C_BG_BLACK|C_FG_LIGHT_GREEN);
            print(p[i][0].c == 'B' ? "B" : "A", 56, 20, C_BG_BLACK|C_FG_LIGHT_GREEN);
            endGame();
        }
    }
}

//Función que nos permite obtener los atributos del terreno pasado por parametro.
uint16_t getTerrainAttributes(char terrain){
    uint16_t res = 0;
    switch(terrain)
    {
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

//Función que indica cuandot lemmings vivos quedan en el equipo pasado por parametro. 0 = Amalin, 1= Betarote
int alive(int equipo){
    int res = 0;
    for(int i = equipo; i < 10; i+=2){
        if(lemmings[i].health != 0)
            res++;
    }
    return res;
}

/*================================================================================
                 FUNCIONES DE LOS LEMMINGS
================================================================================*/

//Implementación en C del servicio que mueve a los lemmings
move_result_e movLemming(direction_e dir){

    //Guardamos el puntero a la pantalla ya que vamos a tener que cambiar la posición del lemming en la misma
    ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;

    //Obtenemos el lemming que va a moverse, y guardamos parte de su info en vars auxiliares
    //. El índice del lemming coincide con el indice de su tarea en el scheduler.
    uint8_t currentLemming = getCurrentTask();
    uint8_t XPos = lemmings[currentLemming].position_x;
    uint8_t YPos = lemmings[currentLemming].position_y;
    char terrain = lemmings[currentLemming].terrain;
    //El print no funciona con chars, así que convertimos el char del terreno en string.
    char* terrainAux = "";
    terrainAux[0] =  lemmings[currentLemming].terrain;
    //Por default el resultado es que intentamos movernos fuera del mapa
    move_result_e res = MOVE_RESULT_BORDER;

    //ARRIBA
    //Chequeamos que podamos movernos en la dirección pedida. Para ello chequemos primero que no nos vayamos fuera de la pantalla
    //Luego chequeamos un lugar arriba de donde está el lemming para ver si es un terreno valido.
    if(dir == DIR_UP && (YPos-1) >= 0){
        if(p[YPos-1][XPos].c == '.' || p[YPos-1][XPos].c == 'x' || p[YPos-1][XPos].c == '+')
        {   
            //Actualizamos el terreno y la posición del lemming
            lemmings[currentLemming].terrain = p[YPos-1][XPos].c;
            lemmings[currentLemming].position_y--;
            print(currentLemming % 2 == 0 ? "A" : "B", lemmings[currentLemming].position_x, lemmings[currentLemming].position_y, 
            lemmings[currentLemming].team == 'A' ? C_BG_RED | C_FG_WHITE : C_BG_BLUE | C_FG_WHITE);
            //Donde antes estaba el lemming escribimos el terreno sobre el que estaba parado.
            p[YPos][XPos].c = terrain;
            p[YPos][XPos].a = getTerrainAttributes(terrain);  
            return MOVE_RESULT_SUCCESS;      
        }
        //Si el terreno no es valido no fijamos que tipo de terreno produjo el fallo y lo devolvemos
        else{
            if(p[YPos-1][XPos].c == 'P'){
                return MOVE_RESULT_WALL;
            }
            else if(p[YPos-1][XPos].c == 'L'){
                return MOVE_RESULT_LAKE;
            }
            else if(p[YPos-1][XPos].c == 'A' || p[YPos-1][XPos].c == 'B' ){
                return MOVE_RESULT_LEMMING;
            }
        }
    }

    //DERECHA
    //Chequeamos que podamos movernos en la dirección pedida. Para ello chequemos primero que no nos vayamos fuera de la pantalla
    //Luego chequeamos un lugar hacia la derecha de donde está el lemming para ver si es un terreno valido.
    else if(dir == DIR_RIGHT && (XPos+1)<80){
        if(p[YPos][XPos+1].c == '.' || p[YPos][XPos+1].c == 'x' || p[YPos][XPos+1].c == '+')
        {   
            //Actualizamos el terreno y la posición del lemming
            lemmings[currentLemming].terrain = p[YPos][XPos+1].c;
            lemmings[currentLemming].position_x++;
            print(currentLemming % 2 == 0 ? "A" : "B", lemmings[currentLemming].position_x, lemmings[currentLemming].position_y, 
            lemmings[currentLemming].team == 'A' ? (C_BG_RED | C_FG_WHITE) : (C_BG_BLUE | C_FG_WHITE));
            //Donde antes estaba el lemming escribimos el terreno sobre el que estaba parado.
            p[YPos][XPos].c = terrain;
            p[YPos][XPos].a = getTerrainAttributes(terrain);
            return MOVE_RESULT_SUCCESS;
        //Si el terreno no es valido no fijamos que tipo de terreno produjo el fallo y lo devolvemos    
        }else{
            if(p[YPos][XPos+1].c == 'P'){
                return MOVE_RESULT_WALL;
            }
            else if(p[YPos][XPos+1].c == 'L'){
                return MOVE_RESULT_LAKE;
            }
            else if(p[YPos][XPos+1].c == 'A' || p[YPos][XPos+1].c == 'B' ){
                return MOVE_RESULT_LEMMING;
            }
        }
    }
    //ABAJO
    //Chequeamos que podamos movernos en la dirección pedida. Para ello chequemos primero que no nos vayamos fuera de la pantalla
    //Luego chequeamos un lugar abajo de donde está el lemming para ver si es un terreno valido.
    else if(dir == DIR_DOWN && (YPos+1)<40){
        if(p[YPos+1][XPos].c == '.' || p[YPos+1][XPos].c == 'x' || p[YPos+1][XPos].c == '+')
        {   
            //Actualizamos el terreno y la posición del lemming
            lemmings[currentLemming].terrain = p[YPos+1][XPos].c;
            lemmings[currentLemming].position_y++;
            print(currentLemming % 2 == 0 ? "A" : "B", lemmings[currentLemming].position_x, lemmings[currentLemming].position_y, 
            lemmings[currentLemming].team == 'A' ? (C_BG_RED | C_FG_WHITE) : (C_BG_BLUE | C_FG_WHITE));
            //Donde antes estaba el lemming escribimos el terreno sobre el que estaba parado.
            p[YPos][XPos].c = terrain;
            p[YPos][XPos].a = getTerrainAttributes(terrain);
            return MOVE_RESULT_SUCCESS;
        //Si el terreno no es valido no fijamos que tipo de terreno produjo el fallo y lo devolvemos 
        }else{
            if(p[YPos+1][XPos].c == 'P'){
                return MOVE_RESULT_WALL;
            }
            else if(p[YPos+1][XPos].c == 'L'){
                return MOVE_RESULT_LAKE;
            }
            else if(p[YPos+1][XPos].c == 'A' || p[YPos-1][XPos].c == 'B' ){
                return MOVE_RESULT_LEMMING;
            }
        }
    }
    //IZQUIERDA
    //Chequeamos que podamos movernos en la dirección pedida. Para ello chequemos primero que no nos vayamos fuera de la pantalla
    //Luego chequeamos un lugar hacia la izquierda de donde está el lemming para ver si es un terreno valido.
    else if(dir == DIR_LEFT && (XPos-1 >= 0)){
        if(p[YPos][XPos-1].c == '.' || p[YPos][XPos-1].c == 'x' || p[YPos][XPos-1].c == '+')
        {   
            //Actualizamos el terreno y la posición del lemming
            lemmings[currentLemming].terrain = p[YPos][XPos-1].c;
            lemmings[currentLemming].position_x--;
            print(currentLemming % 2 == 0 ? "A" : "B", lemmings[currentLemming].position_x, lemmings[currentLemming].position_y, 
            lemmings[currentLemming].team == 'A' ? (C_BG_RED | C_FG_WHITE) : (C_BG_BLUE | C_FG_WHITE));
            //Donde antes estaba el lemming escribimos el terreno sobre el que estaba parado.
            p[YPos][XPos].c = terrain;
            p[YPos][XPos].a = getTerrainAttributes(terrain);
            return MOVE_RESULT_SUCCESS;
        //Si el terreno no es valido no fijamos que tipo de terreno produjo el fallo y lo devolvemos 
        }else{
            if(p[YPos][XPos-1].c == 'P'){
                return MOVE_RESULT_WALL;
            }
            else if(p[YPos][XPos-1].c == 'L'){
                return MOVE_RESULT_LAKE;
            }
            else if(p[YPos][XPos-1].c == 'A' || p[YPos][XPos-1].c == 'B' ){
                return MOVE_RESULT_LEMMING;
            }
        }
    }
    //Si no entramos en ninguno de los cuatro condicionales anteriores entonces es que el movimiento se sale de la pantalla.
    return res;
}


//Implementacion en C del servicio que inmola a los lemmings.
void boom(){
    //Guardamos el puntero a la pantalla ya que vamos a tener que cambiar el mapa en el caso que se exploten paredes
    ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;

    //Obtenemos el lemming que va a morirse, y guardamos parte de su info en vars auxiliares.
    //El índice del lemming coincide con el indice de su tarea en el scheduler.
    uint8_t currentLemming = getCurrentTask();
    uint8_t XPos = lemmings[currentLemming].position_x;
    uint8_t YPos = lemmings[currentLemming].position_y;
    
    //Variable que usaremos para guardar el indice de los lemmings que mueran con la exploción
    uint8_t boomedLemming;

    killLemming(currentLemming);

    //Ahora chequeamos en cada dirección alrededor del lemming que explotó si hay paredes u otros lemmings
    //DERECHA
    if((XPos + 1) < 80){
        char aux = p[YPos][XPos+1].c;
        //Check Pared
        if(aux == 'P'){
            p[YPos][XPos+1].c = 'x';
            p[YPos][XPos+1].a = C_BG_RED | C_FG_GREEN;
        
        //Check Lemming
        }else if( aux == 'A' || aux == 'B'){
            //Obtenemos la posición del lemming que muere
            for(int i = 0; i < 10; i++){
                if(lemmings[i].position_x == XPos+1 && lemmings[i].position_y == YPos)
                    boomedLemming = i;                
            }
            killLemming(boomedLemming);
        }
    }
    //IZQUIERDA
    if((XPos-1) >= 0){
        char aux = p[YPos][XPos-1].c;
        //Check Pared
        if(aux == 'P'){
            p[YPos][XPos-1].c = 'x';
            p[YPos][XPos-1].a = C_BG_RED | C_FG_GREEN;
        //Check Lemming
        }else if(aux == 'A' || aux == 'B'){
            //Obtenemos la posición del lemming que muere
            for(int i = 0; i < 10; i++){
                if(lemmings[i].position_x == XPos-1 && lemmings[i].position_y == YPos)
                    boomedLemming = i;                
            }
            killLemming(boomedLemming);
        }
    }
    //ARRIBA
    if((YPos+1) < 40){
        char aux = p[YPos+1][XPos].c;
        //Check Pared
        if(aux == 'P'){
            p[YPos+1][XPos].c = 'x';
            p[YPos+1][XPos].a = C_BG_RED | C_FG_GREEN;
        //Check Lemming
        }else if( aux == 'A' || aux == 'B'){
            //Obtenemos la posición del lemming que muere
            for(int i = 0; i < 10; i++){
                if(lemmings[i].position_x == XPos && lemmings[i].position_y == YPos+1)
                    boomedLemming = i;                
            }
            killLemming(boomedLemming);
        }
    }
    //ABAJO
    if((YPos-1) >= 0){
        char aux = p[YPos-1][XPos].c;
        //Check Pared
        if(aux == 'P'){
            p[YPos-1][XPos].c = 'x';
            p[YPos-1][XPos].a = C_BG_RED | C_FG_GREEN;
        //Check Lemming
        }else if( aux == 'A' || aux == 'B'){
            //Obtenemos la posición del lemming que muere
            for(int i = 0; i < 10; i++){
                if(lemmings[i].position_x == XPos && lemmings[i].position_y == YPos-1)
                    boomedLemming = i;                
            }
            killLemming(boomedLemming);
        }
    }
}

//Implementacion en C del servicio que crea puentes
void bridge(direction_e dir){
    //Guardamos el puntero a la pantalla ya que vamos a tener que poner un puente en caso de que haya agua
    ca(*p)[VIDEO_COLS] = (ca(*)[VIDEO_COLS])VIDEO;
    //Obtenemos el lemming que va a moverse, y guardamos parte de su info en vars auxiliares.
    //El índice del lemming coincide con el indice de su tarea en el scheduler.
    uint8_t currentLemming = getCurrentTask();
    uint8_t XPos = lemmings[currentLemming].position_x;
    uint8_t YPos = lemmings[currentLemming].position_y;
    
    //Ahora chequeamos para la dirección correspondiente si podemos poner un puente
    //Para ello chequeamos que no salgamos fuera de la pantalla y luego que haya efectivamente un lago.

    //ARRIBA
    if(dir == DIR_UP && (YPos-1) >= 0){
        if(p[YPos-1][XPos].c == 'L'){
            p[YPos-1][XPos].c = '+';
            p[YPos-1][XPos].a = getTerrainAttributes('+');
            killLemming(currentLemming);
        }
    }
    //DERECHA
    else if(dir == DIR_RIGHT && (XPos+1) < 80){
        if(p[YPos][XPos+1].c == 'L'){
            p[YPos][XPos+1].c = '+';
            p[YPos][XPos+1].a = getTerrainAttributes('+');
            killLemming(currentLemming);

        }
    }
    //ABAJO
    else if(dir == DIR_DOWN && (YPos+1)< 40){
        if(p[YPos+1][XPos].c == 'L'){
            p[YPos+1][XPos].c = '+';
            p[YPos+1][XPos].a = getTerrainAttributes('+');
            killLemming(currentLemming);

        }
    }
    //IZQUIERDA
    else if(dir == DIR_LEFT && (XPos-1) >= 0){
        if(p[YPos][XPos-1].c == 'L'){
            p[YPos][XPos-1].c = '+';
            p[YPos][XPos-1].a = getTerrainAttributes('+');
            killLemming(currentLemming);;
        }
    }
}


