#include "player.h"
#include <iostream>

// 建構子
Player::Player(LTexture* texFront, LTexture* texLeft, LTexture* texRight)
    : Character(380, 480, texFront, 5, 5) // 初始位置(380,480), 血量3, 速度5
{
    // 初始化紋理指標
    mTextureFront = texFront;
    mTextureLeft = texLeft;
    mTextureRight = texRight;

    // 修正碰撞箱大小 (如果有需要微調，可不設，預設會抓圖片大小)
    mWidth = mTextureFront->getWidth();
    mHeight = mTextureFront->getHeight();

    // 初始化狀態
    isScatterMode = false;
    isInvincible = false;
    lastHitTime = 0;
}

void Player::handleEvent(SDL_Event& e)
{
    // 按下按鍵 (KeyDown)
    if( e.type == SDL_KEYDOWN && e.key.repeat == 0 )
    {
        switch( e.key.keysym.sym )
        {
            case SDLK_UP:    mVelY -= mSpeed; break;
            case SDLK_DOWN:  mVelY += mSpeed; break;
            case SDLK_LEFT:  mVelX -= mSpeed; break;
            case SDLK_RIGHT: mVelX += mSpeed; break;
        }
    }
    // 放開按鍵 (KeyUp)
    else if( e.type == SDL_KEYUP && e.key.repeat == 0 )
    {
        switch( e.key.keysym.sym )
        {
            case SDLK_UP:    mVelY += mSpeed; break;
            case SDLK_DOWN:  mVelY -= mSpeed; break;
            case SDLK_LEFT:  mVelX += mSpeed; break;
            case SDLK_RIGHT: mVelX -= mSpeed; break;
        }
    }
}

void Player::move()
{
    // 1. 水平移動
    mPosX += mVelX;

    // 檢查左右邊界 (撞牆就退回)
    if( mPosX < 0 || ( mPosX + mWidth > SCREEN_WIDTH ) )
    {
        mPosX -= mVelX;
    }

    // 2. 垂直移動
    mPosY += mVelY;

    // 檢查上下邊界 (撞牆就退回)
    if( mPosY < 0 || ( mPosY + mHeight > SCREEN_HEIGHT ) )
    {
        mPosY -= mVelY;
    }
}

void Player::render()
{
    // --- 處理無敵閃爍效果 ---
    if (isInvincible)
    {
        // 檢查無敵時間是否結束
        if (SDL_GetTicks() - lastHitTime > invincibilityDuration) {
            isInvincible = false;
            // 回復正常透明度
            mTextureFront->setAlpha(255);
            mTextureLeft->setAlpha(255);
            mTextureRight->setAlpha(255);
        }
        else {
            // 閃爍邏輯：每 100ms 切換一次顯示
            if ((SDL_GetTicks() / 100) % 2 == 0) {
                // 這一幀不畫，產生隱形效果
                return;
            }
        }
    }

    // --- 根據移動方向畫出對應圖片 ---
    if (mVelX < 0) // 向左
    {
        if (mTextureLeft != NULL)
            mTextureLeft->render(mPosX, mPosY);
    }
    else if (mVelX > 0) // 向右
    {
        if (mTextureRight != NULL)
            mTextureRight->render(mPosX, mPosY);
    }
    else // 正面
    {
        if (mTextureFront != NULL)
            mTextureFront->render(mPosX, mPosY);
    }
}

void Player::takeDamage(int damage)
{
    // 如果現在是無敵狀態，就不扣血
    if (isInvincible) return;

    // 扣除血量 (呼叫父類別的邏輯，或者直接扣 mHealth)
    mHealth -= damage;
    if (mHealth < 0) mHealth = 0;

    // 設定無敵狀態
    isInvincible = true;
    lastHitTime = SDL_GetTicks();

    std::cout << "Player hit! Remaining HP: " << mHealth << std::endl;
}

std::vector<Bullet*> Player::fire(LTexture* bulletTexture)
{
    std::vector<Bullet*> newBullets;

    // 計算發射起始點 (主角中心上方)
    int startX = mPosX + (mWidth / 2) - (Bullet::BULLET_WIDTH / 2);
    int startY = mPosY - Bullet::BULLET_HEIGHT;
    int bulletSpeed = 10;

    // 檢查是否處於散射模式
    if (isScatterMode)
    {
        // 產生 3 顆子彈 (左斜、中、右斜)
        // 建構子最後一個參數 true 代表是主角發射的
        newBullets.push_back(new Bullet(startX, startY, 0, -bulletSpeed, bulletTexture, true));
        newBullets.push_back(new Bullet(startX, startY, -3, -bulletSpeed, bulletTexture, true));
        newBullets.push_back(new Bullet(startX, startY, 3, -bulletSpeed, bulletTexture, true));
    }
    else
    {
        // 普通模式 (1 顆)
        newBullets.push_back(new Bullet(startX, startY, 0, -bulletSpeed, bulletTexture, true));
    }

    return newBullets;
}

void Player::activateScatter()
{
    isScatterMode = true;
    scatterStartTime = std::chrono::steady_clock::now();
    std::cout << "Scatter Mode ON!" << std::endl;
}

void Player::updateScatterStatus()
{
    if (isScatterMode)
    {
        auto now = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = now - scatterStartTime;

        if (elapsed.count() > scatterDuration)
        {
            isScatterMode = false;
            std::cout << "Scatter Mode OFF!" << std::endl;
        }
    }
}

void Player::reset()
{
    // 重置位置
    mPosX = 380;
    mPosY = 480;

    // 重置速度
    mVelX = 0;
    mVelY = 0;

    // 重置狀態
    mHealth = 5;
    isScatterMode = false;
    isInvincible = false;

    // 確保透明度回復
    if(mTextureFront) mTextureFront->setAlpha(255);
    if(mTextureLeft) mTextureLeft->setAlpha(255);
    if(mTextureRight) mTextureRight->setAlpha(255);
}

int Player::getScatterTimeLeft() const {
    return getScatterTimeLeft();
}
