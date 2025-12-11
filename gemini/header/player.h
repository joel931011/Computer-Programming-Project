#ifndef PLAYER_H
#define PLAYER_H

#include "character.h"
#include "bullet.h"
#include <vector>
#include <chrono> // 用於計時散射時間

// 假設螢幕寬高定義在 main 或全域
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;

class Player : public Character
{
public:
    // 建構子：需要傳入三張不同的紋理指標 (正面, 左傾, 右傾)
    Player(LTexture* texFront, LTexture* texLeft, LTexture* texRight);

    // 處理鍵盤輸入
    void handleEvent(SDL_Event& e);

    // 覆寫移動邏輯：增加邊界檢查，防止主角跑出視窗
    void move() override;

    // 覆寫渲染邏輯：根據移動方向切換圖片 + 無敵時間閃爍效果
    void render() override;

    // 覆寫受傷邏輯：增加無敵時間判斷
    void takeDamage(int damage) override;

    // --- 攻擊相關 ---
    // 發射子彈 (回傳子彈 vector)
    std::vector<Bullet*> fire(LTexture* bulletTexture);

    // --- 道具效果相關 ---
    void activateScatter();     // 啟動散射模式
    void updateScatterStatus(); // 檢查道具時間是否結束

    // --- 重置與狀態 ---
    void reset();               // 重置主角狀態 (滿血復活回到原點)
    int getHealth() const { return mHealth; }
    void addHealth(int n) {mHealth+=n;}
    int getScatterTimeLeft() const;

private:
    // 儲存不同姿態的紋理指標
    LTexture* mTextureFront;
    LTexture* mTextureLeft;
    LTexture* mTextureRight;

    // 散射模式狀態
    bool isScatterMode;
    std::chrono::time_point<std::chrono::steady_clock> scatterStartTime;
    const double scatterDuration = 5.0; // 效果持續 5 秒

    // 無敵時間狀態
    bool isInvincible;
    Uint32 lastHitTime;     // 上次被打到的時間 (SDL_GetTicks)
    const Uint32 invincibilityDuration = 1500; // 無敵時間 1.5 秒 (1500ms)
};

#endif
