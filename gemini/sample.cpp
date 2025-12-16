
#include <SDL2/SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>

#include "header/LTexture.h"
#include "header/player.h"
#include "header/enemy.h"
#include "header/boss.h"
#include "header/bullet.h"
#include "header/magnifier.h"
#include "header/heal.h"
#include "header/shield.h"
#include "header/PatternHelper.h"


// --- ����`�� ---
const int SCREEN_WIDTH = 1600;
const int SCREEN_HEIGHT = 900;

// --- �C�����A ---
enum GameState {
    STATE_MENU,
    STATE_TUTORIAL, // ���N�쥻�� INSTRUCTION
    STATE_PLAYING,
    STATE_GAME_OVER,
    STATE_WIN
};

enum TutorialStep {
    TUT_INTRO_1,
    TUT_INTRO_2,
    TUT_INTRO_3,      // 1. ��ܺ��F�P��1-3�q��r
    TUT_ENEMY_ACTION,    // 2. �X�{�ĤH�����a��
    TUT_ENEMY_TEXT,      // 3. ��������ܲĤG�q��r
    TUT_ITEM_MAG_ACTION, // 4. �X�{��j��
    TUT_ITEM_MAG_TEXT,   // 5. �Y�����ܲĤT�q��r
    TUT_ITEM_HEAL_ACTION,// 6. �X�{�ɦ�
    TUT_FINAL_TEXT       // 7. �̫��r -> �i�C��
};

enum LevelStage {
    STAGE_1,
    BOSS_1_FIGHT,
    STAGE_2,
    BOSS_2_FIGHT,
    STAGE_3,
    BOSS_3_FIGHT
};

// --- �����ܼ� (SDL ����) ---
SDL_Window* gWindow = NULL;
SDL_Renderer* gRenderer = NULL;
TTF_Font* gFont = NULL;

// --- �����ܼ� (���z�귽) ---
LTexture gMenuTexture;
LTexture gInstructionTexture;
LTexture gGameOverTexture;
LTexture gWinTexture;
LTexture gBGTexture; // �I����
TutorialStep gTutStep = TUT_INTRO_1;
LTexture gHelperTexture; // �O�o�ǳƤ@�i���F�Ϥ� helper.png

bool gTutObjectSpawned = false; // �ΨӽT�O�оǪ���u�ͦ��@��

// ����P�D�㯾�z
LTexture gPlayerFront, gPlayerLeft, gPlayerRight;
LTexture gEnemyTexture1;
LTexture gEnemyTexture2;
LTexture gEnemyTexture3;
LTexture gBossTexture1;
LTexture gBossTexture2;
LTexture gBossTexture3;
LTexture gBulletTexture;      // �D���l�u
LTexture gEnemyBulletTexture; // �ĤH�l�u
LTexture gMagnifierTexture;   // �D��
LTexture gHealTexture;     //�^��
LTexture gShieldItemTexture;   // �D���
LTexture gShieldEffectTexture; // �S�Ĺ�
LTexture gHPIconTexture; // ��q�ϥ�

// UI ���z
LTexture gScoreTextTexture;
LTexture gHealthIconTexture;

// --- �����ܼ� (���ĸ귽) ---
Mix_Music *gMusic = NULL;
Mix_Chunk *gLaserSound = NULL;
Mix_Chunk *gExplosionSound = NULL;
Mix_Chunk *gItemSound = NULL;
Mix_Chunk *gHealSound = NULL;

// --- �����ܼ� (�C������޲z) ---
Player* gpPlayer = NULL;
Boss* gpBoss = NULL;

std::vector<Enemy*> gEnemies;
std::vector<Bullet*> gBullets;
std::vector<Magnifier*> gItems;
std::vector<Heal*> gHeals;
std::vector<ShieldItem*> gShields;


// --- �����ܼ� (�C���޿�) ---
GameState gCurrentState = STATE_MENU;
LevelStage gLevelStage = STAGE_1;
int gEnemiesDefeated = 0;
int gScore = 0;

