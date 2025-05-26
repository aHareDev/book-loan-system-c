/**
  * receptor.c
  *
  * Proyecto: Sistema para el Préstamo de Libros
  * Curso: Sistemas Operativos
  * Universidad: Pontificia Universidad Javeriana
  * Fecha: 20 Mayo 2025
  *
  * Descripción:
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

void *auxiliar1(void *arg);
void *auxiliar2(void *arg);

int main(int argc, char *argv[]) {
	char *pipeReceptor = malloc(PIPE_NAME_SIZE), *fileData = NULL, *fileOutput = NULL;
	Request req;

	int verbose = 0;
	int ejecutando = 1;

	if (argc < 4) {
		fprintf(stderr, "Uso correcto: ./receptor -p pipeReceptor -f filedatos [-v] [-s filesalida]\n");
		return 1;
	}
	printf("\n🔧 Iniciando servidor ...\n");

	// Parsear argumentos
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


	printf("\nPipe receptor: %s\n", pipeReceptor);
	printf("Archivo base de datos: %s\n", fileData ? fileData : "(No especificado)");
	if (fileOutput) printf("Archivo de salida: %s\n", fileOutput);
	if (verbose)    printf("Modo verbose activado\n");

	// Buscar pipeReceptor en la carpeta ../ipc/
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

	Buffer buffer;
	buffer_init(&buffer);

	Library library = {
	    .buf = &buffer,
	    .ejecutando = &ejecutando
	};

	strncpy(library.fileOutput, fileOutput, MAX_CHARACTERS - 1);
	library.fileOutput[MAX_CHARACTERS - 1] = '\0';

	pthread_mutex_init(&library.ejecutando_mutex, NULL);
	pthread_mutex_init(&library.print_mutex, NULL);
	library.redibujar_menu = 0;
	pthread_mutex_init(&library.redibujar_mutex, NULL);

	int fd_read = open(pipeReceptor, O_NONBLOCK);
	if (fd_read < 0) {
		perror("Error al abrir el pipe receptor");
		exit(1);
	}

	if (!loadLibraryFromFile(&library, fileData)) {
		perror("Error al cargar la base de datos");
		return 0;
	}
	printf("Base de datos cargada correctamente.: '%s'\n", fileData);

	printf("\n\n\n");
	printf("➤ Servidor esperando solicitudes ...\n");

	pthread_t thread1, thread2;
	pthread_create(&thread1, NULL, auxiliar1, &library);
	pthread_create(&thread2, NULL, auxiliar2, &library);

	while(ejecutando) {
		if (readRequest(fd_read, &req)) {
			if (verbose) showRequest(&req);

			if (req.operation == 'D' || req.operation == 'R') {
				char *msg = (req.operation == 'D') ? "✅ La biblioteca está recibiendo el libro."
							: "✅ Renovación aceptada. Nueva fecha de entrega en 7 días.";

				if (sendResponse(&req, msg)) {
					buffer_insert(&buffer, &req); // Solo inserta si se pudo responder al cliente
				} else {
					fprintf(stderr, "No se pudo enviar la respuesta al cliente %d\n", req.pid);
				}
			} else if (req.operation == 'P') {
				Book *book = library_findBookByISBN(&library, req.isbn);
				Copy *copy = library_findAvailableCopy(&library, req.isbn);
				if (copy && copy->status == 'D') {
					sendResponse(&req, "✅ Solicitud aceptada. Realizando actualización de prestamo.");
					copy->status = 'P';
					time_t t = time(NULL);
					struct tm *now = localtime(&t);
					snprintf(copy->date, sizeof(copy->date), "%02d-%02d-%04d", 
						now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);

	                                library_addReport(&library, 'P', book->title, book->isbn, copy->id, copy->date);
				} else {
					sendResponse(&req, "✅ Solicitud Rechazada. El libro solicitado no tiene copias disponibles.");
				}
			} else if (req.operation == 'Q') {
				printf("\n\n");
				printf("\nCliente %d ha terminado sus solicitudes.\n", req.pid);

				if (fileOutput != NULL) {
					if (saveLibraryToFile(&library, fileOutput)) {
						printf("📁 Base de datos guardada exitosamente en '%s'\n\n", fileOutput);
					} else {
						fprintf(stderr, "Error al guardar la base de datos en '%s'\n", fileOutput);
					}
				}
			}
		}
	}
	printf("\n\n");
        close(fd_read);
	free(pipeReceptor);
	pipeReceptor = NULL;
	sem_post(&buffer.full);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	buffer_destroy(&buffer);
	pthread_mutex_destroy(&library.print_mutex);
	pthread_mutex_destroy(&library.ejecutando_mutex);

	return 0;
}
