#include "sim.h"
#include <stdio.h>
#include <stdlib.h>
#include "descomponer.h"
#include "obtener_direccion_fisica.h"
#include <string.h>


void simulador(const char *archivo_traza, int n_marcos, int tam_marco, int verbose){
    FILE *fp = fopen(archivo_traza, "r");
    if (fp == NULL){
        fprintf(stderr, "Error. No se puede abrir el archivo de traza '%s'\n", archivo_traza);
        exit(1);
    }

    int page_bits = calcular_page_bits(tam_marco);

    int num_paginas_virtuales = (1 << (32 - page_bits)); // Asumimos direcciones de 32 bits

    PageTableEntry *tabla_paginas;
    Marco *memoria_fisica;

    inicializar_estructuras(&tabla_paginas, &memoria_fisica, n_marcos, num_paginas_virtuales);

    int puntero_reloj = 0;
    int total_fallos = 0; 
    int total_accesos = 0;

    char linea[256];

    if (verbose)
    {
        printf("===========================\n");
        printf("Marcos físicos: %d\n", n_marcos);
        printf("Tamaño de marco: %d\n", tam_marco);
        printf("Bits de offset: %d\n", page_bits);
        printf("===========================\n");
    }

    while (fgets(linea, sizeof(linea), fp) !=NULL)
    {
        // Elimina salto de linea
        linea[strcspn(linea, "\r\n")] = 0;

        unsigned int dv = parsear_direccion(linea);
        int fallo;

        total_accesos++;
        if (verbose) {
            printf("Acceso #%d: DV = %u (0x%X)\n", total_accesos, dv, dv);
        }

        unsigned int df = traducir_direccion(dv, tabla_paginas, memoria_fisica, n_marcos,
                                             page_bits, &puntero_reloj, &fallo, verbose);
        
        if (fallo)
        {
            total_fallos++;
        }
        
        if (verbose)
        {
            unsigned int nvp, offset;
            descomponer(dv, &nvp, &offset, page_bits);
            printf("NVP = %u, offset = %u\n", nvp, offset);
            printf("Direccion física = %u, (0x%X)", df, df);
            printf("%s\n\n", fallo ? "FALLO DE PÁGINA" : "HIT");
        }    
    }
    
    printf("\n===RESULTADOS===");
    printf("Total de accesos: %d \n", total_accesos);
    printf("Fallos de página: %d \n", total_fallos);
    printf("Tasa de fallos: %.2f%%\n", (double)total_fallos / total_accesos * 100);
    printf("=================\n");

    fclose(fp);
    free(tabla_paginas);
    free(memoria_fisica);


}

int buscar_marco_libre(Marco *memoria_fisica, int n_marcos){
    for (int i = 0; i < n_marcos; i++)
    {
        if (memoria_fisica[i].pagina_virtual == -1)
        {
            return i;
        }        
    }
    return -1; // NO HAY MARCOS LIBRES
}


int algoritmo_reloj(Marco *memoria_fisica, int n_marcos, int *puntero_reloj){
    while (1) {
        if (memoria_fisica[*puntero_reloj].bit_referencia == 0){
            int victima = *puntero_reloj;
            *puntero_reloj = (*puntero_reloj + 1) % n_marcos;
            return victima;
        }
        // SEGUNDA OPORTUNIDAD SI ES 1
        memoria_fisica[*puntero_reloj].bit_referencia = 0;
        *puntero_reloj = (*puntero_reloj + 1) % n_marcos;
    }
}

unsigned int traducir_direccion(unsigned int dv, PageTableEntry *tabla_paginas, Marco *memoria_fisica,
                                int n_marcos, int page_bits, int *puntero_reloj, int *fallo, int verbose)
    {
    unsigned int nvp, offset;

    descomponer(dv, &nvp ,&offset, page_bits);

    *fallo = 0;
    int marco;

    // hit
    if(tabla_paginas[nvp].valid == 1){
        marco = tabla_paginas[nvp].marco_fisico;
        memoria_fisica[marco].bit_referencia = 1;
        tabla_paginas[nvp].referencia = 1;
        if (verbose)
        {
            printf("HIT : Página %u ya está en marco %d \n", nvp, marco);
        }
        
    } else{ // page fault
        *fallo = 1;
        marco = buscar_marco_libre(memoria_fisica, n_marcos);

        if (marco != -1) { // hay marco libre
            if (verbose)
            {
                printf("FALLO: Página %u no está en memoria. Asignando marco %d \n", nvp, marco);
            }
        } else { 
            marco = algoritmo_reloj(memoria_fisica, n_marcos, puntero_reloj);
            int pagina_victima = memoria_fisica[marco].pagina_virtual;

            if (verbose)
            {
                printf("FALLO: Página %u no está en memoria. Reemplazando página %d en marco %d \n",
                nvp, pagina_victima, marco);
            }
            
            // Invalidar la página victima en la tabla de páginas
            if (pagina_victima >= 0) 
            {
                tabla_paginas[pagina_victima].valid = 0;
                tabla_paginas[pagina_victima].marco_fisico = -1;
        
            }
        }
        
        // Se asigna el marco a la nueva pagina
        memoria_fisica[marco].pagina_virtual = nvp;
        memoria_fisica[marco].bit_referencia = 1;

        tabla_paginas[nvp].marco_fisico = marco;
        tabla_paginas[nvp].valid = 1;
        tabla_paginas[nvp].referencia = 1;
    } 

    unsigned int df = obtener_direccion_fisica(marco, offset, page_bits);

    return df;

}

int calcular_page_bits(int tam_marco){
    int bits = 0;
    int tam = tam_marco;
    while (tam > 1)
    {
        tam >>=1;
        bits++;
    }
    return bits;
}

unsigned int parsear_direccion(const char *str){
    unsigned int dv;
    if (strncmp(str, "0x", 2) == 0)
    {
        sscanf(str, "%x", &dv);
    } else
    {
        sscanf(str, "%u", &dv);
    }
    return dv;
}

int main(int argc, char *argv[]){

    

    int n_marcos = atoi(argv[1]);
    int tam_marco = atoi(argv[2]);
    int verbose = 0;
    const char *archivo_traza;


    if (n_marcos <= 0){
        fprintf(stderr, "Error. El número de marcos debe ser positivo");
        return 1;
    }

    if (tam_marco <= 0 || (tam_marco & (tam_marco - 1) != 0)){
        fprintf(stderr, "Error. tamaño del marco debe ser una potencia de dos");
        return 1;
    }
    
    if (argc == 4) {
        archivo_traza = argv[3];
    } else if (argc == 5)
    {
        if (strncmp(argv[3], "--verbose", 9) == 0)
        {
            verbose = 1;
            archivo_traza = argv[4];
        }  else {
                fprintf(stderr, "Error. Argumento desconocido");
        }
    } else
    {
        fprintf(stderr, "Error. Demasiados argumentos");
    }

    simulador(archivo_traza, n_marcos, tam_marco, verbose);


}