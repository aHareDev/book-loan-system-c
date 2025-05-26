#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "entities.h"

void buffer_init(Buffer *buf) {
    buf->in = 0;
    buf->out = 0;
    pthread_mutex_init(&buf->mutex, NULL);
    sem_init(&buf->full, 0, 0);                 // 0 elementos disponibles para consumir
    sem_init(&buf->empty, 0, ITEMS_BUFFER);     // Todo el buffer está vacío
}

void buffer_destroy(Buffer *buf) {
    pthread_mutex_destroy(&buf->mutex);
    sem_destroy(&buf->full);
    sem_destroy(&buf->empty);
}

void buffer_insert(Buffer *buf, Request *item) {
    sem_wait(&buf->empty);                     // Espera hasta que haya espacio
    pthread_mutex_lock(&buf->mutex);

    buf->items[buf->in] = *item;
    buf->in = (buf->in + 1) % ITEMS_BUFFER;

    pthread_mutex_unlock(&buf->mutex);         // Libera el buffer
    sem_post(&buf->full);                      // Señala que hay un nuevo elemento
}

void buffer_consume(Buffer *buf, Request *item) {
    sem_wait(&buf->full);                      // Espera hasta que haya algo para consumir
    pthread_mutex_lock(&buf->mutex);           // Bloquea el buffer

    *item = buf->items[buf->out];
    buf->out = (buf->out + 1) % ITEMS_BUFFER;

    pthread_mutex_unlock(&buf->mutex);
    sem_post(&buf->empty);
}


