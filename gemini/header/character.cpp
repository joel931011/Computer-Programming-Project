#include "character.h"

Character::Character(int x, int y, LTexture* texture, int hp, int speed)
{
    mPosX = x;
    mPosY = y;
    mTexture = texture;
    mHealth = hp;
    mSpeed = speed;

    mVelX = 0;
    mVelY = 0;

    // 從紋理自動抓取寬高，若紋理為 NULL 則設為 0
    if (mTexture != NULL) {
        mWidth = mTexture->getWidth();
        mHeight = mTexture->getHeight();
    } else {
        mWidth = 0;
        mHeight = 0;
    }
}

Character::~Character()
{
    // 這裡通常不需要 delete mTexture，因為紋理通常由外部資源管理器統一管理
    // 除非你是每個角色 new 出一張新圖
}

void Character::move()
{
    // 最基本的移動：加上速度
    mPosX += mVelX;
    mPosY += mVelY;

    // 注意：基底類別不檢查邊界，因為敵人可能會飛出螢幕
    // 邊界檢查交給 Player 類別去覆寫 (Override)
}

void Character::render()
{
    if (mTexture != NULL) {
        mTexture->render(mPosX, mPosY);
    }
}

SDL_Rect Character::getCollider() const
{
    SDL_Rect r;
    r.x = mPosX;
    r.y = mPosY;
    r.w = mWidth;
    r.h = mHeight;
    return r;
}


void Character::takeDamage(int damage)
{
    mHealth -= damage;
    if (mHealth < 0) mHealth = 0;
}
