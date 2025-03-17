#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480
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

// Function to draw ECG signal
void draw_ecg(SDL_Renderer* renderer, double* data, size_t samples, int y_offset, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);

    int points = SCAN_RATE * TIME_WINDOW;
    if (samples < points) points = samples;

    double min_val = data[CHANNEL_INDEX];
    double max_val = data[CHANNEL_INDEX];

    // Find min and max in window
    for (int i = 0; i < points; i++) {
        double value = data[i * NUM_CHANNELS + CHANNEL_INDEX];
        if (value < min_val) min_val = value;
        if (value > max_val) max_val = value;
    }

    // Scale and plot points
    for (int i = 0; i < points - 1; i++) {
        double v1 = data[i * NUM_CHANNELS + CHANNEL_INDEX];
        double v2 = data[(i + 1) * NUM_CHANNELS + CHANNEL_INDEX];
        int x1 = (i * SCREEN_WIDTH) / points;
        int x2 = ((i + 1) * SCREEN_WIDTH) / points;

        int y1 = y_offset + SCREEN_HEIGHT / 4 - (int)((v1 - min_val) / (max_val - min_val + 1e-6) * (SCREEN_HEIGHT / 4));
        int y2 = y_offset + SCREEN_HEIGHT / 4 - (int)((v2 - min_val) / (max_val - min_val + 1e-6) * (SCREEN_HEIGHT / 4));

        SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
    }
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Failed to initialize SDL: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Mother and Baby ECG",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    size_t mom_samples, baby_samples;
    double* mom_data = load_ecg_data("/home/pi/Desktop/ESET420/ECGSignals/OriginalECGRacapMom.dat", &mom_samples);
    double* baby_data = load_ecg_data("/home/pi/Desktop/ESET420/ECGSignals/BabyECGRacap.dat", &baby_samples);

    if (!mom_data || !baby_data) {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_Color mom_color = { COLOR_MOM_R, COLOR_MOM_G, COLOR_MOM_B };
    SDL_Color baby_color = { COLOR_BABY_R, COLOR_BABY_G, COLOR_BABY_B };

    int running = 1;
    SDL_Event e;
    int offset = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        draw_ecg(renderer, mom_data + offset * NUM_CHANNELS, mom_samples - offset, 0, mom_color);
        draw_ecg(renderer, baby_data + offset * NUM_CHANNELS, baby_samples - offset, SCREEN_HEIGHT / 2, baby_color);

        SDL_RenderPresent(renderer);

        SDL_Delay(100); // Adjust refresh rate

        offset += SCAN_RATE; // Move 1 second forward
        if (offset + SCAN_RATE * TIME_WINDOW >= mom_samples) offset = 0; // Loop around
    }

    free(mom_data);
    free(baby_data);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
