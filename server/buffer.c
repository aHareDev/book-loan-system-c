#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "entities.h"

/**
 * buffer_init
 * -----------
 * Inicializa un buffer circular protegido por semáforos y un mutex.
 * Establece los índices `in` y `out` en 0 e inicializa los semáforos:
 * - `full` inicia en 0: no hay elementos listos para consumir.
 * - `empty` inicia en ITEMS_BUFFER: todo el espacio está disponible.
 *
 * Parámetros:
 *   buf - puntero al buffer a inicializar.
 */
void buffer_init(Buffer *buf) {
    buf->in = 0;
    buf->out = 0;
    pthread_mutex_init(&buf->mutex, NULL);
    sem_init(&buf->full, 0, 0);                 // 0 elementos disponibles para consumir
    sem_init(&buf->empty, 0, ITEMS_BUFFER);     // Todo el buffer está vacío
}

/**
 * buffer_destroy
 * --------------
 * Libera los recursos del buffer: destruye el mutex y los semáforos.
 *
 * Parámetros:
 *   buf - puntero al buffer a destruir.
 */
void buffer_destroy(Buffer *buf) {
    pthread_mutex_destroy(&buf->mutex);
    sem_destroy(&buf->full);
    sem_destroy(&buf->empty);
}

/**
 * buffer_insert
 * -------------
 * Inserta un nuevo elemento en el buffer circular.
 * Espera si el buffer está lleno (semáforo `empty` en 0).
 * Protege el acceso con un mutex para evitar condiciones de carrera.
 *
 * Parámetros:
 *   buf  - puntero al buffer donde se insertará el elemento.
 *   item - puntero a la solicitud que se va a insertar.
 */
void buffer_insert(Buffer *buf, Request *item) {
    sem_wait(&buf->empty);                     // Espera hasta que haya espacio
    pthread_mutex_lock(&buf->mutex);	       // Acceso exclusivo al buffer

    buf->items[buf->in] = *item;	       // Copiar la solicitud al buffer
    buf->in = (buf->in + 1) % ITEMS_BUFFER;    // Avanzar índice circular

    pthread_mutex_unlock(&buf->mutex);         // Libera el buffer
    sem_post(&buf->full);                      // Señala que hay un nuevo elemento
}

/**
 * buffer_consume
 * --------------
 * Extrae un elemento del buffer circular.
 * Espera si el buffer está vacío (semáforo `full` en 0).
 * Protege el acceso con un mutex para evitar condiciones de carrera.
 *
 * Parámetros:
 *   buf  - puntero al buffer desde donde se extraerá el elemento.
 *   item - puntero donde se almacenará la solicitud consumida.
 */
void buffer_consume(Buffer *buf, Request *item) {
    sem_wait(&buf->full);                      // Espera hasta que haya algo para consumir
    pthread_mutex_lock(&buf->mutex);           // Bloquea el buffer

    *item = buf->items[buf->out];	       // Copiar solicitud del buffer
    buf->out = (buf->out + 1) % ITEMS_BUFFER;  // Avanzar índice circular

    pthread_mutex_unlock(&buf->mutex);         // Liberar acceso
    sem_post(&buf->empty);		       // Señalar que hay un espacio libre
}


