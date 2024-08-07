import pygame
import sys, time, os
import subprocess as SU
from pygame.locals import *

pygame.init()

screen = pygame.display.set_mode((640, 480), pygame.SRCALPHA)
pygame.display.set_caption("GB/GBC roms selector")

WHITE = (255, 255, 255)
BLACK = (0, 0, 0, 128)
RED = (255, 0, 0)
GREEN = (0, 255, 0)
BLUE = (0, 0, 255)
GRAY = (128, 128, 128)
SELECTED_COLOR_1 = (0, 128, 255)
SELECTED_COLOR_2 = (255, 128, 0)
SELECTED_COLOR_3 = (255, 64, 255)

font = pygame.font.Font(None, 34)

ROMS_GB_PATH = "/mnt/SDCARD/Roms/GB"
ROMS_GBC_PATH = "/mnt/SDCARD/Roms/GBC"

current_directory = ROMS_GB_PATH

files_gb = sorted([f for f in os.listdir(ROMS_GB_PATH) if f.endswith(('.gb', '.gz', '.zip', '.7z'))])
files_gbc = sorted([f for f in os.listdir(ROMS_GBC_PATH) if f.endswith(('.gbc', '.zip', '.7z'))])

background_gb = pygame.image.load("/mnt/SDCARD/App/gb_server/background_gb.png").convert_alpha()
background_gbc = pygame.image.load("/mnt/SDCARD/App/gb_server/background_gbc.png").convert_alpha()

selected_roms = [None, None]
selected_index = [0, 0]
cursor_pos = [0, 0]

def get_ip_address():
    with open(os.devnull, "w") as fnull:
        output = SU.Popen(["/sbin/ifconfig", "wlan0"], stderr=fnull, stdout=SU.PIPE, close_fds=True).stdout.readlines()

    for line in output:
        if line.strip().startswith("inet addr"):
            ip = line.strip().split()[1].split(":")[1]
            if ip.startswith("192.168.4.100"):
                return True, ip

    return False, None

