#ifndef BULLET_H
#define BULLET_H

#include <SDL.h>
#include "LTexture.h" // 引用你的紋理類別

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

class Bullet
{
public:
    // 建構子：傳入起始位置(x, y)、速度向量(velX, velY)、以及子彈圖片
    Bullet(int x, int y, int velX, int velY, LTexture* texture, bool isFromPlayer);

    // 子彈大小 (假設是 20x20，你可以依圖片調整)
    static const int BULLET_WIDTH = 20;
    static const int BULLET_HEIGHT = 20;

    // 移動
    void move();

    // 渲染
    void render();

    // 檢查子彈是否還在螢幕內 (若飛出去則回傳 true -> 準備刪除)
    bool isOffScreen();

    // 取得碰撞箱
    SDL_Rect getCollider();

    // 標記是否有效 (打中敵人後設為 false)
    bool isAvailable() const { return available; }
    void setHit() { available = false; }
    bool getIsFromPlayer() const { return isPlayerBullet; }

private:
    int mPosX, mPosY;
    int mVelX, mVelY;

    bool available;     // 標記子彈是否還存活
    bool isPlayerBullet; // true = 主角射的, false = 敵人射的
    LTexture* mTexture; // 子彈圖片指標
};

#endif
