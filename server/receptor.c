/**
  * receptor.c
  *
  * Proyecto: Sistema para el Préstamo de Libros
  * Curso: Sistemas Operativos
  * Universidad: Pontificia Universidad Javeriana
  * Fecha: 20 Mayo 2025
  *
  * Descripción:
  * Servidor receptor encargado de atender solicitudes de préstamo, devolución,
  * renovación y finalización enviadas a través de un pipe nombrado. Gestiona una
  * base de datos de libros y copias, maneja concurrencia con hilos, y sincroniza
  * el acceso mediante mutex y semáforos.
  *
  * Autor: Arley Bernal, Sergio Pardo
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "entities.h"
#include "buffer.h"
#include "utils.h"

// Declaraciones de funciones auxiliares que se ejecutarán en hilos
void *auxiliar1(void *arg);
void *auxiliar2(void *arg);

int main(int argc, char *argv[]) {
        // Variables para argumentos de línea de comandos y estructura de solicitud
	char *pipeReceptor = malloc(PIPE_NAME_SIZE), *fileData = NULL, *fileOutput = NULL;
	Request req;

	int verbose = 0;
	int ejecutando = 1;

        // Validación de argumentos mínimos
	if (argc < 4) {
		fprintf(stderr, "Uso correcto: ./receptor -p pipeReceptor -f filedatos [-v] [-s filesalida]\n");
		return 1;
	}
	printf("\n🔧 Iniciando servidor ...\n");

        // Procesamiento de argumentos de línea de comandos
	for (int i = 1; i < argc; i++) {
	    if (strcmp(argv[i], "-p") == 0 && ++i < argc) {
	        strncpy(pipeReceptor, argv[i], PIPE_NAME_SIZE - 1);
	        pipeReceptor[PIPE_NAME_SIZE - 1] = '\0';
	    } else if (strcmp(argv[i], "-f") == 0 && ++i < argc) {
	        fileData = argv[i];
	    } else if (strcmp(argv[i], "-v") == 0) {
	        verbose = 1;
	    } else if (strcmp(argv[i], "-s") == 0 && ++i < argc) {
	        fileOutput = argv[i];
	    }
	}

        // Mostrar parámetros configurados
	printf("\nPipe receptor: %s\n", pipeReceptor);
	printf("Archivo base de datos: %s\n", fileData ? fileData : "(No especificado)");
	if (fileOutput) printf("Archivo de salida: %s\n", fileOutput);
	if (verbose)    printf("Modo verbose activado\n");

        // Validación o creación del pipe receptor
	struct stat st;
	if (stat(pipeReceptor, &st) != 0) {
		printf("ipe no encontrado. Se intentará crear: '../ipc/%s'\n", pipeReceptor);
	        snprintf(pipeReceptor, PIPE_NAME_SIZE, "../ipc/pipeReceptor");
		if (mkfifo(pipeReceptor, 0666) != 0) {
			perror("Error al crear el pipe receptor de solicitudes");
			exit(1);
		}
	} else if (!S_ISFIFO(st.st_mode)) {
		fprintf(stderr, "Error: %s existe pero no es un pipe", pipeReceptor);
		exit(1);
	}

        // Inicialización de buffer compartido
	Buffer buffer;
	buffer_init(&buffer);

        // Inicialización de la biblioteca y recursos compartidos
	Library library = {
		.interaction = 1,  // Para que imprima menú al iniciar
		.buf = &buffer,
		.ejecutando = &ejecutando
	};
	strncpy(library.fileOutput, fileOutput, MAX_CHARACTERS - 1);
	library.fileOutput[MAX_CHARACTERS - 1] = '\0';
	pthread_mutex_init(&library.ejecutando_mutex, NULL);
	pthread_mutex_init(&library.print_mutex, NULL);

        // Apertura del pipe receptor en modo no bloqueante
	int fd_read = open(pipeReceptor, O_NONBLOCK);
	if (fd_read < 0) {
		perror("Error al abrir el pipe receptor");
		exit(1);
	}

        // Cargar base de datos desde archivo
	if (!loadLibraryFromFile(&library, fileData)) {
		perror("Error al cargar la base de datos");
		return 0;
	}
	printf("Base de datos cargada correctamente.: '%s'\n", fileData);

	printf("\n\n\n");
	printf("➤ Servidor esperando solicitudes ...\n");

        // Lanzamiento de hilos auxiliares
	pthread_t thread1, thread2;
	pthread_create(&thread1, NULL, auxiliar1, &library);
	pthread_create(&thread2, NULL, auxiliar2, &library);

        // Bucle principal: atender solicitudes de clientes
	while(ejecutando) {
		if (readRequest(fd_read, &req)) {
			if (req.operation == 'D' || req.operation == 'R') {
                                // Devolución o renovación de libro
				char *msg = (req.operation == 'D') ? "✅ La biblioteca está recibiendo el libro."
							: "✅ Renovación aceptada. Nueva fecha de entrega en 7 días.";

				if (sendResponse(&req, msg)) {
					buffer_insert(&buffer, &req); // Solo inserta si se pudo responder al cliente
				} else {
					fprintf(stderr, "No se pudo enviar la respuesta al cliente %d\n", req.pid);
				}
				library.interaction = 1;
			} else if (req.operation == 'P') {
                                // Solicitud de préstamo de libro
				Book *book = library_findBookByISBN(&library, req.isbn);
				Copy *copy = library_findAvailableCopy(&library, req.isbn);
				if (copy && copy->status == 'D') {
					sendResponse(&req, "✅ Solicitud aceptada. Realizando actualización de prestamo.");
					copy->status = 'P';

                                        // Actualizar fecha de préstamo
					time_t t = time(NULL);
					struct tm *now = localtime(&t);
					snprintf(copy->date, sizeof(copy->date), "%02d-%02d-%04d", 
						now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);

                                        // Registrar el préstamo en el historial
	                                library_addReport(&library, 'P', book->title, book->isbn, copy->id, copy->date);
				} else {
					sendResponse(&req, "Solicitud Rechazada. El libro solicitado no tiene copias disponibles.");
				}
				library.interaction = 1;
			} else if (req.operation == 'Q') {
                                // Finalización de solicitudes por parte del cliente
				printf("\n\n");
				printf("\nCliente %d ha terminado sus solicitudes.\n", req.pid);

                                // Guardar estado de la base de datos si se especificó archivo de salida
				if (fileOutput != NULL) {
					if (saveLibraryToFile(&library, fileOutput)) {
						printf("📁 Base de datos guardada exitosamente en '%s'\n\n", fileOutput);
					} else {
						fprintf(stderr, "Error al guardar la base de datos en '%s'\n", fileOutput);
					}
				}
			}
                        // Mostrar solicitud si está activado el modo verbose
			if (verbose) showRequest(&req);
		}
	}
	printf("\n\n");
        // Finalización del servidor
        close(fd_read);
	free(pipeReceptor);
	pipeReceptor = NULL;
        sem_post(&buffer.full);  // Liberar semáforo para desbloquear hilos

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	buffer_destroy(&buffer);
	pthread_mutex_destroy(&library.print_mutex);
	pthread_mutex_destroy(&library.ejecutando_mutex);

	return 0;
}