def show_message(title, instruction, warning=None):
    screen.fill(BLACK)
    
    title_text = font.render(title, True, WHITE)
    screen.blit(title_text, (screen.get_width() // 2 - title_text.get_width() // 2, 20))

    instruction_text = font.render(instruction, True, WHITE)
    screen.blit(instruction_text, (screen.get_width() // 2 - instruction_text.get_width() // 2, screen.get_height() - 40))

    if warning:
        warning_text = font.render(warning, True, RED)
        text_rect = warning_text.get_rect(center=(screen.get_width() // 2, screen.get_height() // 2))
        screen.blit(warning_text, text_rect)
    
    pygame.display.flip()

def list_roms():
    global selected_roms, selected_index, cursor_pos, current_directory
    clock = pygame.time.Clock()
    scroll = 0
    default_game_image = pygame.Surface((160, 120))
    default_game_image.fill(BLACK)
    
    game_image_rect = pygame.Rect(screen.get_width() - 160 - 30, screen.get_height() - 120 - 50, 160, 120)
    pygame.draw.rect(screen, BLACK, game_image_rect)
    pygame.draw.rect(screen, GRAY, game_image_rect, 3)

    current_game_image = default_game_image

    while True:
        files = files_gb if current_directory == ROMS_GB_PATH else files_gbc
        background = background_gb if current_directory == ROMS_GB_PATH else background_gbc
        image_directory = os.path.join(current_directory, 'Imgs')
        screen.blit(background, (0, 0))

        for i, rom_name in enumerate(files[scroll:scroll+8]):
            text_color = GRAY
            
            if cursor_pos[1] == scroll + i:
                text_color = WHITE
                
            if selected_roms[0] == selected_roms[1] == rom_name:
                text_color = SELECTED_COLOR_3
                number_text = font.render("PS", True, WHITE)
                screen.blit(number_text, (20, 60 + i * 50))
            elif selected_roms[1] == rom_name:
                text_color = SELECTED_COLOR_2
                number_text = font.render("P2", True, WHITE)
                screen.blit(number_text, (20, 60 + i * 50))
            elif selected_roms[0] == rom_name:
                text_color = SELECTED_COLOR_1
                number_text = font.render("P1", True, WHITE)
                screen.blit(number_text, (20, 60 + i * 50))
				
            text = font.render(rom_name, True, text_color)
            rect = text.get_rect(topleft=(50, 60 + i * 50))

            if cursor_pos[1] == scroll + i:
                cursor_rect = Rect(rect.x - 45, rect.y, 4, rect.height)
                pygame.draw.rect(screen, WHITE, cursor_rect)
                
            screen.blit(text, rect)

            image_directory = os.path.join(current_directory, 'Imgs')
            
            if cursor_pos[1] == scroll + i:
                current_rom_name = files[scroll + i]
                image_path = os.path.join(image_directory, os.path.splitext(current_rom_name)[0] + '.png')
                if os.path.exists(image_path):
                    current_game_image = pygame.image.load(image_path).convert_alpha()
                    current_game_image = pygame.transform.scale(current_game_image, (160, 120))
                else:
                    current_game_image = default_game_image
					
        pygame.draw.rect(screen, GRAY, game_image_rect, 3)
        screen.blit(current_game_image, (screen.get_width() - 160 - 30, screen.get_height() - 120 - 50))
        pygame.display.flip()
        clock.tick(30)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return
            if event.type == KEYDOWN:
                if event.key == K_UP:
                    selected_index[0] = max(0, selected_index[0] - 1)
                    cursor_pos[1] = max(cursor_pos[1] - 1, 0)
                    if selected_index[0] < scroll:
                        scroll = selected_index[0]
                elif event.key == K_DOWN:
                    selected_index[0] = min(len(files) - 1, selected_index[0] + 1)
                    cursor_pos[1] = min(cursor_pos[1] + 1, len(files) - 1)
                    if selected_index[0] >= scroll + 5:
                        scroll = selected_index[0] - 4
                elif event.key == K_LEFT:
                    selected_index[0] = max(0, selected_index[0] - 8)
                    cursor_pos[1] = max(cursor_pos[1] - 8, 0)
                    scroll = selected_index[0]
                elif event.key == K_RIGHT:
                    selected_index[0] = min(len(files) - 1, selected_index[0] + 8)
                    cursor_pos[1] = min(cursor_pos[1] + 8, len(files) - 1)
                    scroll = max(0, min(selected_index[0], len(files) - 8))
                elif event.key == K_e:
                    selected_index[0] = max(0, selected_index[0] - 80)
                    cursor_pos[1] = max(cursor_pos[1] - 80, 0)
                    scroll = selected_index[0]
                elif event.key == K_t:
                    selected_index[0] = min(len(files) - 1, selected_index[0] + 80)
                    cursor_pos[1] = min(cursor_pos[1] + 80, len(files) - 1)
                    scroll = max(0, min(selected_index[0], len(files) - 8))
                elif event.key == K_SPACE:
                    if selected_roms[0] is None:
                        selected_roms[0] = files[selected_index[0]]
                        rom1_path = os.path.join(current_directory, selected_roms[0])
                    elif selected_roms[1] is None:
                        selected_roms[1] = files[selected_index[0]]
                        rom2_path = os.path.join(current_directory, selected_roms[1])
                    else:
                        pygame.display.quit()
                        pygame.quit()
                        time.sleep(1)
                        SU.call(['/mnt/SDCARD/App/gb_server/tgbdual_libretro_server', rom1_path, rom2_path])
                        quit()
                        return
                elif event.key == K_ESCAPE:
                    pygame.quit()
                    return
                elif event.key == K_LCTRL:
                    if selected_roms[1] is not None:
                        selected_roms[1] = None
                    elif selected_roms[0] is not None:
                        selected_roms[0] = None
                    else:
                        pygame.quit()
                        quit()
                        return
                elif event.key == K_LSHIFT:
                    if current_directory == ROMS_GB_PATH:
                        current_directory = ROMS_GBC_PATH
                    elif current_directory == ROMS_GBC_PATH:
                        current_directory = ROMS_GB_PATH

if __name__ == "__main__":
    ip_exists, ip_address = get_ip_address()
    if not ip_exists:
        show_message("GB/GBC ROMs Selector", "Press MENU for exit", "You must connect to an adhoc network how server")
        while True:
            for event in pygame.event.get():
                if event.type == pygame.KEYDOWN and event.key == pygame.K_ESCAPE:
                    pygame.quit()
                    quit()
    list_roms()
