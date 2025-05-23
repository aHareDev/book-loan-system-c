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

int main(int argc, char *argv[]) {
	char *pipeName = NULL;
	char *fileName = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0 && i + 1 < argc)
			fileName = argv[++i];
		if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
			pipeName = argv[++i];
	}

	int fd = open(pipeName, O_WRONLY);
	if (fd < 0) {
		perror("Open pipe");
		exit(1);
	}

	if (fileName) {
		FILE *f = fopen(fileName, "r");
		if (!f) {
			perror("fopen");
			return 1;
		}
	}

	return 0;
}
