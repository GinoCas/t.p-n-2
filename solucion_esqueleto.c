// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
// LIBRERIA PARA TESTING
#include <assert.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

int len(char* s1) {
    int i = 0;
    while (s1[i] != 0) {
        i++;
    }
    return i;
}

char* strDuplicate(char* src){
    int cantLetras = len(src); // -> 9 letras (no inclue \0)
    char *srcDuplicado = (char*) malloc ((cantLetras + 1) * sizeof(char)); // -> pedimos la memoria para duplicar
    if (srcDuplicado == NULL) {
        return NULL;
    }
    for (int i = 0; i < cantLetras; i++){
        srcDuplicado[i] = src[i];
    }
    srcDuplicado[cantLetras] = '\0';
    return srcDuplicado;
}

int strCompare(char* s1, char* s2){
    int count = 0;
    while(s1[count] == s2[count] && s1[count] != '\0'){
        count++;
    }
    if(s1[count] < s2[count]){
        return 1;
    }
    if(s1[count] > s2[count]){
        return -1;
    }
    return 0;
}


char* strConcatenate(char* src1, char* src2){
    int lenSrc1 = len(src1);
    int lenSrc2 = len(src2);
    char *concat = (char*) malloc(sizeof(char) * (lenSrc1 + lenSrc2 + 1));
    if (concat == NULL) {
        free(src1);
        free(src2);
        return NULL;
    }
    for (int i = 0; i < lenSrc1; i++) {
        concat[i] = src1[i];
    }
    for (int j = 0; j < lenSrc2; j++) {
        concat[lenSrc1 + j] = src2[j];
    }
    concat[lenSrc1 + lenSrc2] = 0;
    free(src1);
    free(src2);
    return concat;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========
RowNode* createRowNode(){
    RowNode* node = (RowNode*) malloc(sizeof(RowNode));
    if (node != NULL) {
        node->next = NULL;
    }
    return node;
}

void fillCellsZero(int* cells){
    for (int i = 0; i < GRID_COLS; i++) {
        cells[i] = 0;
    }
}

RowNode* addLast(RowNode *previousNode) {
    RowNode *newNode = createRowNode();
    if(newNode != NULL){
        previousNode->next = newNode;
        fillCellsZero(newNode->cells);
    }
    return newNode;
}
/* Funciones auxiliares de limpieza */
void clearGameRows(RowNode *initRow){
    if(initRow == NULL){
        return; 
    }
    clearGameRows(initRow->next);
    free(initRow);
}

void clearGameParticles(ParticleNode *initParticle){
    if(initParticle == NULL){
        return; 
    }
    clearGameParticles(initParticle->next);
    free(initParticle);
}

void clearGamePieces(PieceQueueNode *initPiece){
    if(initPiece == NULL){
        return; 
    }
    clearGamePieces(initPiece->next);
    free(initPiece);
}

void gameBoardDelete(GameBoard* board) {
    // Debe tolerar board == NULL.
    if(board == NULL){
        free(board);
        return;
    }
    // Recorrer y liberar todos los RowNode con free().
    clearGameRows(board->first_row);
    // Recorrer y liberar todos los PieceQueueNode.
    clearGamePieces(board->queue_head);
    // Recorrer y liberar todos los ParticleNode.
    clearGameParticles(board->first_particle);
    // Finalmente, liberar la estructura GameBoard.
    free(board);
}

GameBoard* gameBoardNew() {
    // Reservar memoria para el GameBoard.
    GameBoard *board = (GameBoard*) malloc(sizeof(GameBoard));
    if(board == NULL){
        free(board);
        return NULL;
    }
    // Inicializar queue_head, queue_tail y first_particle en NULL.
    board->score = 0;
    board->tick_timer = 0;
    board->first_row = NULL;
    board->first_particle = NULL;
    board->queue_head = NULL;
    board->queue_tail = NULL;
    board->current_piece.active = 0;
    board->current_piece.col = 0;
    board->current_piece.rotation = 0;
    board->current_piece.row = 0;
    board->current_piece.type = 0;

    
    // Usar un bucle para hacer malloc de las 20 filas (RowNode) y conectarlas.
    RowNode *initRow = createRowNode();
    if(initRow == NULL){
        gameBoardDelete(board);
        return NULL;
    }
    fillCellsZero(initRow->cells);
    board->first_row = initRow;
    RowNode *endRow = initRow;
    for (int i = 1; i<GRID_ROWS; i++){   
        endRow = addLast(endRow);
        if(endRow == NULL){
            gameBoardDelete(board);
            return NULL;
        }
    }
    // El puntero first_row debe apuntar al primero de esos nodos.
    board->first_row = initRow;

    return board;
}

void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {
    struct ParticleNode *newParticle = (struct ParticleNode*) malloc(sizeof(ParticleNode));
    if (newParticle == NULL) {
        return;
    }
    newParticle->x = x;
    newParticle->y = y;
    newParticle->color = color;
    newParticle->vel_x = (float) rand() / RAND_MAX * 8.0f - 4.0f;
    newParticle->vel_y = (float) rand() / RAND_MAX * 8.0f - 4.0f;
    newParticle->life_timer = 31 + rand() % 30;
    newParticle->next = board->first_particle;
    board->first_particle = newParticle;
}

/* Helpers */

bool rowFilled(const RowNode* row) {
    if (row == NULL) return false;
    int currCell = 0;
    while (currCell < 10 && row->cells[currCell] > 0){currCell++;}
    if(currCell == 10){
        return true;
    }
    return false;
}

RowNode* addFirst(RowNode* nextRow) {
    RowNode *newNode = createRowNode();
    if(newNode != NULL){
        newNode->next = nextRow;
        fillCellsZero(newNode->cells);
    }
    return newNode;
}

void doParticlesEffect(GameBoard* board, RowNode* row, int rowId) {
    for (int col = 0; col < GRID_COLS; col++) { // misma logica que gameboardDraw (el for recorre columnas)
        float x = (float)(OFFSET_X + col * CELL_SIZE);
        float y = (float)(OFFSET_Y + rowId * CELL_SIZE);
        int color = row->cells[col];
        gameBoardAddParticle(board, x, y, color);
    }
}

int gameBoardCheckAndClearLines(GameBoard* board) {
    if(board == NULL){
        return 0;
    }
    // Recorrer la lista de RowNode.
    RowNode* currRow = board->first_row;
    RowNode* prevRow = NULL;
    int deletedCount = 0;
    int rowId = 0; // fila actual
    while(currRow != NULL){
        // Si un RowNode tiene todas sus celdas > 0, desenlazarlo y hacer free().
        if(rowFilled(currRow)){ // True si la fila esta llena
            // Por cada celda destruida, invocar a gameBoardAddParticle().
            doParticlesEffect(board, currRow, rowId);

            // Eliminar fila llena
            RowNode* tempRow = currRow->next;
            free(currRow);
            currRow = tempRow;
            if(prevRow != NULL){ // Si no es la primera fila
                prevRow->next = tempRow;
            }else{ // Es la primera fila
                board->first_row = tempRow; // El siguiente pasa hacer la "primera" fila
            }
            deletedCount++;
        }else{
            // Siguiente nodo en la lista.
            prevRow = currRow;
            currRow = currRow->next;   
        }
        rowId++;
    }
    // Crear un nuevo RowNode vacío (lleno de ceros).
    for(int i = 0; i<deletedCount; i++){
        // Insertar el nuevo RowNode en la CABEZA del tablero (board->first_row).
        board->first_row = addFirst(board->first_row);
    }
    //printf("Función gameBoardCheckAndClearLines no implementada.\n");
    return deletedCount;
}

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    PieceQueueNode* NewNode = (PieceQueueNode*) malloc (sizeof(PieceQueueNode));
    if(NewNode == NULL){
        return;
    }
    NewNode->type = type;
    NewNode->next = NULL;
    if (board->queue_head == NULL){
        board->queue_head = NewNode;
        board->queue_tail = NewNode;
    }
    else{
        board->queue_tail->next = NewNode;
        board->queue_tail = NewNode;
    }
    
}

