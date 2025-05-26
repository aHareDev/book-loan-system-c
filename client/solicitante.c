/** 
  * solicitante.c
  *
  * Proyecto: Sistema para el Préstamo de Libros
  * Curso: Sistemas Operativos
  * Universidad: Pontificia Univerdidad Javeriana
  * Fecha: 20 Mayo 2025
  *
  * Descripción: Lógica del Proceso Solicitante(PS). Lee de archivo o menú las operaciones y envía la solicitud usando Pipes.
  *
  * Autor: Arley Bernal, Sergio Pardo
**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "entities.h"

void waitResponse(char *pipeResponse, Response res);
void displayMenu(Request *reqPtr);

int main(int argc, char *argv[]) {
	char *pipeReceptor = malloc(PIPE_NAME_MAX);
	char *pipeResponse = malloc(PIPE_NAME_MAX);
	char *requestFile = NULL;
	Request req;
	Response res;

	if (argc < 3) {
		printf("Uso: ./solicitante [-i requestfile] -p pipeReceptor\n");
		return 1;
	}

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
			requestFile = argv[++i];
		if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
			strncpy(pipeReceptor, argv[++i], PIPE_NAME_MAX);
	}

	if (pipeReceptor == NULL) {
	        fprintf(stderr, "Error: Debes proporcionar el nombre del pipe receptor con -p\n");
	        return 1;
	}

	// Abrir pipe del receptor
	int fd_write = open(pipeReceptor, O_WRONLY);
	if (fd_write < 0) {
		perror("Error al abrir el pipe receptor");
		return EXIT_FAILURE;
	}

	// Crear pipe de respuesta
	snprintf(pipeResponse, PIPE_NAME_MAX, "../ipc/response_%d", getpid());
	if (mkfifo(pipeResponse, 0666) == -1) {
		perror("Error al crear el pipe de respuesta");
		return 1;
	}

	// Si se ha proporcionado nombre de archivo de entrada
	if (requestFile) {
		FILE *file = fopen(requestFile, "r");
		if (!file) {
			perror("Error al abrir el archivo de solicitudes");
			return 1;
		}
		strncpy(req.pipeResponse, pipeResponse, PIPE_NAME_MAX);

		while (fscanf(file, "%c, %[^\n,], %d\n", &req.operation, req.title, &req.isbn) == 3) {
			req.pid = getpid();
			printf("- Solicitud: %c - %s - %d PID: %d\n", req.operation, req.title, req.isbn, req.pid);
			if(write(fd_write, &req, sizeof(Request)) == -1 ) {
				perror("Error al enviar solicitud");
			}
			if (req.operation != 'Q') waitResponse(pipeResponse, res);
			printf("\n");
		}

		fclose(file);
	} else {
		while(1) {
			displayMenu(&req);
			req.pid = getpid();

			strncpy(req.pipeResponse, pipeResponse, PIPE_NAME_MAX);
			if(write(fd_write, &req, sizeof(Request)) == -1) {
				perror("Error al enviar solicitud");
			}
			waitResponse(pipeResponse, res);
			printf("\n");
		} while(1);
	}

	req.operation = 'Q';
	strcpy(req.title, "Salir");
	req.isbn = 0;
	req.pid = getpid();
	write(fd_write, &req, sizeof(Request));

	close(fd_write);

	free(pipeResponse);
	free(pipeReceptor);

	return 0;
}

void waitResponse(char *pipeResponse, Response res) {
	 // Recibir respuesta
	int fd_read = open(pipeResponse, O_RDONLY);
	if (fd_read < 0) {
		perror("Error al abrir pipe de respuesta");
	}

	if (read(fd_read, &res, sizeof(Response)) > 0) {
		printf("\nCódigo de respuesta: %d\nContenido: %s\n", res.code, res.message);
	} else {
		printf("No se recibió respuesta o fue vacía.\n");
	}
}

void displayMenu(Request *reqPtr) {
	int opt;

	printf("\n");
       	printf(" ╔══════════════════════════════════════╗\n");
       	printf(" ║         📚 MENÚ DE BIBLIOTECA        ║\n");
       	printf(" ╠══════════════════════════════════════╣\n");
       	printf(" ║  1. 📤 Devolver un libro             ║\n");
	printf(" ║  2. 🔁 Renovar un libro              ║\n");
       	printf(" ║  3. 📥 Solicitar prestado un libro   ║\n");
       	printf(" ║  0. Salir                            ║\n");
       	printf(" ╚══════════════════════════════════════╝\n");

       	printf("\nOperación a realizar: ");
	while (scanf("%d", &opt) != 1 || opt < 0 || opt > 3) {
                printf("Opción inválida... Intenta de nuevo: ");
		while (getchar() != '\n');
	}

	if (opt == 0) {
		printf("\nSaliendo del programa...\n\n");
		exit(EXIT_SUCCESS);
	}

       	reqPtr->operation = (opt == 1) ? 'D' : (opt == 2) ? 'R' : 'P';

    	printf("\n📄 Nombre del libro: ");
     	scanf(" %[^\n]", reqPtr->title);

	printf("   ISBN: ");
	while (scanf("%d", &reqPtr->isbn) != 1) {
		printf("   ISBN inválido. Intenta de nuevo: ");
	    	while (getchar() != '\n');
	}

	// Mostrar resumen de la solicitud
	printf("\nSolicitud realizada\n");
	printf("Operación: %c  -  Libro: %s  -  ISBN: %d\n", reqPtr->operation, reqPtr->title, reqPtr->isbn);
}
