#include "bullet.h"

Bullet::Bullet(int x, int y, int velX, int velY, LTexture* texture, bool isFromPlayer)
{
    mPosX = x;
    mPosY = y;
    mVelX = velX;
    mVelY = velY;
    mTexture = texture;

    // 儲存這個標記
    isPlayerBullet = isFromPlayer;

    available = true;
}

void Bullet::move()
{
    mPosX += mVelX;
    mPosY += mVelY;

    // 這裡我們在 move 裡面不直接刪除，只負責改變座標
    // 是否刪除交給外部迴圈判斷 isOffScreen
}

void Bullet::render()
{
    if(available && mTexture != NULL)
    {
        mTexture->render(mPosX, mPosY);
    }
}

bool Bullet::isOffScreen()
{
    // 如果子彈完全跑出 上、下、左、右 邊界
    if (mPosY < -BULLET_HEIGHT || mPosY > SCREEN_HEIGHT ||
        mPosX < -BULLET_WIDTH  || mPosX > SCREEN_WIDTH)
    {
        return true;
    }
    return false;
}

SDL_Rect Bullet::getCollider()
{
    SDL_Rect r;
    r.x = mPosX;
    r.y = mPosY;
    r.w = BULLET_WIDTH;
    r.h = BULLET_HEIGHT;
    return r;
}
