#!/usr/bin/env python2.7
import pygame
import os

pygame.init()

screen_width, screen_height = 640, 480
screen = pygame.display.set_mode((screen_width, screen_height), pygame.RESIZABLE)
pygame.display.set_caption("DOSBox Key Mapper")

WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
GREEN = (0, 255, 0)
ORANGE = (255, 165, 0)
BLUE = (0, 255, 255)

font_size = 24
font = pygame.font.Font(None, font_size)

key_mappings = {}
current_hardware_key_index = 0
current_keyboard_key_index = 0
running = True

displayed_changes = set([
    "UP", "DOWN", "RIGHT", "LEFT", "A", "B", "Y", "X",
    "L1", "R1", "L2", "R2", "start", "select", "menu"
])

miyoo_keycodes = [
    "UP", "DOWN", "RIGHT", "LEFT", "A", "B", "Y", "X",
    "L1", "R1", "L2", "R2", "start", "select", "menu"
]

special_keys = {
    "mod_1": ("key 27", "#MENU"),
    "hand_shutdown": ("key 13 mod1", "#MENU + START")
}

keyboard_keys = sorted(
    key for key in [
        "hand_shutdown", "hand_capmouse", "hand_fullscr", "hand_restart", "hand_pause",
        "hand_mapper", "hand_speedlock", "hand_recwave", "hand_caprawmidi", "hand_scrshot",
        "hand_video", "hand_decfskip", "hand_cycledown", "hand_cycleup", "hand_caprawopl",
        "hand_swapimg", "key_esc", "key_f1", "key_f2", "key_f3", "key_f4",
        "key_f5", "key_f6", "key_f7", "key_f8", "key_f9", "key_f10", "key_f11", "key_f12",
        "key_grave", "key_1", "key_2", "key_3", "key_4", "key_5", "key_6", "key_7", "key_8",
        "key_9", "key_0", "key_minus", "key_equals", "key_bspace", "key_tab", "key_q", "key_w",
        "key_e", "key_r", "key_t", "key_y", "key_u", "key_i", "key_o", "key_p", "key_lbracket",
        "key_rbracket", "key_enter", "key_capslock", "key_a", "key_s", "key_d", "key_f", "key_g",
        "key_h", "key_j", "key_k", "key_l", "key_semicolon", "key_quote", "key_backslash",
        "key_lshift", "key_lessthan", "key_z", "key_x", "key_c", "key_v", "key_b", "key_n", "key_m",
        "key_comma", "key_period", "key_slash", "key_rshift", "key_lctrl", "key_lalt", "key_space",
        "key_ralt", "key_rctrl", "key_printscreen", "key_scrolllock", "key_pause", "key_insert",
        "key_home", "key_pageup", "key_delete", "key_end", "key_pagedown", "key_up", "key_left",
        "key_down", "key_right", "key_numlock", "key_kp_divide", "key_kp_multiply", "key_kp_minus",
        "key_kp_7", "key_kp_8", "key_kp_9", "key_kp_plus", "key_kp_4", "key_kp_5", "key_kp_6",
        "key_kp_1", "key_kp_2", "key_kp_3", "key_kp_enter", "key_kp_0", "key_kp_period",
        "mod_1", "mod_2", "mod_3"
    ] if key not in special_keys
)

keycode_mapping = {
    "UP": ("key_up", "key 273", "#UP"),
    "DOWN": ("key_down", "key 274", "#DOWN"),
    "LEFT": ("key_left", "key 276", "#LEFT"),
    "RIGHT": ("key_right", "key 275", "#RIGHT"),
    "A": ("key_space", "key 32", "#A"),
    "B": ("key_lctrl", "key 306", "#B"),
    "Y": ("key_lalt", "key 308", "#Y"),
    "X": ("key_lshift", "key 304", "#X"),
    "L1": ("key_y", "key 101", "#L1"),
    "R1": ("key_c", "key 116", "#R1"),
    "L2": ("key_pageup", "key 9", "#L2"),
    "R2": ("key_pagedown", "key 8", "#R2"),
    "start": ("key_enter", "key 13", "#START"),
    "select": ("key_tab", "key 305", "#SELECT"),
    "menu": ("key_esc", "key 27", "#MENU"),
}

circle_coordinates = {
    "UP": (62, 395),
    "DOWN": (62, 421),
    "LEFT": (48, 408),
    "RIGHT": (75, 408),
    "A": (147, 408),
    "B": (132, 423),
    "Y": (116, 408),
    "X": (132, 393),
    "L1": (316, 340),
    "R1": (204, 340),
    "L2": (291, 340),
    "R2": (231, 340),
    "start": (101, 444),
    "select": (81, 444),
    "menu": (97, 391)
}

def draw_text(text, x, y, color=GREEN):
    screen_text = font.render(text, True, color)
    screen.blit(screen_text, (x, y))

def draw_changes():
    x_offset = 382
    y_offset = 100
    
    order = [
        "UP", "DOWN", "LEFT", "RIGHT", "A", "B", "Y", "X",
        "L1", "R1", "L2", "R2", "start", "select", "menu"
    ]
    
    for hw_key in order:
        if hw_key in displayed_changes:
            if y_offset + font.get_height() > screen_height - 40:
                break
            kb_name, _, _ = keycode_mapping.get(hw_key, ("None", "None", "No Mapping"))
            assigned_key = key_mappings.get(hw_key, kb_name)
            draw_text("%s = %s" % (hw_key, assigned_key), x_offset, y_offset, color=ORANGE)
            y_offset += font.get_height() + 4

