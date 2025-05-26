# Variables
CC = gcc
CFLAGS = -Wall
SRC = solicitante.c
OUT = solicitante
PIPE_PATH = ../ipc/pipeReceptor
INPUT_FILE = file.txt

# Objectivo por defecto
all: $(OUT)

# Cómo compilar
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

# Ejecutar con el pipe (sin archivo de entrada)
run:
	./$(OUT) -p $(PIPE_PATH)

# Ejecutar con el pipe y el archivo de solicitudes
runfile:
	./$(OUT) -i $(INPUT_FILE) -p $(PIPE_PATH)

# Limpiar archivos generales
clean:
	rm -f $(OUT)
