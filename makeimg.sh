#!/bin/bash

# Variables
BASE_DIR="./base"
VERSION_FILE="$BASE_DIR/Koriki/version.txt"
CLUSTER_SIZE=32768  # 32 KB (32 * 1024)
MIN_SIZE_MB=2048    # 2 GB en MiB
MARGIN_INCREMENT_MB=128  # Incremento si falta espacio
MAX_RETRIES=5

# Leer la versión desde el archivo version.txt
if [ -f "$VERSION_FILE" ]; then
    VERSION=$(tr -d '\r\n' < "$VERSION_FILE")
    IMAGE_NAME="Koriki_v${VERSION}.img"
else
    echo "Error: No se pudo encontrar el archivo de versión en '$VERSION_FILE'."
    exit 1
fi

# Función para limpiar en caso de error
cleanup() {
    if [ -n "$MOUNT_POINT" ] && mountpoint -q "$MOUNT_POINT" 2>/dev/null; then
        echo "Desmontando $MOUNT_POINT..."
        sudo umount "$MOUNT_POINT"
    fi
    if [ -n "$LOOP_DEVICE" ] && [ -e "$LOOP_DEVICE" ]; then
        echo "Liberando dispositivo de bucle $LOOP_DEVICE..."
        sudo losetup -d "$LOOP_DEVICE"
    fi
    if [ -d "$MOUNT_POINT" ]; then
        rm -rf "$MOUNT_POINT"
    fi
}

# Trap para limpiar en caso de error
trap cleanup EXIT

# Paso 1: Calcular el tamaño exacto de los archivos
echo "Calculando el tamaño de los archivos en $BASE_DIR..."
SIZE_FILES=$(du -sb "$BASE_DIR" | awk '{print $1}')

# Paso 2: Contar archivos y directorios
NUM_FILES=$(find "$BASE_DIR" -type f | wc -l)
NUM_DIRS=$(find "$BASE_DIR" -type d | wc -l)
echo "Número de archivos: $NUM_FILES"
echo "Número de directorios: $NUM_DIRS"

# Paso 3: Calcular slack por clúster
echo "Calculando la sobrecarga de clústeres (clúster de 32K)..."
OVERHEAD=0
while read -r FILE; do
    FILE_SIZE=$(stat -c%s "$FILE")
    ((OVERHEAD += (CLUSTER_SIZE - (FILE_SIZE % CLUSTER_SIZE)) % CLUSTER_SIZE))
done < <(find "$BASE_DIR" -type f)

# Paso 4: Calcular espacio para directorios
# Cada entrada de directorio ocupa 32 bytes, cada directorio necesita al menos 1 cluster
DIR_ENTRIES_BYTES=$((NUM_FILES * 32 + NUM_DIRS * 32))
DIR_CLUSTERS=$(( (DIR_ENTRIES_BYTES + CLUSTER_SIZE - 1) / CLUSTER_SIZE ))
DIR_OVERHEAD=$((DIR_CLUSTERS * CLUSTER_SIZE - DIR_ENTRIES_BYTES))

DATA_BYTES=$((SIZE_FILES + OVERHEAD + DIR_ENTRIES_BYTES + DIR_OVERHEAD))

# Paso 5: Calcular overhead de FAT32 con margen
SECTOR_SIZE=512
RESERVED_SECTORS=32
RESERVED_BYTES=$((RESERVED_SECTORS * SECTOR_SIZE))

# Calcular clusters totales estimados (con factor de seguridad)
CLUSTERS=$(( (DATA_BYTES + CLUSTER_SIZE - 1) / CLUSTER_SIZE ))
CLUSTERS=$((CLUSTERS * 110 / 100))  # +10% de seguridad

# Calcular tamaño de FAT
FAT_ENTRIES=$((CLUSTERS + 2))
SECTORS_PER_FAT=$(( (FAT_ENTRIES * 4 + SECTOR_SIZE - 1) / SECTOR_SIZE ))
FAT_BYTES=$((2 * SECTORS_PER_FAT * SECTOR_SIZE))

TOTAL_BYTES=$((DATA_BYTES + RESERVED_BYTES + FAT_BYTES))

echo "Tamaño de archivos: $SIZE_FILES bytes"
echo "Espacio desperdiciado por clúster (archivos): $OVERHEAD bytes"
echo "Overhead de directorios: $((DIR_ENTRIES_BYTES + DIR_OVERHEAD)) bytes"
echo "Overhead FAT32: reservados=$RESERVED_BYTES bytes, FAT=$FAT_BYTES bytes"
echo "Tamaño total estimado: $TOTAL_BYTES bytes"

# Margen de seguridad más generoso
MARGIN_BYTES=$((128 * 1024 * 1024))  # 128 MiB fijos
SIZE_MB=$(( (TOTAL_BYTES + MARGIN_BYTES + 1048576 - 1) / 1048576 ))

# Enforce tamaño mínimo
if [ $SIZE_MB -lt $MIN_SIZE_MB ]; then
    SIZE_MB=$MIN_SIZE_MB
fi

echo "Versión: $VERSION"
echo "Nombre de la imagen: $IMAGE_NAME"
echo "Tamaño inicial calculado: $SIZE_MB MiB"

# Bucle de creación con verificación
MOUNT_POINT="./mnt_koriki"
RETRY_COUNT=0