def draw_current_key_indicator():
    current_hardware_key = miyoo_keycodes[current_hardware_key_index]
    if current_hardware_key in circle_coordinates:
        x, y = circle_coordinates[current_hardware_key]
        pygame.draw.circle(screen, BLUE, (x, y), 8)

def load_mapping():
    mapper_directory = "./.dosbox"
    mapper_file_path = os.path.join(mapper_directory, "mapper.txt")
    
    global key_mappings
    key_mappings = {hw_key: kb_name for hw_key, (kb_name, _, _) in keycode_mapping.items()}

    if not os.path.exists(mapper_file_path):
        return

    try:
        with open(mapper_file_path, "r") as mapper_file:
            for line in mapper_file:
                parts = line.strip().split('"')
                if len(parts) >= 3:
                    key = parts[0].strip()
                    value = parts[1].strip()
                    if key in key_mappings:
                        key_mappings[key] = value
    except IOError as e:
        print("Error loading configuration: {}".format(e))

def save_mapping():
    mapper_directory = "./.dosbox"
    mapper_file_path = os.path.join(mapper_directory, "mapper.txt")
    
    if not os.path.exists(mapper_directory):
        os.makedirs(mapper_directory)

    try:
        with open(mapper_file_path, "w") as mapper_file:
            # Write special keys
            for special_key, (special_value, comment) in special_keys.items():
                mapper_file.write('%s "%s" %s\n' % (special_key, special_value, comment))

            # Write changes only for the edited keys and default mappings if they were changed
            written_keys = set()
            for hw_key in sorted(displayed_changes):
                kb_name, kb_code, comment = keycode_mapping.get(hw_key, ("None", "None", "No Mapping"))
                assigned_key = key_mappings.get(hw_key, kb_name)
                mapper_file.write('%s "%s" %s\n' % (assigned_key, kb_code, comment))
                written_keys.add(hw_key)

            # Write default mappings for all non-modified keys
            for hw_key in sorted(keycode_mapping.keys()):
                if hw_key not in written_keys:
                    kb_name, kb_code, comment = keycode_mapping[hw_key]
                    mapper_file.write('%s "%s" %s\n' % (kb_name, kb_code, comment))
                
        print("Configuration saved successfully.")
    except IOError as e:
        print("Error saving configuration: {}".format(e))

background_image = pygame.image.load("background.png")

def scale_background(image, screen_size):
    image_width, image_height = image.get_size()
    screen_width, screen_height = screen_size

    scale = min(screen_width / image_width, screen_height / image_height)
    new_size = (int(image_width * scale), int(image_height * scale))

    return pygame.transform.scale(image, new_size)

load_mapping()

while running:
    current_screen_size = screen.get_size()

    scaled_background = scale_background(background_image, current_screen_size)

    screen.fill(BLACK)
    bg_x = (current_screen_size[0] - scaled_background.get_width()) // 2
    bg_y = (current_screen_size[1] - scaled_background.get_height()) // 2
    screen.blit(scaled_background, (bg_x, bg_y))

    draw_text("Use DPAD LEFT/RIGHT to change hardware key", 20, 20)
    draw_text("DPAD UP/DOWN to change keyboard key, A to assign", 20, 50)
    draw_text("START to save", 20, 80)

    current_hardware_key = miyoo_keycodes[current_hardware_key_index]
    draw_text("Hardware Key: %s" % current_hardware_key, 20, 120)

    current_keyboard_key = keyboard_keys[current_keyboard_key_index]
    draw_text("Keyboard Key: %s" % current_keyboard_key, 20, 150)

    draw_changes()
    
    draw_current_key_indicator()

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_RETURN:  # START
                save_mapping()
                running = False
            elif event.key == pygame.K_RIGHT:  # D-pad right
                current_hardware_key_index = (current_hardware_key_index + 1) % len(miyoo_keycodes)
                current_hardware_key = miyoo_keycodes[current_hardware_key_index]
                displayed_changes.add(current_hardware_key)
            elif event.key == pygame.K_LEFT:  # D-pad left
                current_hardware_key_index = (current_hardware_key_index - 1) % len(miyoo_keycodes)
                current_hardware_key = miyoo_keycodes[current_hardware_key_index]
                displayed_changes.add(current_hardware_key)
            elif event.key == pygame.K_UP:  # D-Pad up
                current_keyboard_key_index = (current_keyboard_key_index - 1) % len(keyboard_keys)
                current_keyboard_key = keyboard_keys[current_keyboard_key_index]
            elif event.key == pygame.K_DOWN:  # D-Pad down
                current_keyboard_key_index = (current_keyboard_key_index + 1) % len(keyboard_keys)
                current_keyboard_key = keyboard_keys[current_keyboard_key_index]
            elif event.key == pygame.K_SPACE:  # A
                key_mappings[current_hardware_key] = current_keyboard_key
                displayed_changes.add(current_hardware_key)

    pygame.display.flip()

pygame.quit()
