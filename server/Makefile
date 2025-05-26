# Variables
CC = gcc
CFLAGS = -Wall -pthread

SRC = receptor.c buffer.c utils.c auxiliar1.c auxiliar2.c
OUT = receptor
PIPE_PATH = ../ipc/pipeReceptor
DATA_FILE = filedatos.txt
OUTPUT_FILE = filesalida.txt

# Objectivo por defecto
all: $(OUT)

# Cómo compilar
$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)

# Ejecutar con el pipe y el archivo de base de datos
run:
	./$(OUT) -p $(PIPE_PATH) -f $(DATA_FILE)

# Ejecutar con el pipe, el archivo de base de datos y el archivo para el informe
runfile:
	./$(OUT) -p $(PIPE_PATH) -f $(DATA_FILE) -s $(OUTPUT_FILE)

# Ejecutar con el pipe y el archivo de base de datos
runv:
	./$(OUT) -p $(PIPE_PATH) -f $(DATA_FILE) -v

# Ejecutar con el pipe, el archivo de base de datos y el archivo para el informe
runfilev:
	./$(OUT) -p $(PIPE_PATH) -f $(DATA_FILE) -v -s $(OUTPUT_FILE)

# Limpiar archivos generales
clean:
	rm -f $(OUT)