// --- �禡�ŧi ---
bool init();
bool loadMedia();
void close();
void resetGame();
void renderTutorialBox(std::string text);

int main(int argc, char* args[])
{
    // 1. ��l�ƶüƺؤl
    srand((unsigned)time(NULL));

    // 2. ��l�� SDL
    if( !init() ) {
        printf( "Failed to initialize!\n" );
        return -1;
    }

    // 3. ���J�C��
    if( !loadMedia() ) {
        printf( "Failed to load media!\n" );
        return -1;
    }

    // 4. �إߥD������ (���ɯ��z�w���J)
    gpPlayer = new Player(&gPlayerFront, &gPlayerLeft, &gPlayerRight);

    // ����I������
    Mix_PlayMusic( gMusic, -1 );

    bool quit = false;
    SDL_Event e;
    SDL_Color textColor = { 255, 255, 255 }; // �զ��r

    // --- �C���D�j�� ---
    while( !quit )
    {
        // --- �ƥ�B�z ---
        while( SDL_PollEvent( &e ) != 0 )
        {
            if( e.type == SDL_QUIT ) quit = true;

            // �ھڪ��A�B�z����
            switch(gCurrentState)
            {
                case STATE_MENU:
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                        // resetGame(); // �����m�T�O���b
                        gCurrentState = STATE_TUTORIAL;
                        gTutStep = TUT_INTRO_1; // ���]�оǨB�J
                        gTutObjectSpawned = false;
                    }
                    break;

                case STATE_TUTORIAL:
                    // �p�G�O�b�u�ݤ�r�v�����q�A���U Enter �i�J�U�@�B
                    if (gTutStep == TUT_INTRO_1 || gTutStep == TUT_INTRO_2 || gTutStep == TUT_INTRO_3 || gTutStep == TUT_ENEMY_TEXT ||
                        gTutStep == TUT_ITEM_MAG_TEXT || gTutStep == TUT_FINAL_TEXT)
                    {
                        gpPlayer->stop();
                        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) {
                            // 切換步驟邏輯
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

                            // 重要：對話切換瞬間再次重置玩家狀態，並清空多餘的按鍵事件
                            gpPlayer->stop();
                            SDL_FlushEvents(SDL_KEYDOWN, SDL_KEYUP);
                        }
                    }
                    else // --- 行動階段 (例如 TUT_ENEMY_ACTION) ---
                    {
                        gpPlayer->handleEvent(e);
                        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                            std::vector<Bullet*> newShots = gpPlayer->fire(&gBulletTexture);
                            gBullets.insert(gBullets.end(), newShots.begin(), newShots.end());
                            Mix_PlayChannel(-1, gLaserSound, 0);
                        }
                    }
                    break;


                case STATE_PLAYING:
                    gpPlayer->handleEvent(e);

                    // �o�g�l�u

                    if (e.type == SDL_KEYDOWN)
                    {
                        std::vector<Bullet*> newShots; // �Ψӱ��l�u

                        // �ť���G�򥻮g�� (�t�D��ĪG)
                        if (e.key.keysym.sym == SDLK_SPACE) {
                            newShots = gpPlayer->fire(&gBulletTexture);
                            Mix_PlayChannel(-1, gLaserSound, 0);
                        }
                        // S ��GPattern S
                        else if (e.key.keysym.sym == SDLK_s&& e.key.repeat == 0) {
                            newShots = gpPlayer->fireSpecialS(&gBulletTexture);
                            Mix_PlayChannel(-1, gLaserSound, 0); // �Ϊ̴��@�ӭ���
                        }
                        // D ��GPattern D
                        else if (e.key.keysym.sym == SDLK_d&& e.key.repeat == 0) {
                            newShots = gpPlayer->fireSpecialD(&gBulletTexture);
                            Mix_PlayChannel(-1, gLaserSound, 0); // �Ϊ̴��@�ӭ���
                        }

                        // �Τ@�N���ͪ��l�u�[�J���� vector
                        if (!newShots.empty()) {
                            gBullets.insert(gBullets.end(), newShots.begin(), newShots.end());
                        }
                    }
                    // �Ȱ��Φ^���� (�i��)
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                        gCurrentState = STATE_MENU;
                    }
                    break;

                case STATE_GAME_OVER:
                case STATE_WIN:
                    if (e.type == SDL_KEYDOWN) {
                        if (e.key.keysym.sym == SDLK_r) { // R ����
                            resetGame();
                            gCurrentState = STATE_PLAYING;
                        }
                    }
                    break;
            }
        }

        // --- �޿��s (Update) ---
        if (gCurrentState == STATE_PLAYING)
        {
            // 1. ��s�D��
            gpPlayer->move();
            gpPlayer->updateScatterStatus();

            // 2. �ͦ��P��s�ĤH/Boss (���d�y�{)
            switch (gLevelStage)
            {
                case STAGE_1:
                    if (rand() % 60 == 0) { // �ͦ��ĤH�W�v
                        gEnemies.push_back(new Enemy(rand() % (SCREEN_WIDTH - 60), -60, &gEnemyTexture1, &gEnemyTexture2, &gEnemyTexture3));
                    }
                    if (gEnemiesDefeated >= 10) {
                        gLevelStage = BOSS_1_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, -100, &gBossTexture1, 1);
                    }
                    break;

                case BOSS_1_FIGHT:
                case BOSS_2_FIGHT:
                case BOSS_3_FIGHT:
                    // Boss �Ԥ��ͦ��p�L
                    break;

                case STAGE_2:
                    if (rand() % 55 == 0) { // �ͦ�����
                        gEnemies.push_back(new Enemy(rand() % (SCREEN_WIDTH - 60), -60, &gEnemyTexture1, &gEnemyTexture2, &gEnemyTexture3));
                    }
                    if (gEnemiesDefeated >= 25) { // �ֿn����
                        gLevelStage = BOSS_2_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, -100, &gBossTexture2, 2);
                    }
                    break;

                case STAGE_3:
                    if (rand() % 50 == 0) { // �ͦ�����
                        gEnemies.push_back(new Enemy(rand() % (SCREEN_WIDTH - 60), -60, &gEnemyTexture1, &gEnemyTexture2, &gEnemyTexture3));
                    }
                    if (gEnemiesDefeated >= 50) { // �ֿn����
                        gLevelStage = BOSS_3_FIGHT;
                        gpBoss = new Boss((SCREEN_WIDTH-100)/2, -100, &gBossTexture3, 2);
                    }
                    break;
            }

            // ��s Boss
            static float gBossSpinAngle = 0; // ���ۼu������
            static int gBossFireTimer = 0;   // �o�g�p�ɾ�

            if (gpBoss) {
                gpBoss->move();

                std::vector<Bullet*> bShots;

                // --- ���[�I�G�ھ����d�M�w�����Ҧ� ---

                // ���p A�GBoss 1 �ϥ��ª��g�k (�޿�g�b boss.cpp �̡A���حp�ɾ�)
                if (gLevelStage == BOSS_1_FIGHT) {
                    bShots = gpBoss->fire(&gEnemyBulletTexture);
                }
                // ���p B�GBoss 2 �ϥηs���u���ۼu�v (PatternHelper)
                else if (gLevelStage == BOSS_2_FIGHT) {
                    // ��s���� (���l�u����)
                    gBossSpinAngle += 0.1f;

                    // ��s�p�ɾ�
                    gBossFireTimer++;

                    // �]�w�o�g�W�v�G�C 10 �V�o�g�@�� (�Ʀr�V�p�g�V��)
                    if (gBossFireTimer > 10) {
                        gBossFireTimer = 0; // ���m�p�ɾ�

                        // ���o Boss �����I (���] Boss �Ϥ��e���� 100x100)
                        // �A�i�H�� gpBoss->getWidth() ���o���Ǽƭ�
                        int bx = gpBoss->getPosX() + 50;
                        int by = gpBoss->getPosY() + 80;

                        // �I�s�A���P�� Pattern
                        bShots = PatternHelper::createBossSpiral(bx, by, gBossSpinAngle, &gEnemyBulletTexture);
                    }
                }
                // ���p C�GBoss 3 �ϥηs���u����X���v�Ρu���u�v
                else if (gLevelStage == BOSS_3_FIGHT) {
                    gBossFireTimer++;
                    // �]�w�� 90 �V (�� 1.5 ��) �o�g�@���A�]���o�� Pattern �l�u�ܦh
                    if (gBossFireTimer > 150) {
                        gBossFireTimer = 0;

                        int bx = gpBoss->getPosX() + 50; // �վ�� Boss ����
                        int by = gpBoss->getPosY() + 80;

                        // --- �ק�o�̡G�I�s Pattern C ---
                        bShots = PatternHelper::createBossPatternC(bx, by, &gEnemyBulletTexture); // �ǤJ Boss 3 ���Ϥ�
                    }
                }

                // --- �N���ͪ��l�u�[�J���� vector ---
                if (!bShots.empty()) {
                    gBullets.insert(gBullets.end(), bShots.begin(), bShots.end());
                }

                // --- ���`�P�w (�O���쥻�޿�) ---
                if (gpBoss->isDead()) {
                    Mix_PlayChannel(-1, gExplosionSound, 0);
                    delete gpBoss;
                    gpBoss = NULL;

                    // ���m�����ܼơA�H�K�U�@�� Boss �X�Ө��׫ܩ_��
                    gBossSpinAngle = 0;
                    gBossFireTimer = 0;

                    if (gLevelStage == BOSS_1_FIGHT) {
                        gLevelStage = STAGE_2;
                    }
                    else if (gLevelStage == BOSS_2_FIGHT) {
                        gLevelStage = STAGE_3;
                    }
                    else if (gLevelStage == BOSS_3_FIGHT) {
                        gCurrentState = STATE_WIN;
                    }
                }
            }

            // 3. ��s�ĤH & �H�����_
            for (int i = 0; i < gEnemies.size(); i++) {
                gEnemies[i]->move();
                std::vector<Bullet*> eShots = gEnemies[i]->fire(&gEnemyBulletTexture);
                if (!eShots.empty()) gBullets.insert(gBullets.end(), eShots.begin(), eShots.end());

                if (gEnemies[i]->isOffScreen() || gEnemies[i]->isDead()) {
                    if (gEnemies[i]->isDead()) {
                        gEnemiesDefeated++;
                        gScore += 100;
                        Mix_PlayChannel(-1, gExplosionSound, 0);

                        // �����D�� (20% ���v)
                        if (rand() % 5 == 0) {
                             // �D��ͦ��b�ĤH���`��m
                             // �Y Magnifier �غc�l���ݭn�y�СA�h�� gItems.push_back(new Magnifier());
                             // �Y�ڭ̧�^�ݭn�y�Ъ�����:
                             gItems.push_back(new Magnifier(gEnemies[i]->getPosX(), gEnemies[i]->getPosY()));
                        }
                        // ����heals (25% ���v)
                        if (rand() % 4 == 0) {
                             // heal�ͦ��b�ĤH���`��m����
                             // �Y Heal �غc�l���ݭn�y�СA�h�� gHeals.push_back(new Heal());
                             // �Y�ڭ̧�^�ݭn�y�Ъ�����:
                             gHeals.push_back(new Heal(gEnemies[i]->getPosX()+30, gEnemies[i]->getPosY()-30));
                        }

                        if (rand() % 5 == 0) { // ���v�C�@�I����öQ
                            gShields.push_back(new ShieldItem(rand() % (SCREEN_WIDTH - 40), -40));
                        }
                    }
                    delete gEnemies[i];
                    gEnemies.erase(gEnemies.begin() + i);
                    i--;
                }
            }

            // 4. ��s�l�u�P�I���˴�
            for (int i = 0; i < gBullets.size(); i++) {
                gBullets[i]->move();
                SDL_Rect bRect = gBullets[i]->getCollider();
                bool hit = false;

                // �D���l�u vs �ĤH/Boss
                if (gBullets[i]->getIsFromPlayer()) {
                    // ���ĤH
                    for (auto& e : gEnemies) {
                        SDL_Rect eRect = e->getCollider();
                        if (SDL_HasIntersection(&bRect, &eRect)) {
                            e->takeDamage(1);
                            hit = true;
                            break;
                        }
                    }
                    // �� Boss
                    if (!hit && gpBoss) {
                        SDL_Rect bossRect = gpBoss->getCollider();
                        if (SDL_HasIntersection(&bRect, &bossRect)) {
                            gpBoss->takeDamage(1);
                            hit = true;
                        }
                    }
                }
                // �ĤH�l�u vs �D��
                else {
                    SDL_Rect pRect = gpPlayer->getCollider();
                    if (SDL_HasIntersection(&bRect, &pRect)) {
                        gpPlayer->takeDamage(1);
                        hit = true;
                        // �i�H�[���˭���
                    }
                }

                if (hit) gBullets[i]->setHit();

                if (gBullets[i]->isOffScreen() || !gBullets[i]->isAvailable()) {
                    delete gBullets[i];
                    gBullets.erase(gBullets.begin() + i);
                    i--;
                }
            }

            // 5.1 ��s�D��
            SDL_Rect pRect = gpPlayer->getCollider();
            for (int i = 0; i < gItems.size(); i++) {
                gItems[i]->move();
                SDL_Rect itemRect = gItems[i]->getCollider();

                // �Y��D��
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

            // 5.2 ��sheals
            //SDL_Rect pRect = gpPlayer->getCollider();
            for (int i = 0; i < gHeals.size(); i++) {
                gHeals[i]->move();
                SDL_Rect healRect = gHeals[i]->getCollider();

                // �Y��heal
                if (SDL_HasIntersection(&pRect, &healRect)) {
                    if ((gpPlayer->getHealth())<5)
                    {
                        gpPlayer->addHealth(1);
                    }
                    gHeals[i]->setTaken();
                    Mix_PlayChannel(-1, gHealSound, 0);
                }

                if (!gHeals[i]->isAvailable()) {
                    delete gHeals[i];
                    gHeals.erase(gHeals.begin() + i);
                    i--;
                }
            }

            for (size_t i = 0; i < gShields.size(); i++) {
                gShields[i]->move();
                gShields[i]->render();

                SDL_Rect pRect = gpPlayer->getCollider();
                SDL_Rect sRect = gShields[i]->getCollider();

                // �Y��D��
                if (SDL_HasIntersection(&pRect, &sRect)) {
                    gpPlayer->addShield(1); // �W�[ 1 �h���@�n

                    // ���񭵮� (�p�G�� item sound)
                    Mix_PlayChannel(-1, gItemSound, 0);

                    gShields[i]->setTaken();
                }
                if (!gShields[i]->isAvailable()) {
                    delete gShields[i];
                    gShields.erase(gShields.begin() + i);
                    i--;
                }
            }

            // 6. �ˬd�D���ͦ�
            if (gpPlayer->getHealth() <= 0) {
                gCurrentState = STATE_GAME_OVER;
            }
        }

        if (gCurrentState == STATE_TUTORIAL)
        {
            gpPlayer->move();

            // ��s�l�u (���l�u�i�H��)
            for (int i=0; i<gBullets.size(); i++) {
                 gBullets[i]->move();
                 // ... �o�̭n�[²�檺��ɲ����޿� ...
            }

            // --- �ھڱоǨB�J�B�z ---
            // --- �ھڱоǨB�J�B�z ---
            switch (gTutStep)
            {
                case TUT_ENEMY_ACTION:
                    // �ͦ��@���оǼĤH
                    if (!gTutObjectSpawned) {
                        // �ͦ����������b���� (stopAtMiddle = true)
                        gEnemies.push_back(new Enemy(SCREEN_WIDTH/2 - 30, -50, &gEnemyTexture1, &gEnemyTexture2, &gEnemyTexture3, true));
                        gTutObjectSpawned = true;
                    }

                    // ��s�ĤH�P�I��
                    if (!gEnemies.empty()) {
                        Enemy* e = gEnemies[0];
                        e->move();

                        // ���o�ĤH�I���c (�ѨM eRect error)
                        SDL_Rect eRect = e->getCollider();

                        // �l�u�I���P�w
                        for (int i = 0; i < gBullets.size(); i++) {
                            // ���o�l�u�I���c (�ѨM bRect error)
                            SDL_Rect bRect = gBullets[i]->getCollider();

                            if (SDL_HasIntersection(&bRect, &eRect)) {
                                e->takeDamage(1);
                                gBullets[i]->setHit(); // �l�u��������
                            }
                        }

                        // �ˬd�O�_���`
                        if (e->isDead()) {
                            delete e;
                            gEnemies.clear();
                            gTutStep = TUT_ENEMY_TEXT; // �i�J�U�@���q��r
                        }
                    }
                    break;

                case TUT_ITEM_MAG_ACTION:
                    if (!gTutObjectSpawned) {
                        // �ͦ���j��
                        gItems.push_back(new Magnifier(SCREEN_WIDTH/2 - 20, -50, true));
                        gTutObjectSpawned = true;
                    }

                     if (!gItems.empty()) {
                         gItems[0]->move();

                         // �w�q�I���c (�ѨM pRect, itemRect error)
                         SDL_Rect pRect = gpPlayer->getCollider();
                         SDL_Rect itemRect = gItems[0]->getCollider();

                         // �P�_�D���Y��
                         if (SDL_HasIntersection(&pRect, &itemRect)) {
                             gpPlayer->activateScatter(); // �Y�D��ĪG
                             delete gItems[0];
                             gItems.clear();
                             gTutStep = TUT_ITEM_MAG_TEXT; // �U�@�B
                         }
                     }
                    break;

                case TUT_ITEM_HEAL_ACTION:
                    if (!gTutObjectSpawned) {
                        // �ͦ��ɦ� (stopAtMiddle = true)
                        gHeals.push_back(new Heal(SCREEN_WIDTH/2 - 20, -50, true));
                        gTutObjectSpawned = true;
                    }

                     if (!gHeals.empty()) {
                         gHeals[0]->move();

                         // �w�q�I���c
                         SDL_Rect pRect = gpPlayer->getCollider();
                         SDL_Rect healRect = gHeals[0]->getCollider();

                         if (SDL_HasIntersection(&pRect, &healRect)) {
                             // �ɦ��޿� (���]�A���g�o�Ө禡�A�Ϊ������q)
                             gpPlayer->addHealth(1);

                             delete gHeals[0];
                             gHeals.clear();
                             gTutStep = TUT_FINAL_TEXT; // �U�@�B
                         }
                     }
                     break;

                // �B�z���ϥΪ� case �H���� Warning
                case TUT_INTRO_1:
                case TUT_INTRO_2:
                case TUT_INTRO_3:
                case TUT_ENEMY_TEXT:
                case TUT_ITEM_MAG_TEXT:
                case TUT_FINAL_TEXT:
                    break;
            }
        }

        // --- ��V (Render) ---

        // �M���e�� (�¦�I��)
        SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x00, 0xFF );
        SDL_RenderClear( gRenderer );

        if (gCurrentState == STATE_MENU) {
            gMenuTexture.render(0, 0);
        }
        else if (gCurrentState == STATE_TUTORIAL)
        {
            gBGTexture.render(0, 0); // �e�I��
            gpPlayer->render();      // �e�D��

            // �e�оǪ���
            for(auto e : gEnemies) e->render();
            for(auto b : gBullets) b->render();
            for(auto i : gItems) i->render();
            for(auto h : gHeals) h->render(); // �O�o�e�ɦ�]
            for(auto s : gShields) s->render();


            // �ھڨB�J�e UI ��ܮ�
            if (gTutStep == TUT_INTRO_1) {
                renderTutorialBox("�[���̡C\n�ثe�Ҧb��m�G�������q�l�p��֤ߤ��h"); // �Ĥ@�q��r
            }
            else if (gTutStep == TUT_INTRO_2) {
                renderTutorialBox("Your mission is to defend the galaxy."); // �ĤG�q��r
            }
            else if (gTutStep == TUT_INTRO_3) {
                renderTutorialBox("Use ARROW KEYS to move."); // �ĤT�q��r
            }
            else if (gTutStep == TUT_ENEMY_TEXT) {
                renderTutorialBox("Great! Use SPACE to shoot.");
            }
            else if (gTutStep == TUT_ITEM_MAG_TEXT) {
                renderTutorialBox("Items give you power!");
            }
            else if (gTutStep == TUT_FINAL_TEXT) {
                renderTutorialBox("Ready? Press Enter to Start!");
            }
        }
        else if (gCurrentState == STATE_PLAYING) {
            // �e�I�� (�p��)
            gBGTexture.render(0, 0);

            // �e����
            gpPlayer->render();
            if (gpBoss) gpBoss->render();
            for (auto e : gEnemies) e->render();
            for (auto b : gBullets) b->render();
            for (auto item : gItems) item->render();
            for (auto heal : gHeals) heal->render();
            for(auto s : gShields) s->render();

           // 1. �e UI ��r (���� HP ��ܡA�u�d Score �M Shield)
            std::stringstream uiText;
            uiText << "Score: " << gScore << "  Shield: " << gpPlayer->getShield();

            // ��ܤ�r (��b���W��)
            gScoreTextTexture.loadFromRenderedText(uiText.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(10, 10);

            // 2. �e HP �ϥ� (��b���U��)
            int hp = gpPlayer->getHealth(); // ���o���e��q
            int iconX = 10;                 // �_�l X �y�� (�����)
            int iconY = SCREEN_HEIGHT - 40; // �_�l Y �y�� (�����������W 40 pixel)
            int padding = 5;                // �ϥܤ��������Z

            for (int i = 0; i < hp; ++i)
            {
                // �p��C�ӹϥܪ� X �y��
                // �����G�_�l�I + �ĴX�� * (�Ϥ��e�� + ���Z)
                int renderX = iconX + i * (gHPIconTexture.getWidth() + padding);

                gHPIconTexture.render(renderX, iconY);
            }
        }
        else if (gCurrentState == STATE_GAME_OVER) {
            gGameOverTexture.render(0, 0);

            // ��̲ܳפ���
            std::stringstream finalScore;
            finalScore << "Final Score: " << gScore;
            gScoreTextTexture.loadFromRenderedText(finalScore.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 + 50);
        }
        else if (gCurrentState == STATE_WIN) {
            // ���]�A���ӧQ�ϡA�Y�S���N�@�� GameOver �ί¤�r
            if (gWinTexture.getWidth() > 0) gWinTexture.render(0, 0);
            else gGameOverTexture.render(0, 0);

            std::stringstream winText;
            winText << "YOU WIN! Score: " << gScore;
            gScoreTextTexture.loadFromRenderedText(winText.str().c_str(), textColor, gFont, gRenderer);
            gScoreTextTexture.render(SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2);
        }

        // ��s�ù�
        SDL_RenderPresent( gRenderer );
    }

    // ����귽�ðh�X
    close();
    return 0;
}

