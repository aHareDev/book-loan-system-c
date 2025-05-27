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

// Espera y muestra la respuesta recibida a través del pipe de respuesta
void waitResponse(char *pipeResponse, Response res);

// Muestra un menú interactivo para que el usuario ingrese una solicitud
void displayMenu(Request *reqPtr);

int main(int argc, char *argv[]) {
    	// Reservar memoria para los nombres de los pipes
	char *pipeReceptor = malloc(PIPE_NAME_MAX);
	char *pipeResponse = malloc(PIPE_NAME_MAX);
	char *requestFile = NULL;
	Request req;
	Response res;

    	// Validar argumentos de línea de comandos
	if (argc < 3) {
		printf("Uso: ./solicitante [-i requestfile] -p pipeReceptor\n");
		return 1;
	}

    	// Procesar argumentos - obtener nombre del pipe receptor y archivo de solicitudes
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

	int fd_write = open(pipeReceptor, O_WRONLY | O_NONBLOCK);
	if (fd_write < 0) {
		perror("\nError al abrir el pipe receptor");
		fprintf(stderr, "El servidor/receptor parece estar apagado. Intenta iniciarlo antes de ejecutar este programa.\n\n");
		return EXIT_FAILURE;
	}

	// Crear pipe de respuesta exclusivo para este proceso
	snprintf(pipeResponse, PIPE_NAME_MAX, "../ipc/response_%d", getpid());
	if (mkfifo(pipeResponse, 0666) == -1) {
		perror("Error al crear el pipe de respuesta");
		return 1;
	}

	// Enviar solicitudes desde archivo si se proporcionó
	if (requestFile) {
		FILE *file = fopen(requestFile, "r");
		if (!file) {
			perror("Error al abrir el archivo de solicitudes");
			return 1;
		}
		// Guardar el nombre del pipe de respuesta en la solicitud
		strncpy(req.pipeResponse, pipeResponse, PIPE_NAME_MAX);

		while (fscanf(file, "%c, %[^\n,], %d\n", &req.operation, req.title, &req.isbn) == 3) {
			req.pid = getpid();
			printf("- Solicitud: %c - %s - %d PID: %d\n", req.operation, req.title, req.isbn, req.pid);
			// Enviar solicitud por el pipe receptor
			if(write(fd_write, &req, sizeof(Request)) == -1 ) {
				perror("Error al enviar solicitud");
			}
			// Esperar respuesta excepto si la operación es 'Q' (salir)
			if (req.operation != 'Q') waitResponse(pipeResponse, res);
			printf("\n");
		}

		fclose(file);
	} else {
	        // Modo interactivo con menú para solicitudes
		while(1) {
			displayMenu(&req);
			req.pid = getpid();
		        // Enviar solicitud
			strncpy(req.pipeResponse, pipeResponse, PIPE_NAME_MAX);

			if(write(fd_write, &req, sizeof(Request)) == -1) {
				perror("Error al enviar solicitud");
			}
            		// Esperar respuesta del servidor
			waitResponse(pipeResponse, res);
			printf("\n");
		}
	}

	// Enviar señal de salida
	req.operation = 'Q';
	strcpy(req.title, "Salir");
	req.isbn = 0;
	req.pid = getpid();
	write(fd_write, &req, sizeof(Request));

    	// Cerrar pipe y liberar recursos
	close(fd_write);
	unlink(pipeResponse);
	free(pipeResponse);
	free(pipeReceptor);
	return 0;
}

void waitResponse(char *pipeResponse, Response res) {
	// Abrir pipe de respuesta para recibir la respuesta del servidor
	int fd_read = open(pipeResponse, O_RDONLY);
	if (fd_read < 0) {
		perror("Error al abrir pipe de respuesta");
	}

    	// Leer y mostrar respuesta recibida
	if (read(fd_read, &res, sizeof(Response)) > 0) {
		printf("\nCódigo de respuesta: %d\nContenido: %s\n", res.code, res.message);
	} else {
		printf("No se recibió respuesta o fue vacía.\n");
	}
}

void displayMenu(Request *reqPtr) {
	int opt;

	// Mostrar menú principal para seleccionar tipo de operación
	printf("\n");
       	printf(" ╔══════════════════════════════════════╗\n");
       	printf(" ║         📚 MENÚ DE BIBLIOTECA        ║\n");
       	printf(" ╠══════════════════════════════════════╣\n");
       	printf(" ║  1. 📤 Devolver un libro             ║\n");
	printf(" ║  2. 🔁 Renovar un libro              ║\n");
       	printf(" ║  3. 📥 Solicitar prestado un libro   ║\n");
       	printf(" ║  0. Salir                            ║\n");
       	printf(" ╚══════════════════════════════════════╝\n");

	// Validar opción ingresada por usuario
       	printf("\nOperación a realizar: ");
	while (scanf("%d", &opt) != 1 || opt < 0 || opt > 3) {
                printf("Opción inválida... Intenta de nuevo: ");
		while (getchar() != '\n');
	}
    	// Salir si opción 0
	if (opt == 0) {
		printf("\nSaliendo del programa...\n\n");
		exit(EXIT_SUCCESS);
	}

	// Asignar operación según opción seleccionada
       	reqPtr->operation = (opt == 1) ? 'D' : (opt == 2) ? 'R' : 'P';
	// Solicitar título del libro
    	printf("\n📄 Nombre del libro: ");
     	scanf(" %[^\n]", reqPtr->title);

    	// Solicitar ISBN y validar que sea un número válido
	printf("   ISBN: ");
	while (scanf("%d", &reqPtr->isbn) != 1) {
		printf("   ISBN inválido. Intenta de nuevo: ");
	    	while (getchar() != '\n');
	}

	// Mostrar resumen de la solicitud
	printf("\nSolicitud realizada\n");
	printf("Operación: %c  -  Libro: %s  -  ISBN: %d\n", reqPtr->operation, reqPtr->title, reqPtr->isbn);
}
