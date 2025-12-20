#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>

// ★★★ 引用您的子彈管理器標頭檔 ★★★
// 請確認您的 bullets and vectors 程式已經存成 "BulletManager.h" 並放在同層目錄
#include "BulletManager.h" 

#include "header/LTexture.h"
#include "header/player.h"
#include "header/enemy.h"
#include "header/boss.h"
// #include "header/bullet.h"        // [已刪除] 避免衝突
#include "header/magnifier.h"
#include "header/heal.h"
#include "header/shield.h"
// #include "header/PatternHelper.h" // [已刪除] 改用 Manager

// --- 全域常數 ---
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;

// --- 遊戲狀態 ---
enum GameState {
    STATE_MENU,
    STATE_TUTORIAL, 
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_WIN
};

enum TutorialStep {
    TUT_INTRO_1,
    TUT_INTRO_2,
    TUT_INTRO_3,      
    TUT_ENEMY_ACTION,    
    TUT_ENEMY_TEXT,      
    TUT_ITEM_MAG_ACTION, 
    TUT_ITEM_MAG_TEXT,   
    TUT_ITEM_HEAL_ACTION,
    TUT_FINAL_TEXT        
};

enum LevelStage {
    STAGE_1,
    BOSS_1_FIGHT,
    STAGE_2,
    BOSS_2_FIGHT,
    STAGE_3,
    BOSS_3_FIGHT
};

// --- SDL 變數 ---
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;

// --- 紋理資源 ---
LTexture gMenuTexture;
LTexture gInstructionTexture;
LTexture gGameOverTexture;
LTexture gWinTexture;
LTexture gBGTexture; 
TutorialStep gTutStep = TUT_INTRO_1;
LTexture gHelperTexture; 
bool gTutObjectSpawned = false; 

// 角色與敵人紋理
LTexture gPlayerFront, gPlayerLeft, gPlayerRight;
LTexture gEnemyTexture1, gEnemyTexture2, gEnemyTexture3;
LTexture gBossTexture1, gBossTexture2, gBossTexture3;
LTexture gBulletTexture;      
LTexture gEnemyBulletTexture; 
LTexture gMagnifierTexture;   
LTexture gHealTexture;     
LTexture gShieldItemTexture;   
LTexture gShieldEffectTexture; 
LTexture gHPIconTexture; 

// UI 紋理
LTexture gScoreTextTexture;
LTexture gHealthIconTexture;

// --- 音效資源 ---
Mix_Music *gMusic = NULL;
Mix_Chunk *gLaserSound = NULL;
Mix_Chunk *gExplosionSound = NULL;
Mix_Chunk *gItemSound = NULL;
Mix_Chunk *gHealSound = NULL;

// --- 遊戲物件管理 ---
Player* gpPlayer = NULL;
Boss* gpBoss = NULL;

std::vector<Enemy*> gEnemies;
// std::vector<Bullet*> gBullets; // [已移除] 改用下方的 gBulletManager

std::vector<Magnifier*> gItems;
std::vector<Heal*> gHeals;
std::vector<ShieldItem*> gShields;

// --- 遊戲變數 ---
GameState gCurrentState = STATE_MENU;
LevelStage gLevelStage = STAGE_1;
int gEnemiesDefeated = 0;
int gScore = 0;

// --- 函式宣告 ---
bool init();
bool loadMedia();
void close();
void resetGame();
void renderTutorialBox(std::string text);

// ★★★ 宣告子彈管理器 (全域變數) ★★★
BulletManager gBulletManager;

