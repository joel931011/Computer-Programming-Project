#include <SDL.h>
#include <iostream>
#include <cmath> // for sin, cos

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_BULLETS 1000000
#define PI 3.14159265

class Vec {
public:
    float vx, vy;

    Vec(float vx = 0, float vy = 0) : vx(vx), vy(vy) {}

    void setSpeed(float sx, float sy) {
        vx = sx;
        vy = sy;
    }
};

class Bullet : public Vec {
public:
    float x, y;
    bool active;

    Bullet() : Vec() {
        active = false;
    }

    // 改成用速度大小 v 與角度 theta
    void fire(float startX, float startY, float v, float theta = 0.0f) {
        x = startX;
        y = startY;
        setSpeed(v * cos(theta), v * sin(theta)); // 用 setSpeed 設定速度
        active = true;
    }

    void update(float dt) {
        if (!active) return;
        x += vx * dt;
        y += vy * dt;

        if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT)
            active = false;
    }
};

Bullet bullets[MAX_BULLETS];

//======================
// shootBullet 改成 overload，底層呼叫 Bullet::fire
//======================


void shootBullet(float x, float y, float v, float theta = 0.0f) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {
            bullets[i].fire(x, y, v, theta);
            break;
        }
    }
}

void updateBullets(float dt) {
    for (int i = 0; i < MAX_BULLETS; i++)
        bullets[i].update(dt);
}

void renderBullets(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            SDL_Rect rect = { (int)bullets[i].x, (int)bullets[i].y, 8, 8 };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "SDL Bullet Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    bool running = true;
    SDL_Event event;

    Uint32 lastTime = SDL_GetTicks();

    float playerX = 100, playerY = 300;

    while (running) {
        Uint32 current = SDL_GetTicks();
        float dt = (current - lastTime) / 1000.0f;
        lastTime = current;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_SPACE: 
                        shootBullet(playerX, playerY, 100.0f);          // 水平向右
                        shootBullet(playerX, playerY, 100.0f, -PI/8);   // 右上斜
                        shootBullet(playerX, playerY, 100.0f, PI/8);    // 右下斜
                        break;
                    case SDLK_UP:      
                        shootBullet(playerX, playerY, 100.0f, -PI/2); 
                        break;
                    case SDLK_DOWN:    
                        shootBullet(playerX, playerY, 100.0f, PI/2);  
                        break;
                    case SDLK_LEFT:    
                        shootBullet(playerX, playerY, 100.0f, PI);    
                        break;
                    case SDLK_RETURN:
                    	{
						float b=0;
					    for (float a=-PI/2;a<=9*PI/2.0;a=a+PI/10) 
					    {
					    	shootBullet(playerX, playerY, 150.0f-b, a);
					    	b+=2;
						}
						break;
					}
					case SDLK_d:
						{
						float c=0;
					    for (float a=-PI/2;a<=PI/2.0;a=a+PI/40) 
					    {
					    	shootBullet(playerX, playerY, 150.0f-c, a);
					    	c+=1;
						}
						for (float a=PI/2;a>=-3*PI/2.0;a=a-PI/40) 
					    {
					    	shootBullet(playerX, playerY, 150.0f-c, a);
					    	c+=1;
						}
					break;
				}
				case SDLK_e:
					{
						float d=0;
					    for (float a=-PI/2;a<=PI/2.0;a=a+PI/40) 
					    {
					    	shootBullet(playerX, playerY, (120.0f-d)*5, a);
					    	d+=1;
						}
						for (float a=PI/2;a>=-3*PI/2.0;a=a-PI/40) 
					    {
					    	shootBullet(playerX, playerY, (80.0f-d)*5, a);
					    	d+=1;
						}
					break;
				}
				case SDLK_f:
					{
						float e=0;
					    /*for (float a=-PI/2;a<=PI/2.0;a=a+PI/40) 
					    {
					    	shootBullet(playerX, playerY, (120.0f-d), a);
					    	d+=1;
						}*/
						for (float a=PI/2;a>=-5*PI/2.0;a=a-PI/40) 
					    {
					    	shootBullet(playerX, playerY, (110.0f-e), a);
					    	e+=0.65;
						}
					break;
					}
				case SDLK_g:
					{
						float f=1;
						for (float a=PI/2;a>=-5*PI/2.0;a=a-PI/40) 
					    {
					    	shootBullet(playerX, playerY, (110.0f-f*f), a);
					    	f=f+0.1;
						}
						break;
					}
                }
                
            }
        }

        updateBullets(dt);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect player = { (int)playerX, (int)playerY, 20, 20 };
        SDL_RenderFillRect(renderer, &player);

        renderBullets(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}