int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
    if (board->queue_tail == NULL) {
        return 0;
    }
    *out_type = (board->queue_head)->type;
    struct PieceQueueNode *noda = board->queue_head;
    if (noda->next == NULL) {
        board->queue_head = NULL;
        board->queue_tail = NULL;
    } else {
        board->queue_head = noda->next;
    }
    free(noda);
    return 1;
}

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========

void correrTests() {
    // TODO: Implementar los casos minimos del enunciado. Por ejemplo:
    //  - strDuplicate: "", "x", un string con distintos caracteres.
    //  - strCompare: "" vs "", "a" vs "a", y una desigualdad en ambos sentidos (s1/s2 y s2/s1).
    //  - strConcatenate: "" + "abc" y "hola " + "mundo". Los argumentos deben crearse con
    //    strDuplicate, porque strConcatenate los libera (no pasar literales).
    //  - Cola: 5 push y 6 pop; el 6to debe devolver 0 y dejar queue_head y queue_tail en NULL.
    //    Verificar tambien que un push posterior funcione.
    //  - Tablero: llenar la fila de indice 19 (la de abajo), llamar a
    //    gameBoardCheckAndClearLines y comprobar que first_row es la fila nueva vacia y que
    //    la lista sigue teniendo 20 filas.
    // Liberar toda la memoria pedida en los tests (strings y tableros).
    //printf("Tests no implementados.\n");
    char* strEmpty = strDuplicate("");
    assert(strEmpty != NULL);
    assert(strCompare(strEmpty, "") == 0);
    free(strEmpty);

    char* unCarac = strDuplicate("x");
    assert(unCarac != NULL);
    assert(strCompare(unCarac, "x") == 0);
    free(unCarac);

    char* muchosCarac = strDuplicate("abc123");
    assert(muchosCarac != NULL);
    assert(strCompare(muchosCarac, "abc123") == 0);
    free(muchosCarac);

    assert(strCompare("", "") == 0);
    assert(strCompare("a", "a") == 0);
    assert(strCompare("a", "b") == 1);
    assert(strCompare("b", "a") == -1);
    assert(strCompare("casa", "casas") == 1);
    assert(strCompare("casas", "casa") == -1);

    char* first = strDuplicate("");
    char* second = strDuplicate("abc");
    assert(first != NULL && second != NULL);
    char* joined = strConcatenate(first, second);
    assert(joined != NULL);
    assert(strCompare(joined, "abc") == 0);
    free(joined);

    first = strDuplicate("hola ");
    second = strDuplicate("mundo");
    assert(first != NULL && second != NULL);
    joined = strConcatenate(first, second);
    assert(joined != NULL);
    assert(strCompare(joined, "hola mundo") == 0);
    free(joined);

    //
    GameBoard* board = gameBoardNew ();
    assert(board != NULL);

    //
    gameBoardEnqueuePiece(board, 1);
    assert (board->queue_head->type == 1);
    assert (board->queue_tail->type == 1);
    gameBoardEnqueuePiece(board, 2);
    assert (board->queue_head->type == 1);
    assert (board->queue_tail->type == 2);
    gameBoardEnqueuePiece(board, 3);
    assert (board->queue_head->type == 1);
    assert (board->queue_tail->type == 3);
    gameBoardEnqueuePiece(board, 4);
    assert (board->queue_head->type == 1);
    assert (board->queue_tail->type == 4);
    gameBoardEnqueuePiece(board, 5);
    assert (board->queue_head->type == 1);
    assert (board->queue_tail->type == 5);

    //
    int out_type = 0;
    assert (gameBoardDequeuePiece(board, &out_type) == 1);
    assert (out_type == 1);
    assert (board->queue_head->type == 2);
    assert (board->queue_tail->type == 5);
    assert (gameBoardDequeuePiece(board, &out_type) == 1);
    assert (out_type == 2);
    assert (board->queue_head->type == 3);
    assert (board->queue_tail->type == 5);
    assert (gameBoardDequeuePiece(board, &out_type) == 1);
    assert (out_type == 3);
    assert (board->queue_head->type == 4);
    assert (board->queue_tail->type == 5);
    assert (gameBoardDequeuePiece(board, &out_type) == 1);
    assert (out_type == 4);
    assert (board->queue_head->type == 5);
    assert (board->queue_tail->type == 5);
    assert (gameBoardDequeuePiece(board, &out_type) == 1);
    assert (out_type == 5);
    assert (board->queue_head == NULL);
    assert (board->queue_tail == NULL);
    assert (gameBoardDequeuePiece(board, &out_type) == 0);
    assert (out_type == 5);
    assert (board->queue_head == NULL);
    assert (board->queue_tail == NULL);

    //
    gameBoardEnqueuePiece(board, 1);
    assert (board->queue_head->type == 1);
    assert (board->queue_tail->type == 1);
    assert (gameBoardDequeuePiece(board, &out_type) == 1);
    assert (out_type == 1);
    assert (board->queue_head == NULL);
    assert (board->queue_tail == NULL);
    assert (gameBoardDequeuePiece(board, &out_type) == 0);
    assert (out_type == 1);
    assert (board->queue_head == NULL);
    assert (board->queue_tail == NULL);
    
    gameBoardDelete(board);
    
    //========
    GameBoard* board_row19 = gameBoardNew ();
    assert (board_row19 != NULL);
    
    //
    RowNode* current_row = board_row19->first_row;
    for (int i = 0; i < 19; i++ ){
        current_row = current_row->next;
    }
    for (int c = 0; c < 10; c++){
        current_row->cells[c] = 1;
    }
    //
    assert (gameBoardCheckAndClearLines (board_row19) == 1);
    assert (board_row19->first_row->cells[0] == 0);

    //
    int num_lines0 = 0;
    RowNode* travel_lines0 = board_row19->first_row;
    while (travel_lines0 != NULL){
        num_lines0++;
        travel_lines0 = travel_lines0->next;
    }
    //
    assert (num_lines0 == 20);

    gameBoardDelete(board_row19);

    //========
    GameBoard* board_row01 = gameBoardNew ();
    assert (board_row01 != NULL);

    for (int c = 0; c < 10; c++){
            board_row01->first_row->cells[c] = 1;
        }
    for (int c = 0; c < 10; c++){
            board_row01->first_row->next->cells[c] = 1;
        }
    
    assert (gameBoardCheckAndClearLines (board_row01) == 2);
    
    int num_lines1 = 0;
    RowNode* travel_lines1 = board_row01->first_row;
    while (travel_lines1 != NULL){
        num_lines1++;
        travel_lines1 = travel_lines1->next;
    }
    //
    assert (num_lines1 == 20);

    gameBoardDelete(board_row01);
}

// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