int main(int argc, char* args[])
{
    // 1. 初始化
    srand((unsigned)time(NULL));

    if( !init() ) { printf( "Failed to initialize!\n" ); return -1; }
    if( !loadMedia() ) { printf( "Failed to load media!\n" ); return -1; }

    gpPlayer = new Player(&gPlayerFront, &gPlayerLeft, &gPlayerRight);
    Mix_PlayMusic( gMusic, -1 );

    bool quit = false;
    SDL_Event e;
    SDL_Color textColor = { 255, 255, 255 }; 
    float dt = 0.016f; // 假設 60FPS

    // --- 遊戲主迴圈 ---
    while( !quit )
    {
        // --- (A) 事件處理 ---
        while( SDL_PollEvent( &e ) != 0 )
        {
            if( e.type == SDL_QUIT ) quit = true;

            switch(gCurrentState)
            {
                case STATE_MENU:
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                        gCurrentState = STATE_TUTORIAL;
                        gTutStep = TUT_INTRO_1; 
                        gTutObjectSpawned = false;
                    }
                    // Debug 跳關快速鍵
                    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_1) {
                        resetGame(); gCurrentState = STATE_PLAYING; gLevelStage = BOSS_1_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, 50, &gBossTexture1, 1);
                    }
                    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_2) {
                        resetGame(); gCurrentState = STATE_PLAYING; gLevelStage = BOSS_2_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, 50, &gBossTexture2, 2);
                    }
                    else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_3) {
                        resetGame(); gCurrentState = STATE_PLAYING; gLevelStage = BOSS_3_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, 50, &gBossTexture3, 2);
                    }
                    break;

                case STATE_TUTORIAL:
                    if (gTutStep == TUT_INTRO_1 || gTutStep == TUT_INTRO_2 || gTutStep == TUT_INTRO_3 || gTutStep == TUT_ENEMY_TEXT ||
                        gTutStep == TUT_ITEM_MAG_TEXT || gTutStep == TUT_FINAL_TEXT)
                    {
                        gpPlayer->stop();
                        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                            if (gTutStep == TUT_INTRO_1) gTutStep = TUT_INTRO_2;
                            else if (gTutStep == TUT_INTRO_2) gTutStep = TUT_INTRO_3;
                            else if (gTutStep == TUT_INTRO_3) gTutStep = TUT_ENEMY_ACTION;
                            else if (gTutStep == TUT_ENEMY_TEXT) gTutStep = TUT_ITEM_MAG_ACTION;
                            else if (gTutStep == TUT_ITEM_MAG_TEXT) gTutStep = TUT_ITEM_HEAL_ACTION;
                            else if (gTutStep == TUT_FINAL_TEXT) {
                                resetGame();
                                gCurrentState = STATE_PLAYING;
                            }
                            gTutObjectSpawned = false;
                            gpPlayer->stop();
                            SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
                        }
                    }
                    else 
                    {
                        gpPlayer->handleEvent(e);
                        // [修改] 教學模式射擊 (使用 BulletManager)
                        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                             // camp=0 (玩家), dmg=1, radius=10
                             gBulletManager.shootBulletup(gpPlayer->getPosX()+30, gpPlayer->getPosY(), 0, 1.0f, 10.0f, &gBulletTexture);
                             Mix_PlayChannel(-1, gLaserSound, 0);
                        }
                    }
                    break;

                case STATE_PLAYING:
                    gpPlayer->handleEvent(e);

                    if (e.type == SDL_KEYDOWN)
                    {
                        // [修改] 玩家射擊邏輯替換為 gBulletManager
                        float px = gpPlayer->getPosX() + 30; // 修正發射位置
                        float py = gpPlayer->getPosY();

                        // 空白鍵: 普通射擊 (Camp 0)
                        if (e.key.keysym.sym == SDLK_SPACE) {
                            gBulletManager.shootBulletup(px, py, 0, 1.0f, 10.0f, &gBulletTexture);
                            Mix_PlayChannel(-1, gLaserSound, 0);
                        }
                        // S 鍵: 散射 (Combo)
                        else if (e.key.keysym.sym == SDLK_s && e.key.repeat == 0) {
                            gBulletManager.shootBulletcombo(px, py, 0, 1.0f, 10.0f, &gBulletTexture);
                            Mix_PlayChannel(-1, gLaserSound, 0); 
                        }
                        // D 鍵: 特殊射擊 (這裡用 Combo 示範)
                        else if (e.key.keysym.sym == SDLK_d && e.key.repeat == 0) {
                            gBulletManager.shootBulletcombo(px, py, 0, 1.0f, 10.0f, &gBulletTexture);
                            Mix_PlayChannel(-1, gLaserSound, 0); 
                        }
                    }
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                        gCurrentState = STATE_MENU;
                    }
                    break;

                case STATE_GAME_OVER:
                case STATE_WIN:
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) { 
                        resetGame();
                        gCurrentState = STATE_PLAYING;
                    }
                    break;
            }
        }

        // --- (B) 邏輯更新 (Update) ---
        if (gCurrentState == STATE_PLAYING)
        {
            gpPlayer->move();
            gpPlayer->updateScatterStatus();

            // 1. 關卡生成邏輯
            switch (gLevelStage)
            {
                case STAGE_1:
                    if (rand() % 60 == 0) { 
                        gEnemies.push_back(new Enemy(rand() % (SCREEN_WIDTH - 60), -60, &gEnemyTexture1, &gEnemyTexture2, &gEnemyTexture3));
                    }
                    if (gEnemiesDefeated >= 10) {
                        gLevelStage = BOSS_1_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, 50, &gBossTexture1, 1);
                    }
                    break;
                case BOSS_1_FIGHT: case BOSS_2_FIGHT: case BOSS_3_FIGHT: break;
                case STAGE_2:
                    if (rand() % 55 == 0) { 
                        gEnemies.push_back(new Enemy(rand() % (SCREEN_WIDTH - 60), -60, &gEnemyTexture1, &gEnemyTexture2, &gEnemyTexture3));
                    }
                    if (gEnemiesDefeated >= 25) { 
                        gLevelStage = BOSS_2_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, 50, &gBossTexture2, 2);
                    }
                    break;
                case STAGE_3:
                    if (rand() % 50 == 0) { 
                        gEnemies.push_back(new Enemy(rand() % (SCREEN_WIDTH - 60), -60, &gEnemyTexture1, &gEnemyTexture2, &gEnemyTexture3));
                    }
                    if (gEnemiesDefeated >= 50) { 
                        gLevelStage = BOSS_3_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, 50, &gBossTexture3, 2);
                    }
                    break;
            }

            // 2. Boss 邏輯 (包含射擊)
            static float gBossSpinAngle = 0; 
            static int gBossFireTimer = 0;   

            if (gpBoss) {
                gpBoss->updateShieldState(0.016f);
                gpBoss->move();

                if (gLevelStage == BOSS_1_FIGHT) {
                        gBossSpinAngle += 0.01f; 
                        gBossFireTimer++;
                        if (gBossFireTimer > 10) {
                            gBossFireTimer = 0;
                            // 呼叫 Manager 的 Boss A 模式
                            gBulletManager.shootBulletbossa(gpBoss->getPosX()+50, gpBoss->getPosY()+50, 1, 1.0f, 10.0f, &gEnemyBulletTexture);
                        }
                }
                else if (gLevelStage == BOSS_2_FIGHT) {
                    gBossSpinAngle += 0.1f;
                    gBossFireTimer++;
                    if (gBossFireTimer > 10) {
                        gBossFireTimer = 0; 
                        // 呼叫 Manager 的 Boss B 模式
                        gBulletManager.shootBulletbossb(gpBoss->getPosX()+50, gpBoss->getPosY()+80, 1, 1.0f, 10.0f, &gEnemyBulletTexture);
                    }
                }
                else if (gLevelStage == BOSS_3_FIGHT) {
                    gBossFireTimer++;
                    if (gBossFireTimer > 150) {
                        gBossFireTimer = 0;
                        // 呼叫 Manager 的 Boss C 模式
                        gBulletManager.shootBulletbossc(gpBoss->getPosX()+50, gpBoss->getPosY()+80, 1, 1.0f, 10.0f, &gEnemyBulletTexture);
                    }
                }

                if (gpBoss->isDead()) {
                    Mix_PlayChannel(-1, gExplosionSound, 0);
                    delete gpBoss;
                    gpBoss = NULL;
                    gBossSpinAngle = 0;
                    gBossFireTimer = 0;
                    if (gLevelStage == BOSS_1_FIGHT) gLevelStage = STAGE_2;
                    else if (gLevelStage == BOSS_2_FIGHT) gLevelStage = STAGE_3;
                    else if (gLevelStage == BOSS_3_FIGHT) gCurrentState = STATE_WIN;
                }
            }

            // 3. 敵人移動 & 射擊
            for (int i = 0; i < gEnemies.size(); i++) {
                gEnemies[i]->move();
                
                // [修改] 敵人隨機射擊 (使用 gBulletManager)
                // 模擬 2% 機率射擊，陣營 Camp=1
                if (rand() % 100 < 2) { 
                    gBulletManager.shootBulletdown(gEnemies[i]->getPosX()+20, gEnemies[i]->getPosY()+40, 1, 1.0f, 10.0f, &gEnemyBulletTexture);
                }

                if (gEnemies[i]->isOffScreen() || gEnemies[i]->isDead()) {
                    if (gEnemies[i]->isDead()) {
                        gEnemiesDefeated++;
                        gScore += 100;
                        Mix_PlayChannel(-1, gExplosionSound, 0);
                        // 掉落道具邏輯
                        if (rand() % 2 == 0) {
                            int r = rand() % 3;
                            if(r == 0) gItems.push_back(new Magnifier(gEnemies[i]->getPosX(), gEnemies[i]->getPosY()));
                            else if(r == 1) gHeals.push_back(new Heal(gEnemies[i]->getPosX()+30, gEnemies[i]->getPosY()-30));
                            else gShields.push_back(new ShieldItem(rand() % (SCREEN_WIDTH - 40), -40));
                        }
                    }
                    delete gEnemies[i];
                    gEnemies.erase(gEnemies.begin() + i);
                    i--;
                }
            }

            // 4. ★★★ [核心修改] 更新子彈 & 碰撞判定 (使用 BulletManager) ★★★
            // 先讓子彈飛
            gBulletManager.updateBullets(dt, SCREEN_WIDTH, SCREEN_HEIGHT);

            // 遍歷所有活躍子彈進行碰撞檢查
            int activeCount = gBulletManager.getActiveCount();
            for (int i = 0; i < activeCount; i++) 
            {
                Bullet& b = gBulletManager.getBulletByIndex(i);
                SDL_Rect bRect = b.getCollider();
                bool hit = false;

                // 判斷是誰的子彈
                if (b.getIsFromPlayer()) { // 玩家子彈 (Camp 0)
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
                        // Boss 護盾範圍 (稍微大一點)
                        int padding = 40;
                        SDL_Rect shieldRect = { bossRect.x - padding, bossRect.y - padding, bossRect.w + (padding*2), bossRect.h + (padding*2) };
                        
                        if (SDL_HasIntersection(&bRect, &bossRect)) {
                            // Boss 反彈盾邏輯
                            if (gpBoss->isReflecting() && SDL_HasIntersection(&bRect, &shieldRect)) {
                                b.reverse(); // 反彈子彈
                                hit = false; // 不算擊中
                            } else {
                                gpBoss->takeDamage(1);
                                hit = true;
                                Mix_PlayChannel(-1, gExplosionSound, 0); // 擊中音效
                            }
                        }
                    }
                }
                else { // 敵人子彈 (Camp != 0)
                    SDL_Rect pRect = gpPlayer->getCollider();
                    if (SDL_HasIntersection(&bRect, &pRect)) {
                        gpPlayer->takeDamage(1);
                        hit = true;
                    }
                }

                if (hit) b.setActive(false); // 擊中後回收
            }

            // 5. 更新道具 (邏輯保持不變)
            SDL_Rect pRect = gpPlayer->getCollider();
            for(int i=0; i<gItems.size(); i++) {
                 gItems[i]->move();
                 SDL_Rect r = gItems[i]->getCollider();
                 if(SDL_HasIntersection(&pRect, &r)) { gpPlayer->activateScatter(); gItems[i]->setTaken(); Mix_PlayChannel(-1, gItemSound, 0); }
                 if(!gItems[i]->isAvailable()) { delete gItems[i]; gItems.erase(gItems.begin()+i); i--; }
            }
            for(int i=0; i<gHeals.size(); i++) {
                 gHeals[i]->move();
                 SDL_Rect r = gHeals[i]->getCollider();
                 if(SDL_HasIntersection(&pRect, &r)) { if(gpPlayer->getHealth()<5) gpPlayer->addHealth(1); gHeals[i]->setTaken(); Mix_PlayChannel(-1, gHealSound, 0); }
                 if(!gHeals[i]->isAvailable()) { delete gHeals[i]; gHeals.erase(gHeals.begin()+i); i--; }
            }
            for(int i=0; i<gShields.size(); i++) {
                 gShields[i]->move();
                 SDL_Rect r = gShields[i]->getCollider();
                 if(SDL_HasIntersection(&pRect, &r)) { gpPlayer->addShield(1); gShields[i]->setTaken(); Mix_PlayChannel(-1, gItemSound, 0); }
                 if(!gShields[i]->isAvailable()) { delete gShields[i]; gShields.erase(gShields.begin()+i); i--; }
            }

            if (gpPlayer->getHealth() <= 0) gCurrentState = STATE_GAME_OVER;
        }

        if (gCurrentState == STATE_TUTORIAL)
        {
            gpPlayer->move();

            // 教學模式下的子彈更新
            gBulletManager.updateBullets(dt, SCREEN_WIDTH, SCREEN_HEIGHT);
            
            // 教學模式碰撞邏輯 (使用 BulletManager)
            int activeCount = gBulletManager.getActiveCount();
            
            switch (gTutStep)
            {
                case TUT_ENEMY_ACTION:
                    if (!gTutObjectSpawned) {
                        gEnemies.push_back(new Enemy(SCREEN_WIDTH/2 - 30, -50, &gEnemyTexture1, &gEnemyTexture2, &gEnemyTexture3, true));
                        gTutObjectSpawned = true;
                    }
                    if (!gEnemies.empty()) {
                        Enemy* e = gEnemies[0];
                        e->move();
                        SDL_Rect eRect = e->getCollider();
                        
                        // 檢查玩家子彈是否打中教學敵人
                        for(int i=0; i<activeCount; i++) {
                            Bullet& b = gBulletManager.getBulletByIndex(i);
                            if(b.getIsFromPlayer()) {
                                SDL_Rect bRect = b.getCollider();
                                if (SDL_HasIntersection(&bRect, &eRect)) {
                                    e->takeDamage(1);
                                    b.setActive(false);
                                }
                            }
                        }

                        if (e->isDead()) {
                            delete e;
                            gEnemies.clear();
                            gTutStep = TUT_ENEMY_TEXT; 
                        }
                    }
                    break;

                case TUT_ITEM_MAG_ACTION:
                    if (!gTutObjectSpawned) {
                        gItems.push_back(new Magnifier(SCREEN_WIDTH/2 - 20, -50, true));
                        gTutObjectSpawned = true;
                    }
                     if (!gItems.empty()) {
                         gItems[0]->move();
                         SDL_Rect pRect = gpPlayer->getCollider();
                         SDL_Rect itemRect = gItems[0]->getCollider();
                         if (SDL_HasIntersection(&pRect, &itemRect)) {
                             gpPlayer->activateScatter(); 
                             delete gItems[0];
                             gItems.clear();
                             gTutStep = TUT_ITEM_MAG_TEXT; 
                         }
                     }
                    break;

                case TUT_ITEM_HEAL_ACTION:
                    if (!gTutObjectSpawned) {
                        gHeals.push_back(new Heal(SCREEN_WIDTH/2 - 20, -50, true));
                        gTutObjectSpawned = true;
                    }
                     if (!gHeals.empty()) {
                         gHeals[0]->move();
                         SDL_Rect pRect = gpPlayer->getCollider();
                         SDL_Rect healRect = gHeals[0]->getCollider();
                         if (SDL_HasIntersection(&pRect, &healRect)) {
                             gpPlayer->addHealth(1);
                             delete gHeals[0];
                             gHeals.clear();
                             gTutStep = TUT_FINAL_TEXT; 
                         }
                     }
                     break;

                case TUT_INTRO_1: case TUT_INTRO_2: case TUT_INTRO_3:
                case TUT_ENEMY_TEXT: case TUT_ITEM_MAG_TEXT: case TUT_FINAL_TEXT:
                    break;
            }
        }

        // --- (C) Render 繪圖 ---
        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear( gRenderer );

        if (gCurrentState == STATE_MENU) {
            gMenuTexture.render(0, 0);
        }
        else if (gCurrentState == STATE_TUTORIAL)
        {
            gBGTexture.render(0, 0); 
            gpPlayer->render();     
            for(auto e : gEnemies) e->render();
            
            // [修改] 繪製所有子彈 (使用 BulletManager)
            gBulletManager.renderAll(); 

            for(auto i : gItems) i->render();
            for(auto h : gHeals) h->render(); 
            for(auto s : gShields) s->render();

            if (gTutStep == TUT_INTRO_1) renderTutorialBox("Projection complete.\nIdentity confirmed ??Observer.\nCurrent location: Inner Layer of the Unstable Quantum Computing Core."); 
            else if (gTutStep == TUT_INTRO_2) renderTutorialBox("The core has been occupied by multiple magnetic field control enemies.\nThey are generating intense magnetic fields, distorting all electron flows.\nPress SPACE to fire high-energy electrons."); 
            else if (gTutStep == TUT_INTRO_3) renderTutorialBox("Warning:\nYou have only 5 lives remaining.\nEach collapse will reset the observation state."); 
            else if (gTutStep == TUT_ENEMY_TEXT) renderTutorialBox("Congratulations on defeating your first enemy.\nFor every five enemies eliminated, a Heisenberg Fragment will appear.");
            else if (gTutStep == TUT_ITEM_MAG_TEXT) renderTutorialBox("Embedding a Heisenberg Fragment activates the Uncertainty Effect.\nFired electrons will split into a probability distribution.");
            else if (gTutStep == TUT_FINAL_TEXT) renderTutorialBox("Alert:\nWhen enemy magnetic fields are generated,\nelectron beam trajectories will be deflected by the magnetic field,\ncausing your attacks to rebound.\nObserver, the fate of the core rests on your choices.");
        }
        else if (gCurrentState == STATE_PLAYING) {
            gBGTexture.render(0, 0);
            gpPlayer->render();
            if (gpBoss) gpBoss->render();
            for (auto e : gEnemies) e->render();
            
            // [修改] 繪製所有子彈 (使用 BulletManager)
            gBulletManager.renderAll(); 

            for (auto item : gItems) item->render();
            for (auto heal : gHeals) heal->render();
            for(auto s : gShields) s->render();

            std::stringstream uiText;
            uiText << "Score: " << gScore << "  Shield: " << gpPlayer->getShield();
            gScoreTextTexture.loadFromRenderedText(uiText.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(10, 10);

            int hp = gpPlayer->getHealth(); 
            for (int i = 0; i < hp; ++i) {
                gHPIconTexture.render(10 + i * (gHPIconTexture.getWidth() + 5), SCREEN_HEIGHT - 40);
            }
        }
        else if (gCurrentState == STATE_GAME_OVER) {
            gGameOverTexture.render(0, 0);
            std::stringstream finalScore;
            finalScore << "Final Score: " << gScore;
            gScoreTextTexture.loadFromRenderedText(finalScore.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 50);
        }
        else if (gCurrentState == STATE_WIN) {
            if (gWinTexture.getWidth() > 0) gWinTexture.render(0, 0);
            else gGameOverTexture.render(0, 0);
            std::stringstream winText;
            winText << "YOU WIN! Score: " << gScore;
            gScoreTextTexture.loadFromRenderedText(winText.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2);
        }

        SDL_RenderPresent( gRenderer );
    }

    close();
    return 0;
}

