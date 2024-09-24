#!/usr/bin/env python2.7
import pygame
import os
import math
import subprocess
import json

pygame.init()

screen_width, screen_height = 640, 480
screen = pygame.display.set_mode((screen_width, screen_height), pygame.RESIZABLE)
pygame.display.set_caption("Koriki keys tester")

ORANGE = (255, 165, 0)
BLACK = (0, 0, 0)

running = True
pressed_keys = set()
l1_r1_pressed_together = False  # Flag to track if L1 and R1 were pressed together
is_charging = 0  # Charging status
vibration_active = False  # Track if vibration is active

# Coordinates for the icons
icon_1_coords = (175, 93)
icon_2_coords = (235, 90)

keycode = {  # Linux event keycodes
    "UP": ("103", "#UP"),
    "DOWN": ("108", "#DOWN"),
    "LEFT": ("105", "#LEFT"),
    "RIGHT": ("106", "#RIGHT"),
    "A": ("57", "#A"),
    "B": ("29", "#B"),
    "Y": ("56", "#Y"),
    "X": ("42", "#X"),
    "L1": ("18", "#L1"),
    "R1": ("20", "#R1"),
    "L2": ("15", "#L2"),
    "R2": ("14", "#R2"),
    "start": ("28", "#START"),
    "select": ("97", "#SELECT"),
    "menu": ("1", "#MENU"),
    "vol+": ("115", "#VOL+"),
    "vol-": ("114", "#VOL-"),
    "power": ("116", "#POWER"),
}

circle_coordinates = {
    "UP": (109, 306),    # "UP" button on the D-pad
    "DOWN": (109, 347),  # "DOWN" button on the D-pad
    "LEFT": (86, 327),   # "LEFT" button on the D-pad
    "RIGHT": (133, 327), # "RIGHT" button on the D-pad
    "A": (255, 327),     # "A" button
    "B": (229, 353),     # "B" button
    "Y": (203, 327),     # "Y" button
    "X": (229, 301),     # "X" button
    "L1": (570, 212),    # "L1" button
    "R1": (377, 212),    # "R1" button
    "L2": (526, 212),    # "L2" button
    "R2": (422, 212),    # "R2" button
    "start": (186, 380), # "start" button
    "select": (151, 380),# "select" button
    "menu": (170, 297),  # "menu" button
    "vol+": (42, 119),   # "vol+" button
    "vol-": (42, 155),   # "vol-" button
    "power": (250, 76)   # "power" button
}

background_image = pygame.image.load("background.png")

def scale_background(image, screen_size):
    image_width, image_height = image.get_size()
    screen_width, screen_height = screen_size

    scale = min(screen_width / image_width, screen_height / image_height)
    new_size = (int(image_width * scale), int(image_height * scale))

    return pygame.transform.scale(image, new_size)

def draw_current_key_indicator():
    for key in pressed_keys:
        if key in circle_coordinates:
            x, y = circle_coordinates[key]
            
            if key in ["UP", "DOWN", "LEFT", "RIGHT"]:
                pygame.draw.rect(screen, ORANGE, (x - 10, y - 10, 20, 20))
                
            elif key in ["L1", "R1"]:
                pygame.draw.rect(screen, ORANGE, (x - 15, y - 15, 30, 30))
                
            elif key in ["L2", "R2"]:
                pygame.draw.rect(screen, ORANGE, (x - 15, y - 15, 30, 20))
                
            elif key in ["vol+", "vol-", "power"]:
                pygame.draw.rect(screen, ORANGE, (x - 7, y - 7, 14, 14))
                
            elif key in ["start", "select"]:
                length = 22
                angle = math.radians(138)
                x_end = x + length * math.cos(angle)
                y_end = y + length * math.sin(angle)
                pygame.draw.line(screen, ORANGE, (x, y), (x_end, y_end), 8)
                
            elif key in ["A", "B", "X", "Y"]:
                pygame.draw.circle(screen, ORANGE, (x, y), 12)
                
            elif key == "menu":
                pygame.draw.circle(screen, ORANGE, (x, y), 7)

def check_special_combinations():
    global l1_r1_pressed_together, vibration_active

    if "L1" in pressed_keys and "R1" in pressed_keys:
        l1_r1_pressed_together = True
        os.system("echo 48 > /sys/class/gpio/export")
        os.system("echo out > /sys/class/gpio/gpio48/direction")
        vibration_active = True
    if "menu" in pressed_keys and "A" in pressed_keys:
        global running
        running = False

def checkCharging():
    global is_charging
    mmModel = os.access("/customer/app/axp_test", os.F_OK)
    if mmModel == 1:
        cmd = "cd /customer/app/ ; ./axp_test"
        result = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE)
        output = result.communicate()[0].decode('utf-8')
        if output:
            try:
                charging_index = output.find('"charging":')
                if charging_index != -1:
                    charging_value = output[charging_index + 11]
                    is_charging = int(charging_value)
            except Exception as e:
                print("Error processing output:", e)
                is_charging = 0
    else:
        try:
            with open("/sys/devices/gpiochip0/gpio/gpio59/value", "r") as f:
                is_charging = int(f.read().strip())
        except:
            is_charging = 0

while running:
    checkCharging()  # Check charging status on each loop

    current_screen_size = screen.get_size()

    scaled_background = scale_background(background_image, current_screen_size)

    screen.fill(BLACK)
    bg_x = (current_screen_size[0] - scaled_background.get_width()) // 2
    bg_y = (current_screen_size[1] - scaled_background.get_height()) // 2
    screen.blit(scaled_background, (bg_x, bg_y))

    # Draw the two black squares
    if not vibration_active:
        pygame.draw.rect(screen, BLACK, (icon_1_coords[0], icon_1_coords[1], 53, 53))
    if not is_charging:
        pygame.draw.rect(screen, BLACK, (icon_2_coords[0], icon_2_coords[1], 53, 53))

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            for key, value in keycode.items():
                if event.scancode == int(value[0]):
                    pressed_keys.add(key)
            check_special_combinations()
        elif event.type == pygame.KEYUP:
            for key, value in keycode.items():
                if event.scancode == int(value[0]):
                    pressed_keys.discard(key)

            if "L1" not in pressed_keys and "R1" not in pressed_keys and l1_r1_pressed_together:
                os.system("echo 1 > /sys/class/gpio/gpio48/value")
                os.system("echo 48 > /sys/class/gpio/unexport")
                vibration_active = False
                l1_r1_pressed_together = False  # Reset the flag

    draw_current_key_indicator()
    pygame.display.flip()

pygame.quit()
