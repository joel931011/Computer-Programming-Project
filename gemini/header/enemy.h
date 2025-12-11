#ifndef ENEMY_H
#define ENEMY_H

#include "character.h"
#include "bullet.h"
#include <vector>

class Enemy : public Character
{
public:
    // 建構子：位置與紋理
    Enemy(int x, int y, LTexture* tex1, LTexture* tex2, LTexture* tex3, bool stopAtMiddle = false);

    // 覆寫移動：敵人單純往下
    void move() override;

    // 覆寫渲染：實作動畫與閃爍
    void render() override;

    // 覆寫受傷：觸發閃爍
    void takeDamage(int damage) override;

    // 嘗試射擊：回傳子彈 vector (如果還沒冷卻好，就回傳空的)
    std::vector<Bullet*> fire(LTexture* bulletTexture);

    // 檢查是否飛出螢幕 (用來刪除物件)
    bool isOffScreen();

private:
    // 用來計數每一幀，控制射擊頻率
    int frameCounter;

    // 射擊間隔 (例如每 100 幀射一次)
    int fireInterval;

    LTexture* mTex1;
    LTexture* mTex2;
    LTexture* mTex3;

    // 閃爍特效相關變數
    bool isHit;           // 是否正在受傷閃爍
    Uint32 hitStartTime;  // 開始閃爍的時間
    bool mStopAtMiddle;  //tutorial
};

#endif
