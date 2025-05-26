#ifndef BUFFER_H
#define BUFFER_H

#include "entities.h"

void buffer_init(Buffer *buf);
void buffer_destroy(Buffer *buf);
void buffer_insert(Buffer *buf, Request *item);
void buffer_consume(Buffer *buf, Request *item);

#endif
