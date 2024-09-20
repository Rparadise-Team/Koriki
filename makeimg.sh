#!/bin/bash

# Verificar que estamos en ~/Koriki/
if [ "$(pwd)" != "$HOME/Koriki" ]; then
    echo "Este script debe ser ejecutado desde $HOME/Koriki/"
    exit 1
fi

# Variables
BASE_DIR="./base"
VERSION_FILE="$BASE_DIR/Koriki/version.txt"

# Leer la versión desde el archivo version.txt
if [ -f "$VERSION_FILE" ]; then
    VERSION=$(tr -d '\r\n' < "$VERSION_FILE")
    IMAGE_NAME="Koriki_v${VERSION}.img"
else
    echo "Error: No se pudo encontrar el archivo de versión en '$VERSION_FILE'."
    exit 1
fi

# Paso 1: Calcular el tamaño necesario
SIZE_FILES=$(du -sb "$BASE_DIR" | awk '{print $1}')
SIZE_ADDITIONAL=$((SIZE_FILES / 5))  # Aumentar el tamaño adicional al 20%
SIZE_TOTAL=$((SIZE_FILES + SIZE_ADDITIONAL))

# Asegurarse de que el tamaño total sea múltiplo de 1 MiB
SIZE_MB=$(( (SIZE_TOTAL + 1048576 - 1) / 1048576 ))

echo "Versión: $VERSION"
echo "Nombre de la imagen: $IMAGE_NAME"
echo "Tamaño de los archivos: $SIZE_FILES bytes"
echo "Tamaño adicional: $SIZE_ADDITIONAL bytes"
echo "Tamaño total: $SIZE_TOTAL bytes"
echo "Creando imagen de $SIZE_MB MiB"

# Paso 2: Crear un archivo vacío del tamaño calculado
dd if=/dev/zero of="$IMAGE_NAME" bs=1M count="$SIZE_MB" status=progress

# Paso 3: Crear una tabla de particiones y una partición con parted
sudo parted -s "$IMAGE_NAME" mklabel msdos
sudo parted -s "$IMAGE_NAME" mkpart primary fat32 1MiB 100%

# Paso 4: Asociar la imagen a un dispositivo de bucle
LOOP_DEVICE=$(sudo losetup -f --show "$IMAGE_NAME")

# Forzar la lectura de la tabla de particiones
sudo partprobe "$LOOP_DEVICE"

# Esperar a que el sistema reconozca la partición
sleep 1

# Encontrar el dispositivo de la partición
if [ -e "${LOOP_DEVICE}p1" ]; then
    PARTITION_DEVICE="${LOOP_DEVICE}p1"
elif [ -e "/dev/mapper/$(basename "$LOOP_DEVICE")p1" ]; then
    PARTITION_DEVICE="/dev/mapper/$(basename "$LOOP_DEVICE")p1"
else
    echo "Error: No se pudo encontrar la partición en $LOOP_DEVICE"
    sudo losetup -d "$LOOP_DEVICE"
    exit 1
fi

# Paso 5: Formatear la partición con FAT32 y clúster de 32K
sudo mkfs.vfat -F 32 -s 64 -S 512 "$PARTITION_DEVICE"

# Paso 6: Montar la partición con opciones adecuadas
MOUNT_POINT="./mnt_koriki"
mkdir -p "$MOUNT_POINT"
sudo mount -o utf8,shortname=mixed,codepage=437,iocharset=utf8 "$PARTITION_DEVICE" "$MOUNT_POINT"

# Paso 7: Copiar los archivos usando rsync
sudo rsync -a --no-owner --no-group "$BASE_DIR"/ "$MOUNT_POINT"/

# Paso 8: Desmontar y liberar
sudo umount "$MOUNT_POINT"
rm -r "$MOUNT_POINT"
sudo losetup -d "$LOOP_DEVICE"

# Paso 9: Comprimir la imagen
gzip -f "$IMAGE_NAME"

echo "La imagen $IMAGE_NAME.gz está lista para ser flasheada."
