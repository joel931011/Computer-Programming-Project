/**
 * Main.cpp
 * 整合：主角、敵人、Boss、道具、子彈、UI、音效、狀態機
 */

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>

// 引入自定義類別
#include "header/LTexture.h"
#include "header/player.h"
#include "header/enemy.h"
#include "header/boss.h"
#include "header/bullet.h"
#include "header/magnifier.h"

// --- 全域常數 ---
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;

// --- 遊戲狀態 ---
enum GameState {
    STATE_MENU,
    STATE_INSTRUCTION,
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_WIN
};

enum LevelStage {
    STAGE_1,
    BOSS_1_FIGHT,
    STAGE_2,
    BOSS_2_FIGHT
};

// --- 全域變數 (SDL 相關) ---
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;

// --- 全域變數 (紋理資源) ---
LTexture gMenuTexture;
LTexture gInstructionTexture;
LTexture gGameOverTexture;
LTexture gWinTexture;
LTexture gBGTexture; // 背景圖

// 角色與道具紋理
LTexture gPlayerFront, gPlayerLeft, gPlayerRight;
LTexture gEnemyTexture;
LTexture gBossTexture;
LTexture gBulletTexture;      // 主角子彈
LTexture gEnemyBulletTexture; // 敵人子彈
LTexture gMagnifierTexture;   // 道具

// UI 紋理
LTexture gScoreTextTexture;
LTexture gHealthTextTexture;

// --- 全域變數 (音效資源) ---
Mix_Music *gMusic = NULL;
Mix_Chunk *gLaserSound = NULL;
Mix_Chunk *gExplosionSound = NULL;
Mix_Chunk *gItemSound = NULL;

// --- 全域變數 (遊戲物件管理) ---
Player* gpPlayer = NULL;
Boss* gpBoss = NULL;
std::vector<Enemy*> gEnemies;
std::vector<Bullet*> gBullets;
std::vector<Magnifier*> gItems;

// --- 全域變數 (遊戲邏輯) ---
GameState gCurrentState = STATE_MENU;
LevelStage gLevelStage = STAGE_1;
int gEnemiesDefeated = 0;
int gScore = 0;

// --- 函式宣告 ---
bool init();
bool loadMedia();
void close();
void resetGame();

// =============================================================
// 主程式
// =============================================================

