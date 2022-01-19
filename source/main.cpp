#include "SDL.h"
#include "SDL_ttf.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>

// window params
const int WIDTH = 640, HEIGHT = 480;
float ROUNDS = 1;
bool IS_RUNNING = true;

// ``ball params''
const int BALL_SIZE = 20;
float BALL_X = (WIDTH / 2) - (BALL_SIZE / 2); 
float BALL_Y = (HEIGHT / 2) - (BALL_SIZE / 2);
float BALL_SPEEDX = 250, BALL_SPEEDY = 250;

// ``paddle params''
const int PADDLE_WIDTH = 10, PADDLE_HEIGHT = 125;
float PADDLE_POS_OFFSET = 20;
float PADDLE_SPEED = 400;

float PLAYER_X = PADDLE_POS_OFFSET, PLAYER_Y = (WIDTH / 2) - (PADDLE_HEIGHT) / 2;                               // player
float OPPONENT_X = WIDTH - PADDLE_WIDTH - PADDLE_POS_OFFSET, OPPONENT_Y = (WIDTH / 2) - (PADDLE_HEIGHT) / 2;   // opponent

void MakeRect(SDL_Rect &rect, int x, int y, int w, int h);
void DrawAll(SDL_Renderer *&renderer, float elapsed);
void DrawBall(SDL_Rect &ball, SDL_Renderer *&renderer, float elapsed);
void DrawPaddle(SDL_Rect &paddle, SDL_Renderer *&renderer, float elapsed, float &paddle_x, float &paddle_y, char* paddle_color);
void HandlePaddleMovement(bool isPlayer, float elapsed, float &whichY);

