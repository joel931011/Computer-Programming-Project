#include <iostream>
#include <cmath>

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

// ===================== Bullet =====================
class Bullet : public Vec {
private:
    float x, y;
    bool active;
    int belong;
	float damage;
	float radius;
public:
    Bullet() : Vec(), x(0), y(0), active(false),belong(0),damage(0),radius(0) {}
    void fire(float startX, float startY, float v, float theta ,int belo,float dam,float r) {
        x = startX;
        y = startY;
        setSpeed(3*v * cos(theta), 3*v * sin(theta)); // 保留原本倍率
        active = true;
        belong=belo;
        damage=dam;
        radius=r;
    }

    void update(float dt) {
        if (!active) return;
        x += getVx() * dt;
        y += getVy() * dt;
    }

    float getX() const { return x; }
    float getY() const { return y; }
    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }
    int getBelong() const { return belong; }
    float getDamage() const { return damage; }
    float getRadius() const { return radius; }
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
    void shootBullet(float x, float y, float v, float theta ,int c,float d,float e) {
        if (freeCount == 0) return;
        int idx = freeList[--freeCount];
        bullets[idx].fire(x, y, v, theta,c,d,e);
        activeIndices[activeCount++] = idx;
    }

    void updateBullets(float dt, float screenWidth = 1920, float screenHeight = 1080) {
        int i = 0;
        while (i < activeCount) {
            int idx = activeIndices[i];
            bullets[idx].update(dt);

            // 超出邊界就回收
            if (bullets[idx].getX() < 0 || bullets[idx].getX() > screenWidth ||
                bullets[idx].getY() < 0 || bullets[idx].getY() > screenHeight) {
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

    // ===================== 射擊模式 =====================
    void shootBulletleft(float playerX, float playerY,int c,float d,float e) { shootBullet(playerX, playerY, 100.0f, PI,c,d,e); bulletnum++; }
    void shootBulletright(float playerX, float playerY,int c,float d,float e) { shootBullet(playerX, playerY, 100.0f,0.0,c,d,e); bulletnum++; }
    void shootBulletup(float a, float b,int c,float d,float e) { shootBullet(a, b, 100.0f, -PI / 2,c,d,e); bulletnum++; }
    void shootBulletdown(float a, float b,int c,float d,float e) { shootBullet(a, b, 100.0f, PI / 2,c,d,e); }

    void shootBulletcombo(float a, float b,int c,float d,float e) {
        shootBullet(a, b, 100.0f, -PI / 2,c,d,e); bulletnum++;
        shootBullet(a, b, 100.0f, -PI / 2 + PI / 8,c,d,e); bulletnum++;
        shootBullet(a, b, 100.0f, -PI / 2 - PI / 8,c,d,e); bulletnum++;
    }

    void shootBulletbossa(float a, float b,int f,float d,float e) {
        float c = 0;
        for (float aa = -PI / 2; aa <= 9 * PI / 2.0; aa += PI / 10) { shootBullet(a, b, 150.0f - c, aa,f,d,e); c += 2; }
    }

    void shootBulletbossb(float a, float b,int f,float d,float e) {
        float c = 0;
        for (float aa = -PI / 2; aa <= PI / 2.0; aa += PI / 40) { shootBullet(a, b, 150.0f - c, aa,f,d,e); c += 1; }
        for (float aa = PI / 2; aa >= -3 * PI / 2.0; aa -= PI / 40) { shootBullet(a, b, 150.0f - c, aa,f,d,e); c += 1; }
    }

    void shootBulletbossc(float a, float b,int f,float g,float e) {
        float d = 0;
        for (float aa = -PI / 2; aa <= PI / 2.0; aa += PI / 40) { shootBullet(a, b, (120.0f - d) * 5, aa,f,g,e); d += 1; }
        for (float aa = PI / 2; aa >= -3 * PI / 2.0; aa -= PI / 40) { shootBullet(a, b, (80.0f - d) * 5, aa,f,g,e); d += 1; }
    }

    void shootBulletbossd(float a, float b,int c,float d,float f) {
        float e = 0;
        for (float aa = PI / 2; aa >= -5 * PI / 2.0; aa -= PI / 40) { shootBullet(a, b, (110.0f - e), aa,c,d,f); e += 0.65; }
    }

    void shootBulletbosse(float a, float b,int c,float d,float e) {
        float f = 1;
        for (float aa = PI / 2; aa >= -5 * PI / 2.0; aa -= PI / 40) { shootBullet(a, b, (110.0f - f*f), aa,c,d,e); f += 0.1; }
    }
};

int BulletManager::bulletnum = 0;
int main()
{
	return 0;
}

