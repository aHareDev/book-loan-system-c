#ifndef BUFFER_H
#define BUFFER_H

#include "entities.h"

// Inicializa el buffer y sus mecanismos de sincronización.
void buffer_init(Buffer *buf);

// Libera los recursos asociados al buffer.
void buffer_destroy(Buffer *buf);

// Inserta una solicitud en el buffer (productor).
void buffer_insert(Buffer *buf, Request *item);

// Extrae una solicitud del buffer (consumidor).
void buffer_consume(Buffer *buf, Request *item);

#endif