int main(int argc, char* args[])
{
    // 1. 初始化亂數種子
    srand((unsigned)time(NULL));

    // 2. 初始化 SDL
    if( !init() ) {
        printf( "Failed to initialize!\n" );
        return -1;
    }

    // 3. 載入媒體
    if( !loadMedia() ) {
        printf( "Failed to load media!\n" );
        return -1;
    }

    // 4. 建立主角物件 (此時紋理已載入)
    gpPlayer = new Player(&gPlayerFront, &gPlayerLeft, &gPlayerRight);

    // 播放背景音樂
    Mix_PlayMusic( gMusic, -1 );

    bool quit = false;
    SDL_Event e;
    SDL_Color textColor = { 255, 255, 255 }; // 白色文字

    // --- 遊戲主迴圈 ---
    while( !quit )
    {
        // --- 事件處理 ---
        while( SDL_PollEvent( &e ) != 0 )
        {
            if( e.type == SDL_QUIT ) quit = true;

            // 根據狀態處理按鍵
            switch(gCurrentState)
            {
                case STATE_MENU:
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_RETURN) { // Enter 開始
                            resetGame();
                            gCurrentState = STATE_PLAYING;
                        }
                        else if (e.key.keysym.sym == SDLK_i) { // I 看說明
                            gCurrentState = STATE_INSTRUCTION;
                        }
                    }
                    break;

                case STATE_INSTRUCTION:
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_b || e.key.keysym.sym == SDLK_ESCAPE) {
                            gCurrentState = STATE_MENU;
                        }
                    }
                    break;

                case STATE_PLAYING:
                    gpPlayer->handleEvent(e);

                    // 發射子彈
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                        std::vector<Bullet*> newShots = gpPlayer->fire(&gBulletTexture);
                        gBullets.insert(gBullets.end(), newShots.begin(), newShots.end());
                        Mix_PlayChannel( -1, gLaserSound, 0 );
                    }
                    // 暫停或回到選單 (可選)
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                        gCurrentState = STATE_MENU;
                    }
                    break;

                case STATE_GAME_OVER:
                case STATE_WIN:
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_r) { // R 重來
                            resetGame();
                            gCurrentState = STATE_PLAYING;
                        }
                    }
                    break;
            }
        }

        // --- 邏輯更新 (Update) ---
        if (gCurrentState == STATE_PLAYING)
        {
            // 1. 更新主角
            gpPlayer->move();
            gpPlayer->updateScatterStatus();

            // 2. 生成與更新敵人/Boss (關卡流程)
            switch (gLevelStage)
            {
                case STAGE_1:
                    if (rand() % 60 == 0) { // 生成敵人頻率
                        gEnemies.push_back(new Enemy(rand() % (SCREEN_WIDTH - 60), -60, &gEnemyTexture));
                    }
                    if (gEnemiesDefeated >= 10) {
                        gLevelStage = BOSS_1_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, -100, &gBossTexture, 1);
                    }
                    break;

                case BOSS_1_FIGHT:
                case BOSS_2_FIGHT:
                    // Boss 戰不生成小兵
                    break;

                case STAGE_2:
                    if (rand() % 40 == 0) { // 生成較快
                        gEnemies.push_back(new Enemy(rand() % (SCREEN_WIDTH - 60), -60, &gEnemyTexture));
                    }
                    if (gEnemiesDefeated >= 25) { // 累積擊殺
                        gLevelStage = BOSS_2_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, -100, &gBossTexture, 2);
                    }
                    break;
            }

            // 更新 Boss
            if (gpBoss) {
                gpBoss->move();
                std::vector<Bullet*> bShots = gpBoss->fire(&gEnemyBulletTexture);
                if (!bShots.empty()) gBullets.insert(gBullets.end(), bShots.begin(), bShots.end());

                if (gpBoss->isDead()) {
                    Mix_PlayChannel(-1, gExplosionSound, 0);
                    delete gpBoss;
                    gpBoss = NULL;

                    if (gLevelStage == BOSS_1_FIGHT) gLevelStage = STAGE_2;
                    else if (gLevelStage == BOSS_2_FIGHT) gCurrentState = STATE_WIN;
                }
            }

            // 3. 更新敵人 & 隨機掉寶
            for (int i = 0; i < gEnemies.size(); i++) {
                gEnemies[i]->move();
                std::vector<Bullet*> eShots = gEnemies[i]->fire(&gEnemyBulletTexture);
                if (!eShots.empty()) gBullets.insert(gBullets.end(), eShots.begin(), eShots.end());

                if (gEnemies[i]->isOffScreen() || gEnemies[i]->isDead()) {
                    if (gEnemies[i]->isDead()) {
                        gEnemiesDefeated++;
                        gScore += 100;
                        Mix_PlayChannel(-1, gExplosionSound, 0);

                        // 掉落道具 (5% 機率)
                        if (rand() % 20 == 0) {
                             // 道具生成在敵人死亡位置
                             // 若 Magnifier 建構子不需要座標，則用 gItems.push_back(new Magnifier());
                             // 若我們改回需要座標的版本:
                             gItems.push_back(new Magnifier(gEnemies[i]->getPosX(), gEnemies[i]->getPosY()));
                        }
                    }
                    delete gEnemies[i];
                    gEnemies.erase(gEnemies.begin() + i);
                    i--;
                }
            }

            // 4. 更新子彈與碰撞檢測
            for (int i = 0; i < gBullets.size(); i++) {
                gBullets[i]->move();
                SDL_Rect bRect = gBullets[i]->getCollider();
                bool hit = false;

                // 主角子彈 vs 敵人/Boss
                if (gBullets[i]->getIsFromPlayer()) {
                    // 撞敵人
                    for (auto& e : gEnemies) {
                        SDL_Rect eRect = e->getCollider();
                        if (SDL_HasIntersection(&bRect, &eRect)) {
                            e->takeDamage(1);
                            hit = true;
                            break;
                        }
                    }
                    // 撞 Boss
                    if (!hit && gpBoss) {
                        SDL_Rect bossRect = gpBoss->getCollider();
                        if (SDL_HasIntersection(&bRect, &bossRect)) {
                            gpBoss->takeDamage(1);
                            hit = true;
                        }
                    }
                }
                // 敵人子彈 vs 主角
                else {
                    SDL_Rect pRect = gpPlayer->getCollider();
                    if (SDL_HasIntersection(&bRect, &pRect)) {
                        gpPlayer->takeDamage(1);
                        hit = true;
                        // 可以加受傷音效
                    }
                }

                if (hit) gBullets[i]->setHit();

                if (gBullets[i]->isOffScreen() || !gBullets[i]->isAvailable()) {
                    delete gBullets[i];
                    gBullets.erase(gBullets.begin() + i);
                    i--;
                }
            }

            // 5. 更新道具
            SDL_Rect pRect = gpPlayer->getCollider();
            for (int i = 0; i < gItems.size(); i++) {
                gItems[i]->move();
                SDL_Rect itemRect = gItems[i]->getCollider();

                // 吃到道具
                if (SDL_HasIntersection(&pRect, &itemRect)) {
                    gpPlayer->activateScatter();
                    gItems[i]->setTaken();
                    Mix_PlayChannel(-1, gItemSound, 0);
                }

                if (!gItems[i]->isAvailable()) {
                    delete gItems[i];
                    gItems.erase(gItems.begin() + i);
                    i--;
                }
            }

            // 6. 檢查主角生死
            if (gpPlayer->getHealth() <= 0) {
                gCurrentState = STATE_GAME_OVER;
            }
        }

        // --- 渲染 (Render) ---

        // 清除畫面 (黑色背景)
        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear( gRenderer );

        if (gCurrentState == STATE_MENU) {
            gMenuTexture.render(0, 0);
        }
        else if (gCurrentState == STATE_INSTRUCTION) {
            gInstructionTexture.render(0, 0);
        }
        else if (gCurrentState == STATE_PLAYING) {
            // 畫背景 (如有)
            gBGTexture.render(0, 0);

            // 畫角色
            gpPlayer->render();
            if (gpBoss) gpBoss->render();
            for (auto e : gEnemies) e->render();
            for (auto b : gBullets) b->render();
            for (auto item : gItems) item->render();

            // 畫 UI
            std::stringstream timeText;
            timeText << "Score: " << gScore << "  HP: " << gpPlayer->getHealth();
            gScoreTextTexture.loadFromRenderedText(timeText.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(10, 10);
        }
        else if (gCurrentState == STATE_GAME_OVER) {
            gGameOverTexture.render(0, 0);

            // 顯示最終分數
            std::stringstream finalScore;
            finalScore << "Final Score: " << gScore;
            gScoreTextTexture.loadFromRenderedText(finalScore.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 50);
        }
        else if (gCurrentState == STATE_WIN) {
            // 假設你有勝利圖，若沒有就共用 GameOver 或純文字
            if (gWinTexture.getWidth() > 0) gWinTexture.render(0, 0);
            else gGameOverTexture.render(0, 0);

            std::stringstream winText;
            winText << "YOU WIN! Score: " << gScore;
            gScoreTextTexture.loadFromRenderedText(winText.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2);
        }

        // 更新螢幕
        SDL_RenderPresent( gRenderer );
    }

    // 釋放資源並退出
    close();
    return 0;
}

