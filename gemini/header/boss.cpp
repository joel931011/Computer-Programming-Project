#include "boss.h"

Boss::Boss(int x, int y, LTexture* texture, int bossLevel)
    : Character(x, y, texture, 0, 0) // 血量和速度在下面設定
{
    mBossLevel = bossLevel;
    frameCounter = 0;
    state = ENTERING; // 一開始是進場狀態
    movingRight = true;

    // 根據 Boss 等級設定數值
    if (mBossLevel == 1) {
        mHealth = 50;  // Boss 1 血量
        mSpeed = 2;    // 移動速度
    } else {
        mHealth = 100; // Boss 2 血量更厚
        mSpeed = 3;    // 移動更快
    }
}

void Boss::move()
{
    // 狀態 1: 進場中 (從螢幕上方慢慢滑入)
    if (state == ENTERING)
    {
        mPosY += 2; // 慢慢往下降

        // 如果到達指定高度 (例如 y=50)，就開始戰鬥
        if (mPosY >= 50) {
            state = FIGHTING;
        }
    }
    // 狀態 2: 戰鬥中 (左右徘徊)
    else if (state == FIGHTING)
    {
        if (movingRight) {
            mPosX += mSpeed;
            // 碰到右邊界，改往左
            if (mPosX + mWidth > SCREEN_WIDTH) {
                movingRight = false;
            }
        } else {
            mPosX -= mSpeed;
            // 碰到左邊界，改往右
            if (mPosX < 0) {
                movingRight = true;
            }
        }
    }
}

std::vector<Bullet*> Boss::fire(LTexture* bulletTexture)
{
    std::vector<Bullet*> bullets;

    // 只有在戰鬥狀態才開槍
    if (state != FIGHTING) return bullets;

    frameCounter++;

    // 設定射擊頻率 (例如每 60 幀射一次)
    int fireRate = (mBossLevel == 1) ? 60 : 40; // Boss 2 射比較快

    if (frameCounter >= fireRate)
    {
        frameCounter = 0;

        // 子彈生成點 (Boss 中心)
        int bX = mPosX + (mWidth / 2) - (Bullet::BULLET_WIDTH / 2);
        int bY = mPosY + mHeight;

        // --- 扇形攻擊 (散射) ---
        // 發射 3 發子彈 (左斜、中、右斜)

        int bulletSpeed = 6;

        // 1. 中間
        bullets.push_back(new Bullet(bX, bY, 0, bulletSpeed, bulletTexture, false));
        // 2. 左斜
        bullets.push_back(new Bullet(bX, bY, -3, bulletSpeed, bulletTexture, false));
        // 3. 右斜
        bullets.push_back(new Bullet(bX, bY, 3, bulletSpeed, bulletTexture, false));

        // 如果是 Boss 2，多射兩發更斜的
        if (mBossLevel == 2) {
            bullets.push_back(new Bullet(bX, bY, -6, bulletSpeed, bulletTexture, false));
            bullets.push_back(new Bullet(bX, bY, 6, bulletSpeed, bulletTexture, false));
        }
    }

    return bullets;
}
