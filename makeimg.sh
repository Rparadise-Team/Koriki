#!/bin/bash

# Verificar que estamos en ~/Koriki/
if [ "$(pwd)" != "$HOME/Koriki" ]; then
    echo "Este script debe ser ejecutado desde $HOME/Koriki/"
    exit 1
fi

# Variables
BASE_DIR="./base"
VERSION_FILE="$BASE_DIR/Koriki/version.txt"
CLUSTER_SIZE=32768  # 32 KB (32 * 1024)

# Leer la versión desde el archivo version.txt
if [ -f "$VERSION_FILE" ]; then
    VERSION=$(tr -d '\r\n' < "$VERSION_FILE")
    IMAGE_NAME="Koriki_v${VERSION}.img"
else
    echo "Error: No se pudo encontrar el archivo de versión en '$VERSION_FILE'."
    exit 1
fi

# Paso 1: Calcular el tamaño exacto de los archivos, incluyendo archivos ocultos
echo "Calculando el tamaño de los archivos en $BASE_DIR..."
SIZE_FILES=$(du -sb "$BASE_DIR" | awk '{print $1}')  # Incluye todos los archivos

# Paso 2: Usar 'ls' para contar archivos y carpetas, incluyendo archivos ocultos
NUM_FILES=$(ls -A -R "$BASE_DIR" | wc -l)
echo "Número total de archivos y carpetas (incluyendo ocultos): $NUM_FILES"

# Paso 3: Calcular el espacio adicional desperdiciado por clústeres
echo "Calculando la sobrecarga de clústeres (clúster de 32K)..."
OVERHEAD=0

# Iterar sobre cada archivo y calcular el espacio desperdiciado por archivo
find "$BASE_DIR" -type f | while read FILE; do
    FILE_SIZE=$(stat -c%s "$FILE")
    # Espacio desperdiciado en el clúster para este archivo
    OVERHEAD=$((OVERHEAD + (CLUSTER_SIZE - (FILE_SIZE % CLUSTER_SIZE)) % CLUSTER_SIZE))
done

# Paso 4: Calcular el número total de clústeres necesarios
CLUSTER_NEEDED=$((SIZE_FILES / CLUSTER_SIZE + NUM_FILES))  # Dividir el tamaño total por clúster y sumar por cada archivo
TOTAL_CLUSTER_SPACE=$((CLUSTER_NEEDED * CLUSTER_SIZE))      # Espacio total necesario para los clústeres en FAT32

# Mostrar los detalles
echo "Número de archivos y carpetas: $NUM_FILES"
echo "Espacio desperdiciado debido a los clústeres de FAT32: $OVERHEAD bytes"
echo "Tamaño total con sobrecarga de clústeres: $TOTAL_CLUSTER_SPACE bytes"

# Asegurarse de que el tamaño total sea múltiplo de 1 MiB para la creación de la imagen
SIZE_MB=$(( (TOTAL_CLUSTER_SPACE + 1048576 - 1) / 1048576 ))

echo "Versión: $VERSION"
echo "Nombre de la imagen: $IMAGE_NAME"
echo "Tamaño total ajustado a MiB: $SIZE_MB MiB"

# Paso 5: Crear un archivo vacío del tamaño calculado
echo "Creando un archivo de imagen vacío de $SIZE_MB MiB..."
dd if=/dev/zero of="$IMAGE_NAME" bs=1M count="$SIZE_MB" status=progress

# Paso 6: Crear una tabla de particiones y una partición FAT32
echo "Creando tabla de particiones y una partición FAT32..."
sudo parted -s "$IMAGE_NAME" mklabel msdos
sudo parted -s "$IMAGE_NAME" mkpart primary fat32 1MiB 100%

# Paso 7: Asociar la imagen a un dispositivo de bucle
echo "Asociando la imagen a un dispositivo de bucle..."
LOOP_DEVICE=$(sudo losetup -f --show "$IMAGE_NAME")

# Forzar la lectura de la tabla de particiones
sudo partprobe "$LOOP_DEVICE"

# Esperar a que el sistema reconozca la partición
sleep 1

# Buscar el dispositivo de la partición
echo "Buscando el dispositivo de la partición..."
if [ -e "${LOOP_DEVICE}p1" ]; then
    PARTITION_DEVICE="${LOOP_DEVICE}p1"
elif [ -e "/dev/mapper/$(basename "$LOOP_DEVICE")p1" ]; then
    PARTITION_DEVICE="/dev/mapper/$(basename "$LOOP_DEVICE")p1"
else
    echo "Error: No se pudo encontrar la partición en $LOOP_DEVICE"
    sudo losetup -d "$LOOP_DEVICE"
    exit 1
fi

# Paso 8: Formatear la partición con FAT32 usando clústeres de 32K
echo "Formateando la partición con FAT32 (clúster de 32K)..."
sudo mkfs.vfat -F 32 -s 64 -S 512 "$PARTITION_DEVICE"

# Paso 9: Montar la partición con opciones adecuadas
echo "Montando la partición en $MOUNT_POINT..."
MOUNT_POINT="./mnt_koriki"
mkdir -p "$MOUNT_POINT"
sudo mount -o utf8,shortname=mixed,codepage=437,iocharset=utf8 "$PARTITION_DEVICE" "$MOUNT_POINT"

# Paso 10: Copiar los archivos usando rsync
echo "Copiando archivos a la imagen..."
sudo rsync -a --no-owner --no-group "$BASE_DIR"/ "$MOUNT_POINT"/

# Paso 11: Desmontar y liberar el dispositivo de bucle
echo "Desmontando la partición y liberando el dispositivo de bucle..."
sudo umount "$MOUNT_POINT"
rm -r "$MOUNT_POINT"
sudo losetup -d "$LOOP_DEVICE"

# Paso 12: Comprimir la imagen
echo "Comprimiendo la imagen generada..."
gzip -f "$IMAGE_NAME"

echo "La imagen $IMAGE_NAME.gz está lista para ser flasheada."
