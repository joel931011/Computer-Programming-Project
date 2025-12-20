#include <iostream>
#include <cmath>
#include <SDL2/SDL.h> // 2. 補上 SDL 標頭檔
#include "header/LTexture.h"
#define MAX_BULLETS 15000
#define PI 3.14159265

// ===================== Vec =====================
class Vec {
private:
    float vx, vy;

public:
    Vec(float vx = 0, float vy = 0) : vx(vx), vy(vy) {}
    void setSpeed(float sx, float sy) { vx = sx; vy = sy; }
    float getVx() const { return vx; }
    float getVy() const { return vy; }
    void reverse() { vx = -vx; vy = -vy; }
};

// ===================== Bullet (融合 SDL 繪圖功能) =====================
class Bullet : public Vec {
private:
    float x, y;
    bool active;
    int belong;    // 0: Player, 1: Enemy
    float damage;
    float radius;
    
    // --- 新增: SDL 繪圖需要的變數 ---
    LTexture* currentTexture; 

public:
    Bullet() : Vec(), x(0), y(0), active(false), belong(0), damage(0), radius(0), currentTexture(NULL) {}

    // 修改 fire: 多接收一個 LTexture* 參數
    void fire(float startX, float startY, float v, float theta, int belo, float dam, float r, LTexture* tex) {
        x = startX;
        y = startY;
        setSpeed(3 * v * cos(theta), 3 * v * sin(theta));
        active = true;
        belong = belo;
        damage = dam;
        radius = r;
        currentTexture = tex; // 設定這顆子彈的圖片
    }

    void update(float dt) {
        if (!active) return;
        x += getVx() * dt;
        y += getVy() * dt;
    }

    // --- 新增: 繪圖與碰撞 ---
    void render() {
        if (active && currentTexture != NULL) {
            // 將中心點座標 (x,y) 轉換為左上角座標繪製
            currentTexture->render((int)(x - radius), (int)(y - radius));
        }
    }

    SDL_Rect getCollider() {
        SDL_Rect rect;
        rect.x = (int)(x - radius);
        rect.y = (int)(y - radius);
        rect.w = (int)(radius * 2);
        rect.h = (int)(radius * 2);
        return rect;
    }

    // Getters
    float getX() const { return x; }
    float getY() const { return y; }
    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }
    int getBelong() const { return belong; }
    float getDamage() const { return damage; }
    float getRadius() const { return radius; }
    
    // 判斷是否為玩家子彈 (為了相容 main 的邏輯)
    bool getIsFromPlayer() const { return belong == 0; }
};

// ===================== Bullet Manager =====================
class BulletManager {
private:
    Bullet bullets[MAX_BULLETS];
    int activeIndices[MAX_BULLETS];
    int activeCount;
    int freeList[MAX_BULLETS];
    int freeCount;

public:
    static int bulletnum;

    BulletManager() : activeCount(0), freeCount(MAX_BULLETS) {
        for (int i = 0; i < MAX_BULLETS; i++)
            freeList[i] = MAX_BULLETS - 1 - i;
    }

    // O(1) 發射子彈
    void shootBullet(float x, float y, float v, float theta ,int c,float d,float e,LTexture* tex) {
        if (freeCount == 0) return;
        int idx = freeList[--freeCount];
        bullets[idx].fire(x, y, v, theta,c,d,e,tex);
        activeIndices[activeCount++] = idx;
    }

    // 負責更新所有子彈並檢查邊界
    void updateBullets(float dt, float screenWidth = 1600, float screenHeight = 900) {
        int i = 0;
        while (i < activeCount) {
            int idx = activeIndices[i];
            bullets[idx].update(dt);

            // 加上 radius 的寬容值邊界檢查
            float r = bullets[idx].getRadius();
            if (bullets[idx].getX() < -r || bullets[idx].getX() > screenWidth + r ||
                bullets[idx].getY() < -r || bullets[idx].getY() > screenHeight + r) {
                bullets[idx].setActive(false);
            }

            if (!bullets[idx].isActive()) {
                freeList[freeCount++] = idx;
                activeIndices[i] = activeIndices[--activeCount];
            } else i++;
        }
    }

