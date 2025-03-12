#include "SDL2/SDL.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define SAMPLING_RATE 1000

// Function to calculate heart rate from extracted ECG signal
int calculate_heart_rate(const float *signal, int length, int sampling_rate) {
    int peak_count = 0;
    for (int i = 1; i < length - 1; i++) {
        if (signal[i] > 0.1 && signal[i] > signal[i - 1] && signal[i] > signal[i + 1]) {
            peak_count++;
        }
    }
    if (peak_count > 1) {
        float rr_interval = (float)length / peak_count / sampling_rate;
        return (int)(60.0 / rr_interval);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() == -1) {
        printf("SDL Initialization failed: %s\n", SDL_GetError());
        return -1;
    }
    
    SDL_Window *window = SDL_CreateWindow("ECG Monitor", SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL, SDL_RENDERER_ACCELERATED);
    
    if (!window || !renderer) {
        printf("Failed to create window or renderer: %s\n", SDL_GetError());
        return -1;
    }
    
    TTF_Font *font = TTF_OpenFont("arial.ttf", 24);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return -1;
    }
    
    int base_unit_battery = 80;
    int wearable_battery = 50;
    int skin_temp = 98;
    
    // Placeholder ECG signals
    int signal_length = 600;
    float mother_ecg[600], baby_ecg[1000];
    for (int i = 0; i < signal_length; i++) {
        mother_ecg[i] = 0.5 * sin(2 * M_PI * 1.0 * i / SAMPLING_RATE);
    }
    for (int i = 0; i < 1000; i++) {
        baby_ecg[i] = 0.115 * sin(2 * M_PI * 1.7 * i / SAMPLING_RATE);
    }
    
    int mother_heart_rate = calculate_heart_rate(mother_ecg, signal_length, SAMPLING_RATE);
    int baby_heart_rate = calculate_heart_rate(baby_ecg, 1000, SAMPLING_RATE);
    
    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        for (int i = 0; i < signal_length - 1; i++) {
            SDL_RenderDrawLine(renderer, i, SCREEN_HEIGHT / 3 - mother_ecg[i] * 50, i + 1, SCREEN_HEIGHT / 3 - mother_ecg[i + 1] * 50);
        }
        
        SDL_SetRenderDrawColor(renderer, 255, 165, 0, 255);
        for (int i = 0; i < 999; i++) {
            SDL_RenderDrawLine(renderer, i, 2 * SCREEN_HEIGHT / 3 - baby_ecg[i] * 50, i + 1, 2 * SCREEN_HEIGHT / 3 - baby_ecg[i + 1] * 50);
        }
        
        SDL_RenderPresent(renderer);
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}