while [ $RETRY_COUNT -lt $MAX_RETRIES ]; do
    echo ""
    echo "============================================"
    echo "Intento $((RETRY_COUNT + 1)) de $MAX_RETRIES - Tamaño: $SIZE_MB MiB"
    echo "============================================"
    
    # Limpiar imagen anterior si existe
    if [ -f "$IMAGE_NAME" ]; then
        rm -f "$IMAGE_NAME"
    fi
    
    # Crear imagen
    echo "Creando un archivo de imagen vacío de $SIZE_MB MiB..."
    dd if=/dev/zero of="$IMAGE_NAME" bs=1M count="$SIZE_MB" status=progress
    
    # Crear partición
    echo "Creando tabla de particiones y una partición FAT32..."
    sudo parted -s "$IMAGE_NAME" mklabel msdos
    sudo parted -s "$IMAGE_NAME" mkpart primary fat32 2048s 100%
    sudo parted -s "$IMAGE_NAME" set 1 boot on
    
    # Asociar a loop device
    echo "Asociando la imagen a un dispositivo de bucle..."
    LOOP_DEVICE=$(sudo losetup -f --show "$IMAGE_NAME")
    sudo partprobe "$LOOP_DEVICE"
    sleep 1
    
    # Buscar partición
    if [ -e "${LOOP_DEVICE}p1" ]; then
        PARTITION_DEVICE="${LOOP_DEVICE}p1"
    elif [ -e "/dev/mapper/$(basename "$LOOP_DEVICE")p1" ]; then
        PARTITION_DEVICE="/dev/mapper/$(basename "$LOOP_DEVICE")p1"
    else
        echo "Error: No se pudo encontrar la partición en $LOOP_DEVICE"
        sudo losetup -d "$LOOP_DEVICE"
        exit 1
    fi
    
    # Formatear
    echo "Formateando la partición con FAT32 (clúster de 32K)..."
    sudo mkfs.vfat -F 32 -s 64 -S 512 "$PARTITION_DEVICE" 2>&1 | tee /tmp/mkfs_output.log
    
    # Verificar advertencias de mkfs
    if grep -q "less then suggested minimum" /tmp/mkfs_output.log; then
        echo "ADVERTENCIA: Número de clusters insuficiente para FAT32"
        SIZE_MB=$((SIZE_MB + 128))
        sudo losetup -d "$LOOP_DEVICE"
        ((RETRY_COUNT++))
        continue
    fi
    
    # Montar
    echo "Montando la partición en $MOUNT_POINT..."
    mkdir -p "$MOUNT_POINT"
    sudo mount -o utf8,shortname=mixed,codepage=437,iocharset=utf8 "$PARTITION_DEVICE" "$MOUNT_POINT"
    
    # Verificar espacio disponible
    echo ""
    echo "Espacio en la partición montada:"
    df -h "$MOUNT_POINT"
    echo ""
    
    AVAILABLE_KB=$(df --output=avail "$MOUNT_POINT" | tail -1)
    AVAILABLE_BYTES=$((AVAILABLE_KB * 1024))
    NEEDED_BYTES=$((SIZE_FILES + OVERHEAD + DIR_ENTRIES_BYTES + DIR_OVERHEAD))
    
    # Añadir 10% de margen extra sobre lo necesario
    NEEDED_BYTES_WITH_MARGIN=$((NEEDED_BYTES * 110 / 100))
    
    echo "Espacio disponible: $((AVAILABLE_BYTES / 1024 / 1024)) MiB"
    echo "Espacio necesario (con margen): $((NEEDED_BYTES_WITH_MARGIN / 1024 / 1024)) MiB"
    
    if [ $AVAILABLE_BYTES -lt $NEEDED_BYTES_WITH_MARGIN ]; then
        echo "INSUFICIENTE: Faltan aproximadamente $(( (NEEDED_BYTES_WITH_MARGIN - AVAILABLE_BYTES) / 1024 / 1024 )) MiB"
        sudo umount "$MOUNT_POINT"
        sudo losetup -d "$LOOP_DEVICE"
        SIZE_MB=$((SIZE_MB + MARGIN_INCREMENT_MB))
        ((RETRY_COUNT++))
        continue
    fi
    
    echo "Espacio suficiente confirmado. Procediendo a copiar archivos..."
    
    # Copiar archivos
    echo "Copiando archivos a la imagen..."
    if sudo rsync -a --no-owner --no-group --exclude="RESIZED" "$BASE_DIR"/ "$MOUNT_POINT"/; then
        echo "Copia completada exitosamente."
        
        # Mostrar espacio final
        echo ""
        echo "Espacio final después de la copia:"
        df -h "$MOUNT_POINT"
        echo ""
        
        # Desmontar
        echo "Desmontando la partición y liberando el dispositivo de bucle..."
        sudo umount "$MOUNT_POINT"
        rm -rf "$MOUNT_POINT"
        sync
        sudo losetup -d "$LOOP_DEVICE"
        
        # Limpiar trap
        trap - EXIT
        
        # Comprimir
        echo "Comprimiendo la imagen generada..."
        gzip -f "$IMAGE_NAME"
        
        echo ""
        echo "============================================"
        echo "ÉXITO: La imagen $IMAGE_NAME.gz está lista"
        echo "============================================"
        exit 0
    else
        echo "Error al copiar archivos. Incrementando tamaño..."
        sudo umount "$MOUNT_POINT"
        sudo losetup -d "$LOOP_DEVICE"
        SIZE_MB=$((SIZE_MB + MARGIN_INCREMENT_MB))
        ((RETRY_COUNT++))
    fi
done

echo "Error: Se alcanzó el número máximo de reintentos"
exit 1
