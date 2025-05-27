# Sistema para el Préstamo de Libros
## Descripción General

Este proyecto consiste en el desarrollo de un sistema distribuido para gestionar el préstamo de libros en una biblioteca, permitiendo que múltiples usuarios realicen operaciones de manera concurrente mediante procesos que se comunican con un servidor central a través de pipes. El servidor procesa préstamos directamente y delega devoluciones y renovaciones a un hilo auxiliar mediante un buffer sincronizado, mientras un segundo hilo administra comandos de consola. El sistema garantiza la sincronización de acceso a los recursos compartidos y persiste el estado de los libros mediante archivos estructurados.

## Objetivos

- Desarrollar un sistema distribuido para la gestión de préstamos bibliográficos, que permita la interacción simultánea de múltiples usuarios mediante procesos independientes, utilizando comunicación interprocesos basada en pipes para enviar solicitudes de préstamo, devolución y renovación a un servidor central.

- Implementar mecanismos de sincronización eficientes mediante semáforos y mutex POSIX, que garanticen el acceso concurrente seguro a estructuras compartidas como el buffer de solicitudes y el archivo que actúa como base de datos, previniendo condiciones de carrera y garantizando la integridad de la información.

- Estructurar la lógica del servidor a través de multihilo, delegando la atención de operaciones diferidas (como devoluciones y renovaciones) a un hilo auxiliar mediante un patrón productor-consumidor, y habilitando un segundo hilo para gestionar comandos administrativos en tiempo de ejecución sin interrumpir el procesamiento principal.

- Garantizar la persistencia y trazabilidad del sistema, mediante el manejo de archivos estructurados para la carga de datos iniciales, el almacenamiento del estado final de la biblioteca, y la generación de reportes detallados de las operaciones ejecutadas, facilitando la verificación y análisis del funcionamiento del sistema.

## Estructura del Proyecto
### Carpeta client/
Contiene el código del proceso solicitante (PS), encargado de obtener peticiones de usuarios mediante consola
o archivos de instrucciones y enviarlas al servidor.

- `solicitante.c`: Código fuente del proceso solicitante (PS). Crea solicitudes y las envía por pipe al servidor.
- `file.txt`: Archivo de entrada conformado por una lista de solicitudes. Cada línea representa una petición.
- `entities.h`: Definición de estructuras compartidas.
- `Makefile`: Contiene instrucciones para compilar y ejecutar automáticamente los archivos del programa solicitante.

### Carpeta server/
- `receptor.c`: Contiene la lógica principal del proceso receptor (PR), incluyendo el procesamiento de solicitudes y creación de hilos.
- `filedatos.txt`: Base de datos inicial de libros de la biblioteca, con información de ejemplares, estado y fechas.
- `filesalida.txt`: Guarda la base de datos modificada por el programa.
- `auxiliar1.c`: Implementación del hilo que atiende solicitudes de devolución y renovación (consumidor del buffer).
- `auxiliar2.c`: Implementación del hilo encargado de leer comandos desde consola (r para reporte, s para salir).
- `buffer.c`: Implementación del buffer tipo productor-consumidor, con funciones para insertar y extraer solicitudes.
- `buffer.h`: Declaraciones de las estructuras y funciones utilizadas para el manejo del buffer compartido.
- `entities.c`: Implementación de funciones aplicadas a las entidades.
- `entities.h`: Definición de estructuras de datos clave como libros, ejemplares, y solicitudes.
- `utils.c`: Funciones auxiliares comunes: parsing de archivos, lectura y escritura de la BD, etc.
- `utils.h`: Declaración de funciones auxiliares y utilitarias utilizadas en todo el sistema.
- `Makefile`: Contiene instrucciones para compilar y ejecutar automáticamente los archivos del programa receptor.

## Compilación y Ejecución
### Compilar
- Código de Receptor (RP).
```bash
make
```
Este comando compila el ejecutable receptor utilizando todos los archivos fuente relacionados (receptor.c, buffer.c, utils.c, auxiliar1.c, auxiliar2.c)
y las librerías necesarias (-pthread).
```bash
gcc -Wall -pthread receptor.c buffer.c entities.c utils.c auxiliar1.c auxiliar2.c -o receptor
```

- Código del Solicitante (PS).
```bash
make
```
Esto compila el archivo fuente solicitante.c para generar el ejecutable solicitante.
```bash
gcc -Wall solicitante.c -o solicitante
```

### Ejecutar Procesos
- Proceso Receptor.
El ejecutable receptor puede ejecutarse de las siguientes formas:
```bash
./receptor -p ../ipc/pipeReceptor -f filedatos.txt
```

Opciones adicionales:
- -v → Modo verbose: imprime todas las solicitudes en consola.
- -s filesalida.txt → Archivo de salida con el estado final de la base de datos.

Ejemplo:
```bash
./receptor -p ../ipc/pipeReceptor -f filedatos.txt -v -s filesalida.txt
```

También se puede usar los siguientes comandos con el Makefile:
```bash
make run           # Ejecuta RP con archivo base
make runv          # Ejecuta RP con verbose
make runfile       # Ejecuta RP y guarda resultado en archivo
make runfilev      # Ejecuta RP con verbose y guarda resultado
```

- Proceso Solicitante.
El ejecutable solicitante puede ejecutarse de dos formas.

Modo interactivo
```bash
./solicitante -p ../ipc/pipeReceptor
```

Modo por arhivo
```bash
./solicitante -i entrada.txt -p ../ipc/pipeReceptor
```

Tambíen se puede usar los siguientes comandos con el Makefile:
```bash
make run           # Ejecuta PS en modo interactivo
make runfile       # Ejecuta PS con archivo de entrada
```

### Limpiar
Para eliminar todos los ejecutables generados:

```bash
make clean
```
