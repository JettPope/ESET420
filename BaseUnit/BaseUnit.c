// gcc -o BaseUnit BaseUnit.c -lSDL2 -lSDL2_ttf

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define CHANNEL_INDEX 1 // ECG channel index
#define SCAN_RATE 1000  // Hz
#define NUM_CHANNELS 5  // Total number of channels
#define DURATION 550    // Total seconds of data
#define TIME_WINDOW 10  // Seconds of data to display
#define COLOR_MOM_R 255
#define COLOR_MOM_G 0
#define COLOR_MOM_B 0
#define COLOR_BABY_R 0
#define COLOR_BABY_G 255
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

// Function to estimate heart rate with moving average filtering
int calculate_heart_rate(double* data, size_t samples, double threshold) {
    int peak_count = 0;
    double prev_val = data[0], curr_val, next_val;

    for (size_t i = 1; i < samples - 1; i++) {
        curr_val = data[i];
        next_val = data[i + 1];

        // Peak detection with threshold
        if (curr_val > prev_val && curr_val > next_val && curr_val > threshold) {
            peak_count++;
            i += SCAN_RATE / 5; // Skip some points to prevent detecting the same peak twice
        }

        prev_val = curr_val;
    }

    double duration_seconds = (double)samples / SCAN_RATE;
    return (int)((peak_count / duration_seconds) * 60);  // BPM conversion
}


// Function to draw text
void draw_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color, int center) {
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x, y, surface->w, surface->h};
    
    if (center) rect.x = (SCREEN_WIDTH - rect.w) / 2;
    
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
        int y1 = y_offset - (int)((data[i] - min_val) / (max_val - min_val + 1e-6) * (SCREEN_HEIGHT / 6));
        int y2 = y_offset - (int)((data[i + 1] - min_val) / (max_val - min_val + 1e-6) * (SCREEN_HEIGHT / 6));
        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

void draw_axes(SDL_Renderer* renderer, TTF_Font* font, int y_offset) {
    SDL_Color white = {255, 255, 255};

    // Draw X-Axis
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, 50, y_offset + SCREEN_HEIGHT / 4, SCREEN_WIDTH - 50, y_offset + SCREEN_HEIGHT / 4);

    // Draw time labels
    for (int i = 0; i <= TIME_WINDOW; i++) {
        int x = 50 + (i * (SCREEN_WIDTH - 100)) / TIME_WINDOW;
        char label[8];
        snprintf(label, sizeof(label), "%ds", i);
        draw_text(renderer, font, label, x, y_offset + SCREEN_HEIGHT / 4 + 10, white, 0);
    }

    // Draw Y-Axis
    SDL_RenderDrawLine(renderer, 50, y_offset + 10, 50, y_offset + SCREEN_HEIGHT / 4 - 10);

    // Draw voltage labels
    for (int i = -1; i <= 1; i++) {  // Adjust range based on ECG amplitude
        int y = y_offset + SCREEN_HEIGHT / 4 - (int)((i + 1) * (SCREEN_HEIGHT / 8));
        char label[8];
        snprintf(label, sizeof(label), "%dmV", i * 500);  // Assuming ±500mV range
        draw_text(renderer, font, label, 10, y, white, 0);
    }
}


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    SDL_Window* window = SDL_CreateWindow("ECG Monitor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", 32);

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

    int mom_heart_rate = calculate_heart_rate(mom_data, mom_samples, 4.5);
    int baby_heart_rate = calculate_heart_rate(baby_data, mom_samples, .095);

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

        // Labels for the ECG graphs
        draw_text(renderer, font, "Mom", 50, SCREEN_HEIGHT / 4 - 50, text_color, 0);
        draw_text(renderer, font, "Baby", 50, SCREEN_HEIGHT / 2 - 50, text_color, 0);

        // Draw ECG signals with increased spacing
        draw_ecg(renderer, mom_data, mom_samples, SCREEN_HEIGHT / 3, mom_color);
        draw_ecg(renderer, baby_data, mom_samples, 2 * SCREEN_HEIGHT / 3, baby_color);

	    // draw axes
	    draw_axes(renderer, font, SCREEN_HEIGHT / 4);
        draw_axes(renderer, font, SCREEN_HEIGHT / 2);

        // Centered heart rate display
        char hr_text[128];
        snprintf(hr_text, sizeof(hr_text), "Mom HR: %d BPM   |   Baby HR: %d BPM", mom_heart_rate, baby_heart_rate);
        draw_text(renderer, font, hr_text, 0, 50, text_color, 1);

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

