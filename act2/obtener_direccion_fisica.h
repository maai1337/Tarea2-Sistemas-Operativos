#ifndef OBTENER_MEMORIA_FISICA_H
#define OBTENER_MEMORIA_FISICA_H

#include <stdint.h>


unsigned int obtener_direccion_fisica(unsigned int marco, unsigned int offset, unsigned int page_bits);

#endif