bool init()
{
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 ) return false;

    // �]�w�u�ʹL�o
    if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) ) {}

    gWindow = SDL_CreateWindow( "EEmission", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
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

    // ���J�r��
    gFont = TTF_OpenFont( "BoutiqueBitmap9x9_1.92.ttf", 28 );
    // if( gFont == NULL ) { printf("Failed to load font!\n"); success = false; }

    // ���J�����Ϥ� (�Цۦ�ǳƳo�ǹϤ�)
    if( !gMenuTexture.loadFromFile( "menu.png", gRenderer ) ) success = false;
    //if( !gInstructionTexture.loadFromFile( "instruction.png", gRenderer ) ) success = false;
    if( !gGameOverTexture.loadFromFile( "gameover.png", gRenderer ) ) success = false;
    gHelperTexture.loadFromFile("helper.png", gRenderer); // �зǳƳo�i��
    // gWinTexture.loadFromFile("win.png", gRenderer); // �i��
    // gBGTexture.loadFromFile("bg.png", gRenderer);   // �i��

    // ���J����Ϥ�
    if( !gPlayerFront.loadFromFile( "player.png", gRenderer ) ) success = false;
    if( !gPlayerLeft.loadFromFile( "player_left.png", gRenderer ) ) success = false;
    if( !gPlayerRight.loadFromFile( "player_right.png", gRenderer ) ) success = false;

    //if( !gEnemyTexture.loadFromFile( "enemy.png", gRenderer ) ) success = false;
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

    // ���J����
    gMusic = Mix_LoadMUS( "bgm.mp3" );
    gLaserSound = Mix_LoadWAV( "laser.wav" );
    gExplosionSound = Mix_LoadWAV( "explosion.wav" );
    gItemSound = Mix_LoadWAV( "item.wav" ); // ���]�A���o��
    gHealSound = Mix_LoadWAV( "heal.wav" );

    return success;
}