// --- 輔助函式定義 ---

bool init()
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ) return false;
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) ) {}
    gWindow = SDL_CreateWindow( "EEmission", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
    if( gWindow == NULL ) return false;
    gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
    if( gRenderer == NULL ) return false;
    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
    int imgFlags = IMG_INIT_PNG;
    if( !( IMG_Init( imgFlags ) & imgFlags ) ) return false;
    if( TTF_Init() == -1 ) return false;
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) return false;
    return true;
}

bool loadMedia()
{
    bool success = true;
    gFont = TTF_OpenFont( "BoutiqueBitmap9x9_1.92.ttf", 28 );
    if( !gMenuTexture.loadFromFile( "menu.png", gRenderer ) ) success = false;
    if( !gGameOverTexture.loadFromFile( "gameover.png", gRenderer ) ) success = false;
    gHelperTexture.loadFromFile("helper.png", gRenderer); 
    
    if( !gPlayerFront.loadFromFile( "player.png", gRenderer ) ) success = false;
    if( !gPlayerLeft.loadFromFile( "player_left.png", gRenderer ) ) success = false;
    if( !gPlayerRight.loadFromFile( "player_right.png", gRenderer ) ) success = false;

    if( !gEnemyTexture1.loadFromFile( "enemy_1.png", gRenderer ) ) success = false;
    if( !gEnemyTexture2.loadFromFile( "enemy_2.png", gRenderer ) ) success = false;
    if( !gEnemyTexture3.loadFromFile( "enemy_3.png", gRenderer ) ) success = false;
    if( !gBossTexture1.loadFromFile( "boss_1.png", gRenderer ) ) success = false;
    if( !gBossTexture2.loadFromFile( "boss_2.png", gRenderer ) ) success = false;
    if( !gBossTexture3.loadFromFile( "boss_3.png", gRenderer ) ) success = false;
    if( !gBulletTexture.loadFromFile( "bullet_player.png", gRenderer ) ) success = false;
    if( !gEnemyBulletTexture.loadFromFile( "bullet_enemy.png", gRenderer ) ) success = false;
    if( !gMagnifierTexture.loadFromFile( "magnifier.png", gRenderer ) ) success = false;
    if( !gHealTexture.loadFromFile( "heal.png", gRenderer ) ) success = false;
    if( !gShieldItemTexture.loadFromFile( "shield_item.png", gRenderer ) ) success = false;
    if( !gHPIconTexture.loadFromFile( "hp_icon.png", gRenderer ) ) success = false;
    if( !gShieldEffectTexture.loadFromFile( "shield_effect.png", gRenderer ) ) success = false;

    gMusic = Mix_LoadMUS( "bgm.mp3" );
    gLaserSound = Mix_LoadWAV( "laser.wav" );
    gExplosionSound = Mix_LoadWAV( "explosion.wav" );
    gItemSound = Mix_LoadWAV( "item.wav" ); 
    gHealSound = Mix_LoadWAV( "heal.wav" );

    return success;
}

