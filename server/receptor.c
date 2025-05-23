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

int main(int argc, char *argv[]) {
	char *pipeName = NULL, *dataFile = NULL, *outputFile = NULL;

	int verbose = 0;

	// Parsear argumentos
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
			pipeName = argv[++i];
		else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc)
			dataFile = argv[++i];
		else if (strcmp(argv[i], "-v") == 0)
			verbose = 1;
		else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc)
			outputFile = argv[++i];
	}

	if (!pipeName || !dataFile) {
		fprintf(stderr, "Uso: receptor -p pipe -f dataFile [-v] [-s outputFile]\n");
		exit(0);
	}

	struct stat st;
	if (stat(pipeName, &st) != 0) {
		if (mkfifo(pipeName, 0666) != 0) {
			perror("Error al crear el pipe receptor de solicitudes.\n");
			exit(0);
		}
	} else if (!S_ISFIFO(st.st_mode)) {
	    	fprintf(stderr, "Error: %s existe pero no es un pipe\n", pipeName);
		exit(0);
	}

	int pipe_fd = open(pipeName, O_RDONLY);
	if (pipe_fd < 0) {
        	perror("Error al abrir el pipe para lectura");
	        exit(EXIT_FAILURE);
	}


	return 0;
}
