#!/bin/bash

# Versión con hash MD5 para máxima precisión
# Extrae SOLO los cambios entre ANTIGUO (153) y NUEVO (base) a DIFERENCIAL (160)
# Genera archivo .deletes con lista de eliminados con rutas completas
# Crea zip con nombre update_koriki_v{VERSION}.zip

ANTIGUO="$1"      # 153 - estado previo
NUEVO="$2"        # base - estado actual
DIFERENCIAL="$3"  # 160 - aquí irán solo los cambios
VERSION="$4"      # Versión para el nombre del zip
RUTA_PREFIX="$5"  # Prefijo de ruta para .deletes (por defecto /mnt/SDCARD/)

# Colores para salida
VERDE='\033[0;32m'
ROJO='\033[0;31m'
AMARILLO='\033[1;33m'
AZUL='\033[0;34m'
NC='\033[0m'

# Si no se proporciona prefijo, usar el valor por defecto
if [ -z "$RUTA_PREFIX" ]; then
    RUTA_PREFIX="/mnt/SDCARD/"
fi

# Función para mostrar errores
mostrar_error() {
    echo -e "${ROJO}Error: $1${NC}"
    exit 1
}

# Verificar argumentos
if [ $# -lt 3 ]; then
    echo -e "${AMARILLO}Uso: $0 <dir_antiguo_153> <dir_nuevo_base> <dir_diferencial_160> [VERSION] [RUTA_PREFIX]${NC}"
    echo -e "${AMARILLO}Ejemplo: $0 ./153 ./base ./160 1.6 /mnt/SDCARD/${NC}"
    exit 1
fi

# Convertir a rutas absolutas
ANTIGUO=$(cd "$ANTIGUO" 2>/dev/null && pwd) || mostrar_error "Directorio ANTIGUO ($1) no existe: $1"
NUEVO=$(cd "$NUEVO" 2>/dev/null && pwd) || mostrar_error "Directorio NUEVO ($2) no existe: $2"

# Validar que sean diferentes
if [ "$ANTIGUO" = "$NUEVO" ]; then
    mostrar_error "Los directorios ANTIGUO y NUEVO no pueden ser iguales"
fi

# Crear y limpiar directorio diferencial
if [ -d "$DIFERENCIAL" ]; then
    echo -e "${AMARILLO}Limpiando directorio diferencial previo...${NC}"
    rm -rf "$DIFERENCIAL"
fi
mkdir -p "$DIFERENCIAL" || mostrar_error "No se puede crear directorio diferencial: $DIFERENCIAL"
DIFERENCIAL=$(cd "$DIFERENCIAL" && pwd)

echo -e "${AZUL}========================================${NC}"
echo -e "${AZUL}Extracción de Cambios (Hash MD5)${NC}"
echo -e "${AZUL}========================================${NC}"
echo -e "Directorio ANTIGUO ($1):        ${VERDE}$ANTIGUO${NC}"
echo -e "Directorio NUEVO ($2):          ${VERDE}$NUEVO${NC}"
echo -e "Directorio DIFERENCIAL ($3):    ${VERDE}$DIFERENCIAL${NC}"
if [ -n "$VERSION" ]; then
    echo -e "Versión ZIP:                    ${VERDE}$VERSION${NC}"
fi
echo -e "Prefijo para .deletes:          ${VERDE}$RUTA_PREFIX${NC}"
echo -e "${AZUL}========================================${NC}"
echo ""

echo -e "${AMARILLO}Calculando hashes MD5...${NC}"

# Generar hashes normalizados (sin rutas base)
find "$ANTIGUO" -type f -print0 | xargs -0 md5sum 2>/dev/null | \
    sed "s|$ANTIGUO/||" | sort -k2 > /tmp/hash_antiguo.txt

find "$NUEVO" -type f -print0 | xargs -0 md5sum 2>/dev/null | \
    sed "s|$NUEVO/||" | sort -k2 > /tmp/hash_nuevo.txt

ARCHIVOS_ANTIGUO=$(wc -l < /tmp/hash_antiguo.txt)
ARCHIVOS_NUEVO=$(wc -l < /tmp/hash_nuevo.txt)

echo -e "${VERDE}✓ Hashes calculados${NC}"
echo -e "  Archivos en ANTIGUO: $ARCHIVOS_ANTIGUO"
echo -e "  Archivos en NUEVO: $ARCHIVOS_NUEVO"
echo ""

echo -e "${AMARILLO}Detectando cambios...${NC}"
echo ""

COPIADOS=0
ERRORES=0
MODIFICADOS=0
NUEVOS=0
ELIMINADOS=0
DIRS_ELIMINADOS=0

# PASO 1: Encontrar archivos MODIFICADOS (existen en ambos pero con hash diferente)
echo -e "${AZUL}Buscando archivos MODIFICADOS...${NC}"
while read ruta hash_antiguo hash_nuevo; do
    if [ "$hash_antiguo" != "$hash_nuevo" ]; then
        archivo_origen="$NUEVO/$ruta"
        archivo_destino="$DIFERENCIAL/$ruta"
        
        if [ -f "$archivo_origen" ]; then
            dir_dest=$(dirname "$archivo_destino")
            mkdir -p "$dir_dest"
            
            if cp -p "$archivo_origen" "$archivo_destino" 2>/dev/null; then
                echo -e "  ${AMARILLO}[MOD]${NC} $ruta"
                ((MODIFICADOS++))
                ((COPIADOS++))
            else
                echo -e "  ${ROJO}✗${NC} Error al copiar: $ruta"
                ((ERRORES++))
            fi
        fi
    fi
done < <(join -j 2 <(sort -k2 /tmp/hash_antiguo.txt) <(sort -k2 /tmp/hash_nuevo.txt))

# PASO 2: Encontrar archivos NUEVOS (existen en NUEVO pero no en ANTIGUO)
echo -e "${AZUL}Buscando archivos NUEVOS...${NC}"
while read archivo; do
    [ -z "$archivo" ] && continue
    
    archivo_origen="$NUEVO/$archivo"
    archivo_destino="$DIFERENCIAL/$archivo"
    
    if [ -f "$archivo_origen" ]; then
        dir_dest=$(dirname "$archivo_destino")
        mkdir -p "$dir_dest"
        
        if cp -p "$archivo_origen" "$archivo_destino" 2>/dev/null; then
            echo -e "  ${VERDE}[NEW]${NC} $archivo"
            ((NUEVOS++))
            ((COPIADOS++))
        else
            echo -e "  ${ROJO}✗${NC} Error al copiar: $archivo"
            ((ERRORES++))
        fi
    fi
done < <(comm -23 <(cut -d' ' -f2- /tmp/hash_nuevo.txt | sort) \
                   <(cut -d' ' -f2- /tmp/hash_antiguo.txt | sort))

# PASO 3: Encontrar archivos ELIMINADOS y guardar en .deletes con rutas completas
echo -e "${AZUL}Buscando archivos ELIMINADOS...${NC}"

DELETES_FILE="$DIFERENCIAL/.deletes"

# Crear archivo temporal con eliminados sin procesar
TEMP_DELETES=$(mktemp)

comm -13 <(cut -d' ' -f2- /tmp/hash_nuevo.txt | sort) \
         <(cut -d' ' -f2- /tmp/hash_antiguo.txt | sort) > "$TEMP_DELETES"

# Procesar eliminados y agregar prefijo
> "$DELETES_FILE"

while read archivo; do
    [ -z "$archivo" ] && continue
    
    # Agregar prefijo a la ruta
    ruta_completa="${RUTA_PREFIX}${archivo}"
    
    echo "$ruta_completa" >> "$DELETES_FILE"
    echo -e "  ${ROJO}[DEL]${NC} $ruta_completa"
    ((ELIMINADOS++))
done < "$TEMP_DELETES"

# Si el archivo de deletes está vacío, eliminarlo
if [ ! -s "$DELETES_FILE" ]; then
    rm -f "$DELETES_FILE"
    echo -e "  ${VERDE}(No hay archivos eliminados)${NC}"
else
    echo -e "${VERDE}✓ Lista de eliminados guardada en: ${AZUL}$DELETES_FILE${NC}"
fi

# PASO 4: Eliminar directorios vacíos de ANTIGUO simulando el estado
echo -e "${AZUL}Limpiando directorios vacíos...${NC}"

# Crear lista temporal de directorios que quedarían vacíos después de eliminar archivos
TEMP_DIRS=$(mktemp)
find "$ANTIGUO" -type d -print0 | while IFS= read -r -d '' dir; do
    rel_path="${dir#$ANTIGUO/}"
    
    # Contar archivos que quedarían en este directorio después de las eliminaciones
    files_count=0
    while read archivo; do
        [ -z "$archivo" ] && continue
        archivo_dir=$(dirname "$archivo")
        
        if [ "$rel_path" = "$archivo_dir" ] || [[ "$archivo_dir" == "$rel_path"/* ]]; then
            ((files_count++))
        fi
    done < "$TEMP_DELETES"
    
    # Si el directorio estaría vacío, marcarlo para eliminación
    if [ $files_count -eq 0 ] && [ -n "$rel_path" ]; then
        if ! grep -q "^$rel_path" /tmp/hash_nuevo.txt 2>/dev/null; then
            echo "$rel_path" >> "$TEMP_DIRS"
            echo -e "  ${ROJO}[DIR-VACIO]${NC} ${RUTA_PREFIX}${rel_path}"
            ((DIRS_ELIMINADOS++))
        fi
    fi
done < "$TEMP_DELETES"

# Agregar directorios vacíos al archivo .deletes si existen
if [ -s "$TEMP_DIRS" ]; then
    if [ -f "$DELETES_FILE" ]; then
        echo "" >> "$DELETES_FILE"
        while read dir; do
            echo "${RUTA_PREFIX}${dir}/" >> "$DELETES_FILE"
        done < "$TEMP_DIRS"
    fi
fi

rm -f "$TEMP_DELETES" "$TEMP_DIRS"

# Limpiar archivos temporales
rm -f /tmp/hash_antiguo.txt /tmp/hash_nuevo.txt

echo ""
echo -e "${AZUL}========================================${NC}"
echo -e "${VERDE}Proceso completado${NC}"
echo -e "${AZUL}========================================${NC}"
echo -e "Archivos MODIFICADOS:     ${AMARILLO}$MODIFICADOS${NC}"
echo -e "Archivos NUEVOS:          ${VERDE}$NUEVOS${NC}"
echo -e "Archivos ELIMINADOS:      ${ROJO}$ELIMINADOS${NC}"
echo -e "Directorios VACÍOS:       ${ROJO}$DIRS_ELIMINADOS${NC}"
echo -e "Total COPIADOS:           ${VERDE}$COPIADOS${NC}"
if [ "$ERRORES" -gt 0 ]; then
    echo -e "Errores:                  ${ROJO}$ERRORES${NC}"
fi
echo -e "${AZUL}========================================${NC}"

# PASO 5: Crear archivo ZIP si se proporciona versión
if [ -n "$VERSION" ]; then
    echo ""
    echo -e "${AMARILLO}Creando archivo ZIP...${NC}"
    
    ZIPNAME="update_koriki_v${VERSION}.zip"
    PADRE_DIFERENCIAL=$(dirname "$DIFERENCIAL")
    ZIPPATH="$PADRE_DIFERENCIAL/$ZIPNAME"
    
    # Crear zip desde el contenido de DIFERENCIAL sin incluir ruta absoluta
    if (cd "$DIFERENCIAL" && zip -r "$ZIPPATH" . -q > /dev/null 2>&1); then
        ZIPSIZE=$(du -h "$ZIPPATH" | cut -f1)
        echo -e "${VERDE}✓ ZIP creado correctamente${NC}"
        echo -e "  Nombre: ${AZUL}$ZIPNAME${NC}"
        echo -e "  Ubicación: ${AZUL}$ZIPPATH${NC}"
        echo -e "  Tamaño: ${AZUL}$ZIPSIZE${NC}"
    else
        echo -e "${ROJO}✗ Error al crear el ZIP${NC}"
    fi
    echo -e "${AZUL}========================================${NC}"
fi

echo -e "${VERDE}¡Proceso finalizado!${NC}"