// =============================================================
// 輔助函式實作
// =============================================================

bool init()
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ) return false;

    // 設定線性過濾
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) ) {}

    gWindow = SDL_CreateWindow( "Space Shooter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( gWindow == NULL ) return false;

    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( gRenderer == NULL ) return false;

    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

    // IMG Init
    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) ) return false;

    // TTF Init
    if( TTF_Init() == -1 ) return false;

    // Mixer Init
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) return false;

    return true;
}

bool loadMedia()
{
    bool success = true;

    // 載入字體
    gFont = TTF_OpenFont( "BoutiqueBitmap9x9_1.92.ttf", 28 );
    if( gFont == NULL ) { printf("Failed to load font!\n"); success = false; }

    // 載入介面圖片 (請自行準備這些圖片)
    if( !gMenuTexture.loadFromFile( "menu.png", gRenderer ) ) success = false;
    if( !gInstructionTexture.loadFromFile( "instruction.png", gRenderer ) ) success = false;
    if( !gGameOverTexture.loadFromFile( "gameover.png", gRenderer ) ) success = false;
    // gWinTexture.loadFromFile("win.png", gRenderer); // 可選
    // gBGTexture.loadFromFile("bg.png", gRenderer);   // 可選

    // 載入角色圖片
    if( !gPlayerFront.loadFromFile( "player.png", gRenderer ) ) success = false;
    if( !gPlayerLeft.loadFromFile( "player_left.png", gRenderer ) ) success = false;
    if( !gPlayerRight.loadFromFile( "player_right.png", gRenderer ) ) success = false;

    if( !gEnemyTexture.loadFromFile( "enemy.png", gRenderer ) ) success = false;
    if( !gBossTexture.loadFromFile( "boss.png", gRenderer ) ) success = false;
    if( !gBulletTexture.loadFromFile( "bullet_player.png", gRenderer ) ) success = false;
    if( !gEnemyBulletTexture.loadFromFile( "bullet_enemy.png", gRenderer ) ) success = false;
    if( !gMagnifierTexture.loadFromFile( "magnifier.png", gRenderer ) ) success = false;

    // 載入音效
    gMusic = Mix_LoadMUS( "bgm.mp3" );
    gLaserSound = Mix_LoadWAV( "laser.wav" );
    gExplosionSound = Mix_LoadWAV( "explosion.wav" );
    gItemSound = Mix_LoadWAV( "item.wav" ); // 假設你有這個

    return success;
}