    void reverseAllBullets() {
        for (int i = 0; i < activeCount; i++)
            bullets[activeIndices[i]].reverse();
    }

    int getActiveCount() const { return activeCount; }
    const Bullet& getBullet(int i) const { return bullets[activeIndices[i]]; }

    int getMaxBullets() const { return MAX_BULLETS; }
    // 新增: 繪製所有活躍子彈
    void renderAll() {
        for (int i = 0; i < activeCount; i++) {
            bullets[activeIndices[i]].render();
        }
    }
    Bullet& getBulletByIndex(int i) { return bullets[activeIndices[i]]; } // 回傳參考以便修改狀態
    // ===================== 射擊模式 =====================
    void shootBulletleft(float playerX, float playerY,int c,float d,float e,LTexture* tex) { shootBullet(playerX, playerY, 100.0f, PI,c,d,e, tex); bulletnum++; }
    void shootBulletright(float playerX, float playerY,int c,float d,float e,LTexture* tex) { shootBullet(playerX, playerY, 100.0f,0.0,c,d,e, tex); bulletnum++; }
    void shootBulletup(float a, float b,int c,float d,float e,LTexture* tex) { shootBullet(a, b, 100.0f, -PI / 2,c,d,e, tex); bulletnum++; }
    void shootBulletdown(float a, float b,int c,float d,float e,LTexture* tex) { shootBullet(a, b, 100.0f, PI / 2,c,d,e, tex); }

    void shootBulletcombo(float a, float b,int c,float d,float e,LTexture* tex) {
        shootBullet(a, b, 100.0f, -PI / 2,c,d,e, tex); bulletnum++;
        shootBullet(a, b, 100.0f, -PI / 2 + PI / 8,c,d,e, tex); bulletnum++;
        shootBullet(a, b, 100.0f, -PI / 2 - PI / 8,c,d,e, tex); bulletnum++;
    }

    void shootBulletbossa(float a, float b,int f,float d,float e,LTexture* tex) {
        float c = 0;
        for (float aa = -PI / 2; aa <= 9 * PI / 2.0; aa += PI / 10) { shootBullet(a, b, 150.0f - c, aa,f,d,e, tex); c += 2; }
    }

    void shootBulletbossb(float a, float b,int f,float d,float e,LTexture* tex) {
        float c = 0;
        for (float aa = -PI / 2; aa <= PI / 2.0; aa += PI / 40) { shootBullet(a, b, 150.0f - c, aa,f,d,e, tex); c += 1; }
        for (float aa = PI / 2; aa >= -3 * PI / 2.0; aa -= PI / 40) { shootBullet(a, b, 150.0f - c, aa,f,d,e, tex); c += 1; }
    }

    void shootBulletbossc(float a, float b,int f,float g,float e,LTexture* tex) {
        float d = 0;
        for (float aa = -PI / 2; aa <= PI / 2.0; aa += PI / 40) { shootBullet(a, b, (120.0f - d) * 5, aa,f,g,e, tex); d += 1; }
        for (float aa = PI / 2; aa >= -3 * PI / 2.0; aa -= PI / 40) { shootBullet(a, b, (80.0f - d) * 5, aa,f,g,e, tex); d += 1; }
    }

    void shootBulletbossd(float a, float b,int c,float d,float f,LTexture* tex) {
        float e = 0;
        for (float aa = PI / 2; aa >= -5 * PI / 2.0; aa -= PI / 40) { shootBullet(a, b, (110.0f - e), aa,c,d,f, tex); e += 0.65; }
    }

    void shootBulletbosse(float a, float b,int c,float d,float e,LTexture* tex) {
        float f = 1;
        for (float aa = PI / 2; aa >= -5 * PI / 2.0; aa -= PI / 40) { shootBullet(a, b, (110.0f - f*f), aa,c,d,e, tex); f += 0.1; }
    }
};

int BulletManager::bulletnum = 0;

