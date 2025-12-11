#include <stdio.h>
#include "heal.h"
#include <ctime>
#include <iostream>
#include <cmath>


// 移除原本內部的隨機邏輯，改成使用傳入的 x, y
Heal::Heal(int x, int y, bool stopAtMiddle)
{
    mPosX = x;
    mPosY = y;

    mVelY = 2; // 固定掉落速度
    available = true;
    mStopAtMiddle = stopAtMiddle; // 記錄
}

Heal::~Heal()
{
    // 如果沒有動態配置記憶體(new)，這裡可以留空
}

void Heal::render()
{
    // 只有在道具有效時才繪製
    if (available)
    {
        gHealTexture.render(mPosX, mPosY);
    }
}

void Heal::move()
{
    if (mStopAtMiddle && mPosY >= 200) { // 道具停低一點方便吃
        // 停住
    } else {
        mPosY += mVelY;
    }

    // 邊界檢查：如果掉出螢幕下方，就標記為無效
    if (mPosY > SCREEN_HEIGHT)
    {
        available = false;
    }
}


SDL_Rect Heal::getCollider()
{
    SDL_Rect r;
    r.x = mPosX;
    r.y = mPosY;
    r.w = HEAL_WIDTH;
    r.h = HEAL_HEIGHT;
    return r;
}

