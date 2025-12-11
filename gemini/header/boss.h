#ifndef BOSS_H
#define BOSS_H

#include "character.h"
#include "bullet.h"
#include <vector>

class Boss : public Character
{
public:
    // bossLevel: 1 代表第一關 Boss，2 代表第二關 Boss
    Boss(int x, int y, LTexture* texture, int bossLevel);

    // Boss 的獨特移動邏輯 (進場 -> 左右徘徊)
    void move() override;

    // Boss 的攻擊邏輯 (散射彈)
    std::vector<Bullet*> fire(LTexture* bulletTexture);

    // 取得 Boss 等級 (決定掉落物或分數)
    int getLevel() const { return mBossLevel; }

private:
    int mBossLevel;      // 1 or 2
    int frameCounter;    // 攻擊計時器

    // Boss 的狀態
    enum BossState { ENTERING, FIGHTING };
    BossState state;

    bool movingRight;    // 控制左右移動方向
};

#endif
