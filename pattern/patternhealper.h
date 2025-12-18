#include <vector>
#include <cmath>
#include "bullet.h"

// �w�q��P�v
#define PI 3.14159265

namespace PatternHelper
{
    // 1. ���a���u (Shotgun)
    inline std::vector<Bullet*> createPlayerShotgun(int x, int y, LTexture* tex)
    {
        std::vector<Bullet*> bullets;
        float speed = 10.0f;
        float baseAngle = -PI / 2; // -90�� (�¤W)
        bullets.push_back(new Bullet(x, y, speed, baseAngle, tex, true));
        bullets.push_back(new Bullet(x, y, speed, baseAngle - 0.2f, tex, true));
        bullets.push_back(new Bullet(x, y, speed, baseAngle + 0.2f, tex, true));

        return bullets;
    }

    inline std::vector<Bullet*> createPlayerPatternS(int x, int y, LTexture* tex)
    {
        std::vector<Bullet*> bullets;
        float baseSpeed = 8.0f;
        for (float f = 0; f < 5; f += 1.0f)
        {
            float speed = baseSpeed - (f * 1.0f);
            // ���שT�w�¤W (-PI/2)
            bullets.push_back(new Bullet(x, y, speed, -PI/2, tex, true));
        }
        return bullets;
    }

    // [�s�W] Pattern D: �K����z�o (��g�� shootBulletbossb)
    // �S��G�V�|�P�g�X 8 ���l�u
    inline std::vector<Bullet*> createPlayerPatternD(int x, int y, LTexture* tex)
    {
        std::vector<Bullet*> bullets;
        float speed = 7.0f;

        // �@�� 360 �� (2*PI)�A���� 8 ���� -> �C�� PI/4
        for (float angle = 0; angle < 2 * PI; angle += PI / 4)
        {
            bullets.push_back(new Bullet(x, y, speed, angle, tex, true));
        }
        return bullets;
    }

    // 2. Boss ����X���u (��g�ۦP�Ǫ� shootBulletbossb)
    inline std::vector<Bullet*> createBossCircle(int x, int y, LTexture* tex)
    {
        std::vector<Bullet*> bullets;
        float speed = 5.0f; // Boss �l�u�C�@�I

        // �C 18 �� (PI/10) �o�g�@���A¶�@��
        for (float angle = 0; angle < 2 * PI; angle += PI / 10)
        {
            bullets.push_back(new Bullet(x, y, speed, angle, tex, false)); // false = �ĤH
        }
        return bullets;
    }

    // 3. Boss 旋轉螺旋 (升級版：支援加速度與曲線)
    // offsetAngle: 由外部計時器傳入，控制每一波發射的基礎旋轉
// 3. Boss 螺旋彈幕 (完全適配 GMS 參數邏輯版)
    inline std::vector<Bullet*> createBossSpiral(int x, int y, float offsetAngle, LTexture* tex)
    {
        std::vector<Bullet*> bullets;

        // --- 這些就是你要求的參數設定 ---
        int total_arrays = 3;              // 螺旋的手數 (4條)
        int bullets_per_array = 3;         // 每一手每次射幾顆 (通常螺旋是1)
        float array_spread_total = 2 * PI; // 陣列總分佈範圍 (360度)
        float spread = PI / 4.0f;          // 單一陣列內的扇形範圍
        float start_angle = 0.0f;          // 初始額外角度
        float bullet_speed = 2.0f;         // 子彈速度
        float bullet_accel = 1.5f;        // 子彈加速度
        float bullet_curve = 0.0f;        // 子彈曲線路徑 (讓螺旋更捲)

        // 發射點圓周偏移 (讓子彈從 Boss 身體邊緣產生的效果)
        float object_width = 20.0f;
        float object_height = 20.0f;
        float x_offset = 0.0f;
        float y_offset = 0.0f;

        // --- 核心邏輯計算 ---

        // 避免除以零的步進計算
        int aa = std::max(1, total_arrays - 1);
        int bb = std::max(1, bullets_per_array - 1);

        float a_ang = (array_spread_total / (float)total_arrays); // 陣列間的角度間隔
        float b_ang = (bullets_per_array > 1) ? (spread / (float)bb) : 0; // 陣列內子彈間隔

        for (int j = 0; j < total_arrays; j++)
        {
            for (int i = 0; i < bullets_per_array; i++)
            {
                // 計算最終發射角度：基礎旋轉(offsetAngle) + 陣列偏移 + 陣列內偏移 + 初始修正
                float current_angle = offsetAngle + (a_ang * j) + (b_ang * i) + start_angle;

                // 計算發射點 (Lengthdir 邏輯)：考慮物體寬度與偏移量
                float xx = x + x_offset + object_width * cos(current_angle);
                float yy = y + y_offset + object_height * sin(current_angle);

                // 產生子彈 (使用 10 參數完整建構子)
                // 參數：x, y, speed, angle, texture, isPlayer, accel, curve, waveAmp, waveFreq
                bullets.push_back(new Bullet(
                    (int)xx, (int)yy,
                    bullet_speed,
                    current_angle,
                    tex,
                    false,          // 是否為玩家子彈
                    bullet_accel,
                    bullet_curve,
                    0.0f, 0.0f      // wave 參數暫不使用
                ));
            }
        }
        return bullets;
    }    // [�s�W] Boss Pattern C (��g�� shootBulletbossc)
    // �O�o�[ inline �קK���Ʃw�q���~
    inline std::vector<Bullet*> createBossPatternC(int x, int y, LTexture* tex)
    {
        std::vector<Bullet*> bullets;
        float d = 0; // �Ψӱ���t���ܤƪ��ܼ�
        for (float aa = -PI / 2; aa <= PI / 2.0; aa += PI / 40)
        {
            float speed = (120.0f - d) / 20.0f;

            bullets.push_back(new Bullet(x, y, speed, aa+PI/2, tex, false));
            d += 1;
        }

        for (float aa = PI / 2; aa >= -3 * PI / 2.0; aa -= PI / 40)
        {
            float speed = (80.0f - d) / 20.0f;

            bullets.push_back(new Bullet(x, y, speed, aa+PI/2, tex, false));
            d += 1;
        }

        return bullets;
    }
    //  GMS Star Spiral Pattern (12�V�i������)
    inline std::vector<Bullet*> createGMSStarSpiral(int x, int y, float baseAngle, LTexture* tex)
    {
        std::vector<Bullet*> bullets;

        // �ѼƳ]�w (�Ѧ� setupStarSpiralPattern)
        int totalArrays = 12;       // 12 �����u
        float speed = 3.0f;         // �t�� (�y�L�վ�A�t�A���ѪR��)
        float waveAmp = 2.0f;       // �i���T�� (�P�ǳ] 0.5 �O���סA�o�̥i��n�դj�@�I���ĪG����)
        float waveFreq = 3.0f;      // �i���W�v

        // �j�鲣�� 12 �Ӥ�V���l�u
        for (int i = 0; i < totalArrays; i++)
        {
            // �p��C�Ӱ}�C�����סG��¦���� + (��i�� * 360�� / �`��)
            float angle = baseAngle + (i * (2 * PI / totalArrays));

            // �إߤl�u�A�ǤJ�i���Ѽ�
            // �غc�l�Ѽƶ���: x, y, speed, angle, texture, isPlayer, accel, curve, waveAmp, waveFreq
            bullets.push_back(new Bullet(x, y, speed, angle, tex, false, 0.0f, 0.0f, waveAmp, waveFreq));
        }

        return bullets;
    }

