// gcc -o BaseUnit BaseUnit.c -lSDL2 -lSDL2_ttf

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define CHANNEL_INDEX 1 // ECG channel index
#define SCAN_RATE 1000  // Hz
#define NUM_CHANNELS 5  // Total number of channels
#define DURATION 550    // Total seconds of data
#define TIME_WINDOW 10  // Seconds of data to display
#define COLOR_MOM_R 255
#define COLOR_MOM_G 0
#define COLOR_MOM_B 0
#define COLOR_BABY_R 0
#define COLOR_BABY_G 0
#define COLOR_BABY_B 255

// Placeholder values
#define BASE_UNIT_BATTERY 80
#define WEARABLE_BATTERY 50
#define SKIN_TEMP 98

// Function to load ECG data from file
double* load_ecg_data(const char* filepath, size_t* out_samples) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        perror("Failed to open file");
        return NULL;
    }

    size_t total_samples = SCAN_RATE * DURATION * NUM_CHANNELS;
    double* buffer = (double*)malloc(total_samples * sizeof(double));
    if (!buffer) {
        perror("Failed to allocate memory");
        fclose(file);
        return NULL;
    }

    fread(buffer, sizeof(double), total_samples, file);
    fclose(file);

    *out_samples = total_samples / NUM_CHANNELS;
    return buffer;
}

// Function to compute baby's ECG signal
double* compute_baby_ecg(double* sum_ecg, double* mom_ecg, size_t samples) {
    double* baby_ecg = (double*)malloc(samples * sizeof(double));
    if (!baby_ecg) {
        perror("Failed to allocate memory for baby ECG");
        return NULL;
    }

    for (size_t i = 0; i < samples; i++) {
        baby_ecg[i] = sum_ecg[i * NUM_CHANNELS + CHANNEL_INDEX] - mom_ecg[i * NUM_CHANNELS + CHANNEL_INDEX];
    }

    return baby_ecg;
}

// Function to estimate heart rate
int calculate_heart_rate(double* data, size_t samples) {
    int peak_count = 0;
    for (size_t i = 1; i < samples - 1; i++) {
        if (data[i] > data[i - 1] && data[i] > data[i + 1]) {
            peak_count++;
        }
    }
    return (peak_count * 60) / TIME_WINDOW;
}

// Function to draw text
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

// Function to draw ECG signal
void draw_ecg(SDL_Renderer* renderer, double* data, size_t samples, int y_offset, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    int points = SCAN_RATE * TIME_WINDOW;
    if (samples < points) points = samples;

    double min_val = data[0], max_val = data[0];
    for (int i = 0; i < points; i++) {
        if (data[i] < min_val) min_val = data[i];
        if (data[i] > max_val) max_val = data[i];
    }

    for (int i = 0; i < points - 1; i++) {
        int x1 = (i * SCREEN_WIDTH) / points;
        int x2 = ((i + 1) * SCREEN_WIDTH) / points;
        int y1 = y_offset + SCREEN_HEIGHT / 4 - (int)((data[i] - min_val) / (max_val - min_val + 1e-6) * (SCREEN_HEIGHT / 4));
        int y2 = y_offset + SCREEN_HEIGHT / 4 - (int)((data[i + 1] - min_val) / (max_val - min_val + 1e-6) * (SCREEN_HEIGHT / 4));
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window* window = SDL_CreateWindow("ECG Monitor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 24);

    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }

    size_t mom_samples, sum_samples;
    double* mom_data = load_ecg_data("../../ECGSignals/OriginalECGRacapMom.dat", &mom_samples);
    double* sum_data = load_ecg_data("../../ECGSignals/SumECGRacapMomPlusBaby.dat", &sum_samples);
    
    if (!mom_data || !sum_data) {
        printf("Failed to load ECG data.\n");
        return 1;
    }

    double* baby_data = compute_baby_ecg(sum_data, mom_data, mom_samples);

    int mom_heart_rate = calculate_heart_rate(mom_data, mom_samples);
    int baby_heart_rate = calculate_heart_rate(baby_data, mom_samples);

    SDL_Color text_color = {255, 255, 255};
    SDL_Color mom_color = {COLOR_MOM_R, COLOR_MOM_G, COLOR_MOM_B};
    SDL_Color baby_color = {COLOR_BABY_R, COLOR_BABY_G, COLOR_BABY_B};

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_ecg(renderer, mom_data, mom_samples, SCREEN_HEIGHT / 4, mom_color);
        draw_ecg(renderer, baby_data, mom_samples, SCREEN_HEIGHT / 2, baby_color);

        char mom_hr_text[64], baby_hr_text[64];
        snprintf(mom_hr_text, sizeof(mom_hr_text), "Mom HR: %d BPM", mom_heart_rate);
        snprintf(baby_hr_text, sizeof(baby_hr_text), "Baby HR: %d BPM", baby_heart_rate);

        draw_text(renderer, font, mom_hr_text, 50, 50, text_color);
        draw_text(renderer, font, baby_hr_text, 50, 100, text_color);

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60 FPS
    }

    free(mom_data);
    free(sum_data);
    free(baby_data);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}