int main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Event event;
    SDL_Renderer *renderer;
    srand(time(0));

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "ERROR: Could not initialize window: %s\n", SDL_GetError());
    }

    window = SDL_CreateWindow(
        "Pong",
        20,
        20,
        WIDTH,
        HEIGHT,
        SDL_WINDOW_SHOWN
    );

    renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    uint64_t last_tick = SDL_GetTicks64();

    while(IS_RUNNING){
        uint64_t current_tick = SDL_GetTicks64();
        uint64_t tick_diff = current_tick - last_tick;
        float elapsed = tick_diff / 1000.0f;
        DrawAll(renderer, elapsed);
        last_tick = current_tick;
        
        SDL_PollEvent(&event);
        switch(event.type){
            case SDL_QUIT:
                IS_RUNNING = false;
                break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
}

// NOTE: Draw sprites and ``screen''
void DrawAll(SDL_Renderer *&renderer, float elapsed){
    // the canvas
    SDL_SetRenderDrawColor(
        renderer,
        40,
        44,
        52,
        255
    );
    SDL_RenderClear(renderer);

    SDL_Rect ball;
    DrawBall(ball, renderer, elapsed);

    SDL_Rect player;
    DrawPaddle(player, renderer, elapsed, PLAYER_X, PLAYER_Y, "#e06c75");     // player
    HandlePaddleMovement(true, elapsed, PLAYER_Y);

    SDL_Rect opponent;
    DrawPaddle(opponent, renderer, elapsed, OPPONENT_X, OPPONENT_Y, "#61afef"); // opponent
    HandlePaddleMovement(false, elapsed, OPPONENT_Y);

    if(SDL_HasIntersection(&ball, &player) && BALL_SPEEDX < 0){
        if (abs(ball.x - (player.x + player.w)) < 10) {
            BALL_SPEEDX = fabs(BALL_SPEEDX);
        }
        // TODO: MIDDLE GLITCH:

        // else if ((ball.x < player.x + player.w / 2) && (ball.x + ball.w > player.x + player.w / 2)) {
        //     printf("middle glitch\n");
        // }
        ROUNDS += 1;
    }
    if(SDL_HasIntersection(&ball, &opponent) && BALL_SPEEDX > 0){
        if (abs(ball.x + ball.w - opponent.x) < 10) {
            BALL_SPEEDX = -fabs(BALL_SPEEDX);
        }
        else if(abs((ball.y + ball.h) - opponent.y) < 10 && (BALL_SPEEDY > 0)){
            BALL_SPEEDY = -fabs(BALL_SPEEDY);
        }
        else if(abs(ball.y - (opponent.y + opponent.h)) < 10 && (BALL_SPEEDY < 0)){
            BALL_SPEEDY = fabs(BALL_SPEEDY);
        }
    }


    SDL_RenderPresent(renderer);
}

// ------------
std::vector <int> FromHexToRGB(char *hexString){
    std::vector <int> rgb;
    int r, g, b;
    sscanf(hexString, "#%02x%02x%02x", &r, &g, &b);
    rgb.push_back(r);
    rgb.push_back(g);
    rgb.push_back(b);

    return rgb;
}

// ------------
void DrawPaddle(SDL_Rect &paddle, SDL_Renderer *&renderer, float elapsed, float &paddle_x, float &paddle_y, char* paddle_color) {
    // the ``paddle''
    MakeRect(paddle, paddle_x, paddle_y, PADDLE_WIDTH, PADDLE_HEIGHT);

    std::vector <int> rgb;
    rgb = FromHexToRGB(paddle_color);

    // COLOR: LIGHT YELLOW
    SDL_SetRenderDrawColor(
        renderer, 
        rgb[0],
        rgb[1],
        rgb[2],
        255
    );
    SDL_RenderFillRect(renderer, &paddle);
}

void HandlePaddleMovement(bool isPlayer, float elapsed, float &whichY){
    if (isPlayer) {
        const Uint8 *state = SDL_GetKeyboardState(NULL);

        if (state[SDL_SCANCODE_W] || state[SDL_SCANCODE_UP]) {
            PLAYER_Y -= PADDLE_SPEED * elapsed; 
        }
        if (state[SDL_SCANCODE_S] || state[SDL_SCANCODE_DOWN]) {
            PLAYER_Y += PADDLE_SPEED * elapsed; 
        }
    }
    else {
        if (whichY < BALL_Y && BALL_X > WIDTH / 2 + 200) {
            whichY += round(PADDLE_SPEED * elapsed);
        }
        else if (whichY > BALL_Y && BALL_X > WIDTH / 2 + 200) {
            whichY -= round(PADDLE_SPEED * elapsed);
        }
    }
    
    if (round(whichY + PADDLE_HEIGHT) >= HEIGHT) {
        whichY = HEIGHT - PADDLE_HEIGHT - 1;
        return;
    }
    if (round(whichY) <= 0) {
        whichY = 1;
        return;
    }
}

// ------------
void DrawBall(SDL_Rect &ball, SDL_Renderer *&renderer, float elapsed){
    // the ``ball''
    MakeRect(ball, BALL_X, BALL_Y, BALL_SIZE, BALL_SIZE);

    BALL_X += (int)(BALL_SPEEDX * elapsed * (round(ROUNDS) / 30 + 1));
    BALL_Y += (int)(BALL_SPEEDY * elapsed * (round(ROUNDS) / 30 + 1));

    if(BALL_X <= 0){
        BALL_SPEEDX = BALL_SPEEDY = 0;
        IS_RUNNING = false;
    }
    else if (BALL_X >= WIDTH - BALL_SIZE){
        BALL_SPEEDX = BALL_SPEEDY = 0;
        IS_RUNNING = false;
    }
    else if (BALL_Y <= 0){
        BALL_SPEEDY = fabs(BALL_SPEEDY);
    }
    else if (BALL_Y >= HEIGHT - BALL_SIZE){
        BALL_SPEEDY = -fabs(BALL_SPEEDY);
    }

    // COLOR: LIGHT YELLOW
    SDL_SetRenderDrawColor(
        renderer,
        229,
        192,
        123,
        255
    );
    SDL_RenderFillRect(renderer, &ball);
}

// NOTE: Filling in Rect info easier
void MakeRect(SDL_Rect &rect, int x, int y, int w, int h){
    rect.w = w;
    rect.h = h;
    rect.x = x;
    rect.y = y;
}