    inline std::vector<Bullet*> createDynamicPhasePattern(int x, int y, float timer, float& baseAngle, float& spinSpeed, LTexture* tex)
    {
        std::vector<Bullet*> bullets;

        // --- 1. �w�q�w�]�Ѽ� (�����P�ǵ{���X���ܼ�) ---
        int totalArrays = 10;
        float speed = 5.0f;
        float accel = 0.0f;
        float curve = 0.0f;
        float spinMod = 0.0f; // ����[�t��
        // --- 2. �P�_���q (Phase Logic) ---
        // Phase 1: �w�C���� (0s ~ 5s)
        if (timer < 5.0f) {
            totalArrays = 10;
            speed = 5.0f;
            spinSpeed = 0.03f; // �T�w��t
            accel = 0.0f;
            curve = 0.0f;
        }
        // Phase 2: �z���ʥ[�t (5s ~ 10s)
        else if (timer >= 5.0f && timer < 10.0f) {
            totalArrays = 15;
            speed = 5.0f;
            spinMod = 0.005f; // �}�l�V��V��
            accel = 8.0f;    // �l�u�V���V��
            curve = 0.0f;
        }
        // Phase 3: ���ĺx�� (10s ~ 15s)
        else if (timer >= 10.0f && timer < 15.0f) {
            totalArrays = 20;
            // �o�̤����] speed�A���򤧫e��
            spinMod = -0.01f; // ����}�l�ϦV��t
            accel = -5.0f;   // �l�u��t
            curve = -3.14f * 0.5f; // �j�P����V (�x���ĪG)
        }
        // Phase 4: í�w���g (15s ~ END)
        else {
            totalArrays = 8;
            speed = 6.0f;
            spinSpeed = 0.0f; // �������
            spinMod = 0.0f;
            accel = 0.0f;
            curve = 0.0f;


        }
        // --- 3. ��s���બ�A ---
        // �p�G������[�t�� (spinMod)�A��s��t
        if (timer >= 5.0f && timer < 15.0f) {
            spinSpeed += spinMod;
        }
        baseAngle += spinSpeed;

        // --- 4. ���ͤl�u ---
        for (int i = 0; i < totalArrays; i++)
        {
            float angle = baseAngle + (i * (2 * 3.14159f / totalArrays));

            // x, y, speed, angle, texture, isPlayer, accel, curve, waveAmp, waveFreq
            bullets.push_back(new Bullet(x, y, speed, angle, tex, false, accel, curve, 0.0f, 0.0f));
        }

        return bullets;
    }
}