void close()
{
    if(gpPlayer) delete gpPlayer;
    if(gpBoss) delete gpBoss;
    for(auto e : gEnemies) delete e;
    
    // gBullets 不需要 delete，因為 gBulletManager 是靜態陣列管理

    for(auto i : gItems) delete i;
    for(auto j : gHeals) delete j;

    gPlayerFront.free();
    gPlayerLeft.free();
    gPlayerRight.free();
    gEnemyTexture1.free();
    gEnemyTexture2.free();
    gEnemyTexture3.free();
    gBossTexture1.free();
    gBossTexture2.free();
    gBossTexture3.free();
    gBulletTexture.free();
    gEnemyBulletTexture.free();
    gMagnifierTexture.free();
    gHealTexture.free();
    gMenuTexture.free();
    gInstructionTexture.free();
    gGameOverTexture.free();
    gScoreTextTexture.free();
    gHealthIconTexture.free();
    gShieldItemTexture.free();
    gShieldEffectTexture.free();
    gHPIconTexture.free();
    for(auto s : gShields) delete s; 

    Mix_FreeMusic( gMusic );
    Mix_FreeChunk( gLaserSound );
    Mix_FreeChunk( gExplosionSound );
    Mix_FreeChunk( gItemSound );
    Mix_FreeChunk( gHealSound );

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
    gEnemiesDefeated = 0;
    gScore = 0;
    gLevelStage = STAGE_1;
    if(gpPlayer) {
        delete gpPlayer;
        gpPlayer = new Player(&gPlayerFront, &gPlayerLeft, &gPlayerRight);
    }

    for(auto e : gEnemies) delete e;
    gEnemies.clear();

    // ★★★ [修改] 重置子彈池 ★★★
    gBulletManager.reset(); 

    for(auto i : gItems) delete i;
    gItems.clear();
    for(auto j : gHeals) delete j;
    gHeals.clear();
    for(auto s : gShields) delete s;
    gShields.clear();

    if(gpBoss) {
        delete gpBoss;
        gpBoss = NULL;
    }
}

void renderTutorialBox(std::string text)
{
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND); 
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 180); 

    SDL_Rect fillRect = { 0, SCREEN_HEIGHT - 200, SCREEN_WIDTH, 200 }; 
    SDL_RenderFillRect(gRenderer, &fillRect);

    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_NONE); 
    gHelperTexture.render(20, SCREEN_HEIGHT - gHelperTexture.getHeight() - 20);

    SDL_Color textColor = { 255, 255, 255 };
    int startX = 200;
    int startY = SCREEN_HEIGHT - 150;

    std::stringstream ss(text);
    std::string line;
    while (std::getline(ss, line, '\n')) {
        if (line.empty()) {
            startY += 30; 
            continue;
        }
        LTexture tempText;
        if (tempText.loadFromRenderedText(line, textColor, gFont, gRenderer)) {
            tempText.render(startX, startY);
            startY += tempText.getHeight() + 10;
            tempText.free(); 
        }
    }
}
