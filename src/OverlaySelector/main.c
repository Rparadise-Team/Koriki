#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP 32
#define NUM_IMAGES 3

#define BUTTON_A SDLK_SPACE
#define BUTTON_B SDLK_LCTRL
#define BUTTON_MENU SDLK_ESCAPE
#define BUTTON_LEFT SDLK_LEFT
#define BUTTON_RIGHT SDLK_RIGHT

typedef struct {
    char key[64];
    char value[256];
} ConfigPair;

typedef struct {
    char core[64];
    char system[64];
    char border[8];
    ConfigPair modes[3][9]; // AspectRatio(0), Overlay(1), Fullscreen(2)
} OverlayConfig;

OverlayConfig cfg;
int miyoov4 = 0;

/* ---------------------------------------------------
   Utilidades básicas
--------------------------------------------------- */
void trim(char *s) {
    char *p = s;
    while (*p == ' ' || *p == '\t') p++;
    if (p != s) memmove(s, p, strlen(p) + 1);
    char *end = s + strlen(s) - 1;
    while (end >= s && (*end == ' ' || *end == '\t' || *end == '\n' || *end == '\r'))
        *end-- = '\0';
}

/* ---------------------------------------------------
   Carga del archivo overlay.cfg
--------------------------------------------------- */
void loadConfig(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        printf("No se pudo abrir el archivo de configuración: %s\n", filename);
        exit(1);
    }

    char line[512];
    int section = -1; // 0=Aspect, 1=Overlay, 2=Fullscreen
    int idx = 0;

    while (fgets(line, sizeof(line), f)) {
        trim(line);
        if (line[0] == '#' || strlen(line) == 0)
            continue;

        if (strstr(line, "[General]")) { section = -1; continue; }
        if (strstr(line, "[AspectRatio]")) { section = 0; idx = 0; continue; }
        if (strstr(line, "[Overlay]")) { section = 1; idx = 0; continue; }
        if (strstr(line, "[Fullscreen]")) { section = 2; idx = 0; continue; }

        if (section == -1) {
            if (sscanf(line, "core = %63[^#\n]", cfg.core)) { trim(cfg.core); continue; }
            if (sscanf(line, "system = %63[^#\n]", cfg.system)) { trim(cfg.system); continue; }
            if (sscanf(line, "border = %7[^#\n]", cfg.border)) { trim(cfg.border); continue; }
        } else {
            char key[128], val[256];
            if (sscanf(line, "%*d = %127[^,],%255[^\n]", key, val) == 2) {
                trim(key);
                trim(val);
                strcpy(cfg.modes[section][idx].key, key);
                strcpy(cfg.modes[section][idx].value, val);
                if (++idx >= 9) idx = 8;
            }
        }
    }

    fclose(f);
}

/* ---------------------------------------------------
   Escribe el archivo .cfg del core/sistema
--------------------------------------------------- */
void update_config(int mode) {
    char path[256];
    snprintf(path, sizeof(path),
             "/mnt/SDCARD/RetroArch/.retroarch/config/%s/%s.cfg",
             cfg.core, cfg.system);

    FILE *file = fopen(path, "w");
    if (!file) {
        printf("Error creando %s\n", path);
        return;
    }

    for (int i = 0; i < 9; i++) {
        if (strlen(cfg.modes[mode][i].key) && strlen(cfg.modes[mode][i].value)) {
            const char *value = cfg.modes[mode][i].value;

            // Detecta variantes MM/MMv4
            char val_copy[256];
            strcpy(val_copy, value);
            char *normal = strtok(val_copy, "|");
            char *v4 = strtok(NULL, "|");

            const char *final_value = normal;
            if (v4 && miyoov4)
                final_value = v4;

            fprintf(file, "%s = \"%s\"\n", cfg.modes[mode][i].key, final_value);
        }
    }

    fclose(file);
    printf("Configuración escrita en %s (%s, modo %d)\n",
           path, miyoov4 ? "MMv4" : "MM", mode);
}

