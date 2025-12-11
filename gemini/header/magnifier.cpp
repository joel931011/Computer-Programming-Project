//#include <SDL.h>
//#include <SDL_image.h>
#include <stdio.h>
//#include <string>
#include "magnifier.h"
#include <ctime>
#include <iostream>
#include <cmath>


// 移除原本內部的隨機邏輯，改成使用傳入的 x, y
Magnifier::Magnifier(int x, int y, bool stopAtMiddle)
{
    mPosX = x;
    mPosY = y;

    mVelY = 2; // 固定掉落速度
    available = true;
    mStopAtMiddle = stopAtMiddle; // 記錄
}

Magnifier::~Magnifier()
{
    // 如果沒有動態配置記憶體(new)，這裡可以留空
}

void Magnifier::render()
{
    // 只有在道具有效時才繪製
    if (available)
    {
        gMagnifierTexture.render(mPosX, mPosY);
    }
}

void Magnifier::move()
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


SDL_Rect Magnifier::getCollider()
{
    SDL_Rect r;
    r.x = mPosX;
    r.y = mPosY;
    r.w = MAGNIFIER_WIDTH;
    r.h = MAGNIFIER_HEIGHT;
    return r;
}

