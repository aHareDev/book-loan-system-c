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

	// Parsear argumentos
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-p") == 0 && ++i < argc) pipeReceptor = argv[i];
		else if (strcmp(argv[i], "-f") == 0 && ++i < argc) fileData = argv[i];
		else if (strcmp(argv[i], "-v") == 0) verbose = 1;
		else if (strcmp(argv[i], "-s") == 0 && ++i < argc) fileOutput = argv[i];
	}

	// Buscar pipeReceptor en la carpeta ../ipc/
	struct stat st;
	if (stat(pipeReceptor, &st) != 0) {
	        snprintf(pipeReceptor, PIPE_NAME_SIZE, "../ipc/pipeReceptor");
		if (mkfifo(pipeReceptor, 0666) != 0) {
			perror("Error al crear el pipe receptor de solicitudes\n");
			exit(1);
		}
	} else if (!S_ISFIFO(st.st_mode)) {
		fprintf(stderr, "Error: %s existe pero no es un pipe\n", pipeReceptor);
		exit(1);
	}

	Buffer buffer;
	buffer_init(&buffer);
	Library library = { .buf = &buffer, .ejecutando = &ejecutando, .file = fileData };

	printf("\n\n");
	if (!loadLibraryFromFile(&library, fileData)) {
		fprintf(stderr, "Error cargando la base de datos de libros.\n");
		return 0;
	}

	printf("=== Servidor esperando solicitudes en '%s'... ===\n", pipeReceptor);

	pthread_t thread1, thread2;
	pthread_create(&thread1, NULL, auxiliar1, &library);
	pthread_create(&thread2, NULL, auxiliar2, &library);

	int fd_read = open(pipeReceptor, O_RDONLY);
	if (fd_read < 0) {
		perror("Error al abrir el pipe receptor");
		exit(1);
	}

	while(ejecutando) {
		if (readRequest(fd_read, &req)) {
			mostrarSolicitud(&req);
			if (req.operation == 'D' || req.operation == 'R') {
				if (sendResponse(&req)) {
					buffer_insert(&buffer, &req);
				} else {
					perror("Error al escribir en el pipe");
				}
			} else if (req.operation == 'P') {
				printf("Actualiza base de datos\n");
			} else if (req.operation == 'Q') {
				printf("Cliente %d ha terminado sus solicitudes.\n", req.pid);
			}
		}
	}

	close(fd_read);
	free(pipeReceptor);

	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);

	buffer_destroy(&buffer);

	return 0;
}