/* ---------------------------------------------------
   Modifica mgba_sgb_borders si el sistema es SGB
--------------------------------------------------- */
void handle_sgb_border() {
    if (strcmp(cfg.system, "SGB") != 0)
        return;

    const char *optfile = "/mnt/SDCARD/RetroArch/.retroarch/config/mSGB/mSGB.opt";
    FILE *fp = fopen(optfile, "r+");
    if (!fp)
        return;

    char lines[512][256];
    int count = 0;
    while (fgets(lines[count], sizeof(lines[count]), fp) && count < 512)
        count++;

    rewind(fp);
    for (int i = 0; i < count; i++) {
        if (strstr(lines[i], "mgba_sgb_borders =")) {
            snprintf(lines[i], sizeof(lines[i]),
                     "mgba_sgb_borders = \"%s\"\n", cfg.border);
        }
        fputs(lines[i], fp);
    }
    fclose(fp);
}

/* ---------------------------------------------------
   Carga imágenes numeradas (1.png, 2.png, 3.png)
--------------------------------------------------- */
SDL_Surface *load_image(int index) {
    char filename[32];
    snprintf(filename, sizeof(filename), "./%d.png", index + 1);
    SDL_Surface *img = IMG_Load(filename);
    if (!img)
        printf("No se pudo cargar %s\n", filename);
    return img;
}

/* ---------------------------------------------------
   Dibuja recuadro con mensaje de confirmación
--------------------------------------------------- */
void draw_message(SDL_Surface *screen, SDL_Surface *msg) {
    SDL_Rect rect_bg;
    rect_bg.w = msg->w + 40;
    rect_bg.h = msg->h + 40;
    rect_bg.x = (SCREEN_WIDTH - rect_bg.w) / 2;
    rect_bg.y = (SCREEN_HEIGHT - rect_bg.h) / 2;

    Uint32 bg_color = SDL_MapRGB(screen->format, 40, 40, 40);
    SDL_FillRect(screen, &rect_bg, bg_color);

    SDL_Rect rect_pos = {
        (Sint16)(rect_bg.x + 20),
        (Sint16)(rect_bg.y + 20),
        (Uint16)msg->w,
        (Uint16)msg->h
    };
    SDL_BlitSurface(msg, NULL, screen, &rect_pos);
}

/* ---------------------------------------------------
   Programa principal
--------------------------------------------------- */
int main(int argc, char *argv[]) {
    const char *cfgpath = "./overlay.cfg";
    if (argc > 1)
        cfgpath = argv[1];

    // Detección MMv4
    miyoov4 = (access("/tmp/new_res_available", F_OK) == 0);

    // Cargar configuración externa
    loadConfig(cfgpath);

    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
        printf("Error inicializando SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Surface *screen =
        SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
    if (!screen) {
        printf("Error creando ventana SDL: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    // Cargar imágenes
    SDL_Surface *images[NUM_IMAGES];
    for (int i = 0; i < NUM_IMAGES; i++)
        images[i] = load_image(i);

    // Texto informativo
    TTF_Init();
    TTF_Font *font = TTF_OpenFont("./Exo-2-Bold-Italic.ttf", 32);
    SDL_Color white = {255, 255, 255};
    SDL_Surface *msg =
        TTF_RenderText_Blended(font, "Done! Press B to exit!", white);

    // Bucle principal
    bool running = true;
    int current = 0;
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                running = false;
            else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                case BUTTON_RIGHT:
                    current = (current + 1) % NUM_IMAGES;
                    break;
                case BUTTON_LEFT:
                    current = (current - 1 + NUM_IMAGES) % NUM_IMAGES;
                    break;
                case BUTTON_A:
                    update_config(current);
                    handle_sgb_border();
                    draw_message(screen, msg);
                    SDL_Flip(screen);
                    SDL_Delay(3000);
                    break;
                case BUTTON_B:
                case BUTTON_MENU:
                    running = false;
                    break;
                default:
                    break;
                }
            }
        }

        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        if (images[current]) {
            SDL_Rect img_rect = {
                (Sint16)((SCREEN_WIDTH - images[current]->w) / 2),
                (Sint16)((SCREEN_HEIGHT - images[current]->h) / 2),
                (Uint16)images[current]->w,
                (Uint16)images[current]->h};
            SDL_BlitSurface(images[current], NULL, screen, &img_rect);
        }
        SDL_Flip(screen);
    }

    // Limpieza
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();

    return 0;
}
