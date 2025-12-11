#include "enemy.h"
#include <cstdlib> // 用於隨機

Enemy::Enemy(int x, int y, LTexture* tex1, LTexture* tex2, LTexture* tex3, bool stopAtMiddle)
    : Character(x, y, tex1, 1, 2) // 血量 1，速度 2
{
    // 儲存三張圖
    mTex1 = tex1;
    mTex2 = tex2;
    mTex3 = tex3;
    frameCounter = 0;

    // 讓每個敵人的射擊頻率稍微不同，避免同時開槍太整齊
    // 設定在 60 ~ 120 幀之間 (假設 60FPS，約 1~2 秒射一次)
    fireInterval = 60 + rand() % 60;
    // 初始化閃爍狀態
    isHit = false;
    hitStartTime = 0;
    mStopAtMiddle = stopAtMiddle; // 記錄下來
}

void Enemy::move()
{
    // 如果設定要停在中間 (假設 Y=100 處)，且已經到了，就不再增加 Y
    if (mStopAtMiddle && mPosY >= 100) {
        // 停住了，不做事 (或者你可以讓它左右抖動)
    } else {
        mPosY += mSpeed; // 否則正常往下
    }
}

bool Enemy::isOffScreen()
{
    // 檢查是否跑出螢幕下方
    return mPosY > SCREEN_HEIGHT;
}

// 2. 實作受傷邏輯
void Enemy::takeDamage(int damage)
{
    // 呼叫父類別扣血
    Character::takeDamage(damage);

    // 觸發閃爍狀態
    isHit = true;
    hitStartTime = SDL_GetTicks();
}

// 3. 實作渲染邏輯 (動畫 + 閃爍)
void Enemy::render()
{
    // --- 處理受傷閃爍 ---
    if (isHit)
    {
        // 閃爍持續時間：100ms (0.1秒)
        if (SDL_GetTicks() - hitStartTime > 100)
        {
            isHit = false; // 時間到，停止閃爍

            // 確保圖片透明度恢復正常 (如果有的話)
            mTex1->setAlpha(255);
            mTex2->setAlpha(255);
            mTex3->setAlpha(255);
        }
        else
        {
            // 在閃爍期間，每 30ms 切換一次顯示/隱藏
            if ((SDL_GetTicks() / 30) % 2 == 0)
            {
                return; // 這一幀不畫圖，產生「隱形」效果，造成視覺上的閃爍
            }
        }
    }

    // --- 處理移動動畫 ---

    // 計算動畫幀數：每 150ms 切換一張圖
    // (SDL_GetTicks() / 速度) % 圖片總數
    int animFrame = (SDL_GetTicks() / 150) % 3;

    switch(animFrame)
    {
        case 0:
            if(mTex1) mTex1->render(mPosX, mPosY);
            break;
        case 1:
            if(mTex2) mTex2->render(mPosX, mPosY);
            break;
        case 2:
            if(mTex3) mTex3->render(mPosX, mPosY);
            break;
    }
}

std::vector<Bullet*> Enemy::fire(LTexture* bulletTexture)
{
    std::vector<Bullet*> bullets;

    // 每一幀計數器 +1
    frameCounter++;

    // 如果計數器達到設定的間隔
    if (frameCounter >= fireInterval)
    {
        // --- 發射邏輯 ---

        // 重置計數器
        frameCounter = 0;

        // 計算子彈生成位置 (敵人的正下方)
        int bX = mPosX + (mWidth / 2) - (Bullet::BULLET_WIDTH / 2);
        int bY = mPosY + mHeight;

        // 生成子彈：速度往下 (Y = 5)，X = 0
        bullets.push_back(new Bullet(bX, bY, 0, 5, bulletTexture, false));
    }

    return bullets;
}
