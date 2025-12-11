#ifndef CHARACTER_H
#define CHARACTER_H

#include <SDL.h>
#include "LTexture.h" // 假設這是你的紋理類別
#include <vector>

// 宣告全域變數 (如果需要的話，但建議盡量減少依賴全域)
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

class Character
{
public:
    // 建構子：初始化座標、紋理、血量、速度
    // texture 傳入指標，這樣不同角色可以用不同圖片
    Character(int x, int y, LTexture* texture, int hp, int speed);

    // 虛擬解構子 (繼承時必須要有的，防止記憶體洩漏)
    virtual ~Character();

    // --- 核心功能 (所有角色都有) ---

    // 移動 (設為 virtual，因為主角用鍵盤，敵人用 AI)
    virtual void move();

    // 渲染 (設為 virtual，因為主角可能有轉向圖片，敵人可能只有一張)
    virtual void render();

    // --- 碰撞與狀態 ---

    // 取得碰撞箱
    SDL_Rect getCollider() const;

    // 判斷是否死亡
    bool isDead() const { return mHealth <= 0; }

    // 受傷扣血
    virtual void takeDamage(int damage);

    // 取得座標 (給子彈生成用)
    int getPosX() const { return mPosX; }
    int getPosY() const { return mPosY; }

protected:
    // 使用 protected，讓繼承的子類別 (Player, Enemy) 可以直接存取這些變數

    int mPosX, mPosY;       // 座標
    int mVelX, mVelY;       // 速度
    int mSpeed;             // 移動速度數值
    int mHealth;            // 生命值
    int mWidth, mHeight;    // 圖片寬高

    LTexture* mTexture;     // 該角色的貼圖指標
};

#endif
