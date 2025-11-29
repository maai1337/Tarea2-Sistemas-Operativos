#ifndef SIM_H
#define SIM_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int marco_fisico; 
    int valid;       
    int dirty;        // 1 modificado, 0 sino
} PageTableEntry;

typedef struct {
    int pagina_virtual; // Qué página virtual está almacenada aquí
    int ocupado;        // 1 si el marco está ocupado, 0 si está libre
} Marco;

#endif
