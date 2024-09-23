# Nombre del ejecutable
TARGET = keytester_launcher

# Compilador
CC = gcc

# Opciones del compilador
CFLAGS = -Wall

# Archivo fuente
SRC = keytester_launcher.c

# Archivo objeto
OBJ = $(SRC:.c=.o)

# Regla por defecto (compilar el binario)
all: $(TARGET)

# Regla para compilar el ejecutable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Regla para compilar los archivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos compilados
clean:
	rm -f $(OBJ) $(TARGET)
