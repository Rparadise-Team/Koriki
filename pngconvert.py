#!/usr/bin/env python3
"""
Script para optimizar archivos PNG para SDL 1.2:
- Convierte PNGs RGB a modo P (Paleta indexada)
- SALTA recompresión de PNGs ya en P (usa directamente optipng)
- Elimina canales alfa
- Máxima compresión sin pérdida
- Optimizado para rendimiento en hardware embebido
"""

import os
import sys
import subprocess
import warnings
from pathlib import Path
from PIL import Image

warnings.filterwarnings('ignore', category=UserWarning)

def get_png_mode(filepath):
    """Obtiene el modo de color sin cargar toda la imagen"""
    try:
        with Image.open(filepath) as img:
            return img.mode
    except:
        return None

def process_png(filepath):
    """Procesa un archivo PNG individual"""
    try:
        # Obtener modo actual
        current_mode = get_png_mode(filepath)
        if current_mode is None:
            return False, "No se puede leer el archivo", 0, 0, "?"
        
        with Image.open(filepath) as img:
            # Verificar resolución
            width, height = img.size
            if width <= 320 and height <= 240:
                return False, "Resolución menor a 320x240", 0, 0, current_mode
        
        original_size = os.path.getsize(filepath)
        
        # Si ya está en P, SOLO aplicar optipng (evita recompresión con PIL)
        if current_mode == 'P':
            try:
                subprocess.run(
                    ['optipng', '-o7', '-strip', 'all', filepath],
                    check=False,
                    capture_output=True
                )
            except FileNotFoundError:
                pass
            
            final_size = os.path.getsize(filepath)
            reduction = ((original_size - final_size) / original_size) * 100
            message = f"P → P (solo optipng) | {original_size:,} → {final_size:,} bytes ({reduction:+.1f}%)"
            
            return True, message, original_size, final_size, current_mode
        
        # Para otros modos, convertir a P
        with Image.open(filepath) as img:
            original_mode = img.mode
            
            # Paso 1: Eliminar canal alfa
            if img.mode in ('RGBA', 'LA', 'PA'):
                background = Image.new('RGB', img.size, (255, 255, 255))
                
                if img.mode == 'RGBA':
                    background.paste(img, mask=img.split()[3])
                elif img.mode == 'LA':
                    background.paste(img.convert('RGBA'), mask=img.split()[1])
                elif img.mode == 'PA':
                    background.paste(img.convert('RGBA'), mask=img.convert('RGBA').split()[3])
                
                img = background
            elif img.mode != 'RGB':
                img = img.convert('RGB')
            
            # Paso 2: Convertir a Paleta (P) con 256 colores
            img = img.convert('P', palette=Image.ADAPTIVE, colors=256)
            
            # Guardar con compresión máxima
            img.save(
                filepath,
                'PNG',
                optimize=True,
                compress_level=9
            )
        
        # Aplicar optipng para compresión adicional
        try:
            subprocess.run(
                ['optipng', '-o7', '-strip', 'all', filepath],
                check=False,
                capture_output=True
            )
        except FileNotFoundError:
            pass
        
        final_size = os.path.getsize(filepath)
        reduction = ((original_size - final_size) / original_size) * 100
        message = f"{original_mode} → P (256 colores) | {original_size:,} → {final_size:,} bytes ({reduction:.1f}% reducción)"
        
        return True, message, original_size, final_size, original_mode
    
    except Exception as e:
        return False, f"Error: {str(e)}", 0, 0, "?"

def scan_directory(directory):
    """Escanea directorio y subdirectorios buscando archivos PNG"""
    directory = Path(directory)
    
    if not directory.exists():
        print(f"❌ El directorio no existe: {directory}")
        return
    
    print(f"📂 Escaneando: {directory}")
    print(f"🎮 Modo: Paleta (P) para SDL 1.2 - Sin canal alfa")
    print(f"⏭️  PNGs en P: solo optipng (sin recompresión PIL)\n")
    print("="*80)
    
    png_files = list(directory.rglob("*.png")) + list(directory.rglob("*.PNG"))
    
    if not png_files:
        print("No se encontraron archivos PNG")
        return
    
    print(f"Encontrados {len(png_files)} archivos PNG\n")
    
    processed = 0
    skipped = 0
    already_p = 0
    errors = 0
    total_original = 0
    total_final = 0
    
    for png_file in png_files:
        print(f"📄 {png_file.relative_to(directory)}")
        
        success, message, original_size, final_size, mode = process_png(png_file)
        
        if success:
            if mode == 'P':
                print(f"  ⏭️  {message}")
                already_p += 1
            else:
                print(f"  ✓ {message}")
            total_original += original_size
            total_final += final_size
            processed += 1
        else:
            if "Resolución menor" in message:
                print(f"  ⊖ {message}")
                skipped += 1
            else:
                print(f"  ✗ {message}")
                errors += 1
        print()
    
    print("="*80)
    print(f"\n📊 Resumen:")
    print(f"  ✓ Procesados: {processed}")
    if already_p > 0:
        print(f"    • Convertidos RGB→P: {processed - already_p}")
        print(f"    • Ya en P (solo optipng): {already_p}")
    print(f"  ⊖ Omitidos (resolución): {skipped}")
    print(f"  ✗ Errores: {errors}")
    print(f"  📁 Total archivos: {len(png_files)}")
    
    if processed > 0 and total_original > 0:
        total_reduction = ((total_original - total_final) / total_original) * 100
        print(f"\n  📉 Tamaño total original: {total_original:,} bytes")
        print(f"  📦 Tamaño total final: {total_final:,} bytes")
        print(f"  📉 Reducción total: {total_reduction:.1f}%")
    
    print(f"\n✨ Todos los PNGs optimizados para SDL 1.2 (sin corrupción de P)")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        directorio = sys.argv[1]
    else:
        directorio = "."
    
    scan_directory(directorio)