void close()
{
    // 釋放物件
    if(gpPlayer) delete gpPlayer;
    if(gpBoss) delete gpBoss;
    for(auto e : gEnemies) delete e;
    for(auto b : gBullets) delete b;
    for(auto i : gItems) delete i;

    // 釋放紋理
    gPlayerFront.free();
    gPlayerLeft.free();
    gPlayerRight.free();
    gEnemyTexture.free();
    gBossTexture.free();
    gBulletTexture.free();
    gEnemyBulletTexture.free();
    gMagnifierTexture.free();
    gMenuTexture.free();
    gInstructionTexture.free();
    gGameOverTexture.free();
    gScoreTextTexture.free();
    gHealthTextTexture.free();

    // 釋放音效
    Mix_FreeMusic( gMusic );
    Mix_FreeChunk( gLaserSound );
    Mix_FreeChunk( gExplosionSound );
    Mix_FreeChunk( gItemSound );

    // 關閉 SDL 子系統
    TTF_CloseFont( gFont );
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void resetGame()
{
    // 重置關卡數值
    gEnemiesDefeated = 0;
    gScore = 0;
    gLevelStage = STAGE_1;

    // 重置主角 (這裡假設 Player 類別有實作 reset)
    // 如果沒有，你也可以在這裡 delete gpPlayer; gpPlayer = new Player(...);
    if(gpPlayer) {
        // 簡單的方式：直接重置位置和血量
        // 如果你的 Player 類別沒有 reset()，請記得加上去
        // gpPlayer->reset();

        // 或者手動重置：
        delete gpPlayer;
        gpPlayer = new Player(&gPlayerFront, &gPlayerLeft, &gPlayerRight);
    }

    // 清空敵人、子彈、道具、Boss
    for(auto e : gEnemies) delete e;
    gEnemies.clear();

    for(auto b : gBullets) delete b;
    gBullets.clear();

    for(auto i : gItems) delete i;
    gItems.clear();

    if(gpBoss) {
        delete gpBoss;
        gpBoss = NULL;
    }
}
