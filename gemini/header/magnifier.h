#ifndef MAGNIFIER_H_INCLUDED
#define MAGNIFIER_H_INCLUDED

#include "LTexture.h"
#include "character.h" //
#include <chrono>
#include <vector>
#include <stdlib.h>
#include <time.h>


extern LTexture gMagnifierTexture;

extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

class Magnifier
{
public:
    // 建構子：設定生成的 X, Y 座標
    Magnifier(int x, int y, bool stopAtMiddle = false);

    // 解構子 (如果未來有動態配置資源要在這裡釋放，目前可以留空)
    ~Magnifier();

    // 道具的尺寸
    static const int MAGNIFIER_WIDTH = 40;
    static const int MAGNIFIER_HEIGHT = 40;

    // 移動邏輯 (單純向下移動)
    void move();

    // 渲染邏輯
    void render();

    // 檢查道具是否還有效 (若飛出螢幕或被吃掉則為 false)
    bool isAvailable() const { return available; }

    // 當道具被吃到時呼叫此函式
    void setTaken() { available = false; }

    // 獲取道具碰撞箱 (用於碰撞檢測)
    SDL_Rect getCollider();

private:
    // 因為不繼承 Character，所以要有自己的座標
    int mPosX, mPosY;

    // 垂直移動速度
    int mVelY;

    // 標記道具是否存活
    bool available;
    bool mStopAtMiddle;

};

#endif // MAGNIFIER_H_INCLUDED
