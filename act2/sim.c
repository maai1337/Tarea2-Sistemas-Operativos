#include "sim.h"
#include <stdio.h>
#include <descomponer.h>
#include <obtener_direccion_fisica.h>


void simulador(){
    int puntero_reloj = 0;





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
    if(tabla_paginas[nvp].valid = 1){
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
        tabla_paginas[nvp].valid = -1;
        tabla_paginas[nvp].referencia = 1;
    } 

    unsigned int df = obtener_direccion_fisica(marco, offset, page_bits);

    return df;

}