void close()
{
    // ���񪫥�
    if(gpPlayer) delete gpPlayer;
    if(gpBoss) delete gpBoss;
    for(auto e : gEnemies) delete e;
    for(auto b : gBullets) delete b;
    for(auto i : gItems) delete i;
    for(auto j : gHeals) delete j;

    // ���񯾲z
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
    for(auto s : gShields) delete s; // �M�� vector

    // ���񭵮�
    Mix_FreeMusic( gMusic );
    Mix_FreeChunk( gLaserSound );
    Mix_FreeChunk( gExplosionSound );
    Mix_FreeChunk( gItemSound );
    Mix_FreeChunk( gHealSound );

    // ���� SDL �l�t��
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
    // ���m���d�ƭ�
    gEnemiesDefeated = 0;
    gScore = 0;
    gLevelStage = STAGE_1;
    if(gpPlayer) {

        // �Ϊ̤�ʭ��m�G
        delete gpPlayer;
        gpPlayer = new Player(&gPlayerFront, &gPlayerLeft, &gPlayerRight);
    }

    // �M�żĤH�B�l�u�B�D��BBoss
    for(auto e : gEnemies) delete e;
    gEnemies.clear();

    for(auto b : gBullets) delete b;
    gBullets.clear();

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
    // 1. �e�b�z���¦�����
    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND); // �}�ҲV�X�Ҧ�
    SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 180); // �¦�Aalpha=180 (�b�z��)

    SDL_Rect fillRect = { 0, SCREEN_HEIGHT - 200, SCREEN_WIDTH, 200 }; // �U�� 200 pixel ��
    SDL_RenderFillRect(gRenderer, &fillRect);

    SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_NONE); // �����V�X�Ҧ�
    gHelperTexture.render(20, SCREEN_HEIGHT - gHelperTexture.getHeight() - 20);

    SDL_Color textColor = { 255, 255, 255 };
    LTexture tempText;
    tempText.loadFromRenderedText(text, textColor, gFont, gRenderer);
    tempText.render(200, SCREEN_HEIGHT - 150); // �վ��r�y��
    tempText.free();
}
