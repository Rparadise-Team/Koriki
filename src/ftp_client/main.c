#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_ttf.h>

#define FONT_FILE "/mnt/SDCARD/App/ftp_client/Exo-2-Bold-Italic.ttf"
#define BACKGROUND_FILE "/mnt/SDCARD/App/ftp_client/background.png"
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define TEXT_COLOR 0xFF0000
#define ESCAPE_KEY SDLK_ESCAPE

// Función para obtener la dirección IP de wlan0 usando /bin/ip
char* getIPAddress() {
    FILE* pipe = popen("/bin/ip -4 -o addr show wlan0 | awk '{print $4}' | cut -d'/' -f1", "r");
    if (!pipe) {
        return NULL;
    }

    char buffer[128];
    fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);

    // Eliminar el salto de línea al final
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }

    return strdup(buffer);
}

// Función para configurar y ejecutar LFTP en segundo plano
void configureLFTP(const char* ipAddress) {
    FILE* lftpPipe = popen("lftp -c 'set ftp:ssl-allow no; set ftp:passive-mode true; set net:timeout 10; "
                           "open -u miyoomini ftp://%s; lcd /mnt/SDCARD; cd /mnt/SDCARD; shell'", "r");
    if (!lftpPipe) {
        fprintf(stderr, "Error: Failed to execute LFTP.\n");
        return;
    }

    pclose(lftpPipe);
}

// Función para mostrar la interfaz gráfica con los datos para FileZilla
void showGUI(const char* ipAddress) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Surface* screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_SWSURFACE);
    if (!screen) {
        fprintf(stderr, "Error: Failed to set video mode.\n");
        return;
    }

    SDL_Surface* background = IMG_Load(BACKGROUND_FILE);
    if (!background) {
        fprintf(stderr, "Error: Failed to load background image.\n");
        return;
    }

    SDL_Color textColor = {TEXT_COLOR >> 16, (TEXT_COLOR >> 8) & 0xFF, TEXT_COLOR & 0xFF};
    SDL_Surface* textSurface = NULL;

    TTF_Init();
    TTF_Font* font = TTF_OpenFont(FONT_FILE, 24);
    if (!font) {
        fprintf(stderr, "Error: Failed to load font.\n");
        return;
    }

    SDL_Rect textRect;
    textRect.x = 20;
    textRect.y = 20;

    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || (event.type == SDL_KEYDOWN && event.key.keysym.sym == ESCAPE_KEY)) {
                running = 0;
            }
        }

        SDL_BlitSurface(background, NULL, screen, NULL);

        char text[256];
        snprintf(text, sizeof(text), "Server Protocol: ftp://%s", ipAddress);
        textSurface = TTF_RenderText_Solid(font, text, textColor);
        if (textSurface) {
            SDL_BlitSurface(textSurface, NULL, screen, &textRect);
            textRect.y += textSurface->h + 10;
            SDL_FreeSurface(textSurface);
        }

        snprintf(text, sizeof(text), "FTP Client: miyoomini@%s", ipAddress);
        textSurface = TTF_RenderText_Solid(font, text, textColor);
        if (textSurface) {
            SDL_BlitSurface(textSurface, NULL, screen, &textRect);
            textRect.y += textSurface->h + 10;
            SDL_FreeSurface(textSurface);
        }

        snprintf(text, sizeof(text), "FileZilla Configuration:");
        textSurface = TTF_RenderText_Solid(font, text, textColor);
        if (textSurface) {
            SDL_BlitSurface(textSurface, NULL, screen, &textRect);
            textRect.y += textSurface->h + 10;
            SDL_FreeSurface(textSurface);
        }

        snprintf(text, sizeof(text), "Host: %s", ipAddress);
        textSurface = TTF_RenderText_Solid(font, text, textColor);
        if (textSurface) {
            SDL_BlitSurface(textSurface, NULL, screen, &textRect);
            textRect.y += textSurface->h + 10;
            SDL_FreeSurface(textSurface);
        }

        snprintf(text, sizeof(text), "Username: miyoomini");
        textSurface = TTF_RenderText_Solid(font, text, textColor);
        if (textSurface) {
            SDL_BlitSurface(textSurface, NULL, screen, &textRect);
            textRect.y += textSurface->h + 10;
            SDL_FreeSurface(textSurface);
        }

        snprintf(text, sizeof(text), "Password: <No password>");
        textSurface = TTF_RenderText_Solid(font, text, textColor);
        if (textSurface) {
            SDL_BlitSurface(textSurface, NULL, screen, &textRect);
            textRect.y += textSurface->h + 10;
            SDL_FreeSurface(textSurface);
        }

        snprintf(text, sizeof(text), "Port: 21");
        textSurface = TTF_RenderText_Solid(font, text, textColor);
        if (textSurface) {
            SDL_BlitSurface(textSurface, NULL, screen, &textRect);
            textRect.y += textSurface->h + 10;
            SDL_FreeSurface(textSurface);
        }

        SDL_Flip(screen);
    }

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_FreeSurface(background);
    SDL_Quit();
}

int main() {
    char* ipAddress = getIPAddress();
    if (!ipAddress) {
        fprintf(stderr, "Error: Failed to get IP address.\n");
        return 1;
    }

    printf("IP address: %s\n", ipAddress);

    configureLFTP(ipAddress);
    showGUI(ipAddress);

    free(ipAddress);

    return 0;
}
