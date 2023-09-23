import pygame
import os
from pygame.locals import *

MUSIC_PATH = "/mnt/SDCARD/App/Musicselector/music/"
MUSIC_WAV_PATH = "/mnt/SDCARD/Media/music.wav"

pygame.init()

screen = pygame.display.set_mode((640, 480))
pygame.display.set_caption("Music Selector")

WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (255, 0, 0)

font = pygame.font.Font(None, 36)

songs = os.listdir(MUSIC_PATH)
songs.insert(0, "NO MUSIC")

selected_song = None
selected_index = 0

def list_songs():
    global selected_song, selected_index
    clock = pygame.time.Clock()
    scroll = 0

    while True:
        screen.fill(BLACK)
        
        for i, song in enumerate(songs[scroll:scroll+8]):
            text = font.render(song, True, RED)
            rect = text.get_rect(topleft=(60, 100 + i * 50))
            screen.blit(text, rect)

            if i == selected_index - scroll:
                pygame.draw.circle(screen, WHITE, (50, 100 + i * 50), 10)

        pygame.display.flip()
        clock.tick(30)

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                return
            if event.type == KEYDOWN:
                if event.key == K_UP:
                    selected_index = max(0, selected_index - 1)
                    if selected_index < scroll:
                        scroll = selected_index
                elif event.key == K_DOWN:
                    selected_index = min(len(songs) - 1, selected_index + 1)
                    if selected_index >= scroll + 8:
                        scroll = selected_index - 7
                elif event.key == K_SPACE:
                    selected_song = songs[selected_index]
                    return
                elif event.key == K_ESCAPE:
                    pygame.quit()
                    return

def delete_music():
    os.remove(MUSIC_WAV_PATH)

def convert_to_wav(song_name):
    command = '/mnt/SDCARD/Koriki/bin/ffmpeg -y -i "{}{}" -ar 48000 "{}"'.format(MUSIC_PATH, song_name, MUSIC_WAV_PATH)
    os.system(command)

def display_message(message):
    font = pygame.font.Font(None, 36)
    text = font.render(message, True, RED)
    rect = text.get_rect(center=(320, 240))
    screen.blit(text, rect)
    pygame.display.flip()

def wait_for_conversion():
    display_message("Please wait, converting file...")
	
if __name__ == "__main__":
    list_songs()

    if selected_song == "NO MUSIC":
        delete_music()
        print "File music.wav erased."
    elif selected_song:
        wait_for_conversion()
        convert_to_wav(selected_song)
        print "File convert and copy to music.wav."
