#include <SDL2/SDL.h>
#include <iostream>
#include <cmath>
#include <algorithm> // for std::max

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_BULLETS 1000000
#define PI 3.14159265

// ------------------------------------------------
// 基礎向量類別
// ------------------------------------------------
class Vec {
public:
    float vx, vy;
    Vec(float vx = 0, float vy = 0) : vx(vx), vy(vy) {}
    void setSpeed(float sx, float sy) {
        vx = sx;
        vy = sy;
    }
};

// ------------------------------------------------
// 擴展子彈類別 (新增 accel 和 curve)
// ------------------------------------------------
// ------------------------------------------------
// 擴展子彈類別 (新增 accel, curve, 和 wave 參數)
// ------------------------------------------------
// ------------------------------------------------
// 擴展子彈類別 (新增 accel, curve, 和 wave 參數)
// ------------------------------------------------
class AdvancedBullet : public Vec {
public:
    float x, y;
    bool active;
    float direction;
    float speed;
    float accel;
    float curve;

    // 新增波浪參數 <---【重要】確保這三個變數存在
    float wave_amplitude; // 波浪擺動的幅度 (弧度)
    float wave_frequency; // 波浪擺動的頻率
    float time_elapsed;   // 追蹤子彈生命週期

    AdvancedBullet() : Vec(), direction(0), speed(0), accel(0), curve(0),
                       wave_amplitude(0), wave_frequency(0), time_elapsed(0) { // 確保建構子也初始化
        active = false;
    }

    // fire 函式新增兩個波浪參數
    void fire(float startX, float startY, float v, float theta, float a, float c, float amp = 0.0f, float freq = 0.0f) {
        x = startX;
        y = startY;
        direction = theta;
        speed = v;
        accel = a;
        curve = c;

        // 設定波浪參數
        wave_amplitude = amp;
        wave_frequency = freq;
        time_elapsed = 0;

        active = true;
        setSpeed(speed * cos(direction), speed * sin(direction));
    }

    void update(float dt) {
        if (!active) return;

        time_elapsed += dt; // 更新生命週期時間

        // 1. 曲線 (Curve) + 波浪 (Wave)
        float base_curve_change = curve * dt;

        float wave_curve_change = 0.0f;
        if (wave_amplitude > 0.0f && wave_frequency > 0.0f) {
            // 使用 sin 函式產生週期性轉向率
            wave_curve_change = sin(time_elapsed * wave_frequency) * wave_amplitude * dt;
        }

        direction += base_curve_change + wave_curve_change;

        // 2. 加速度 (Accel)
        speed += accel * dt;

        // 3. 重新計算速度向量和位置更新
        vx = speed * cos(direction);
        vy = speed * sin(direction);

        x += vx * dt;
        y += vy * dt;

        // 邊界檢查...
        if (x < -10 || x > SCREEN_WIDTH + 10 || y < -10 || y > SCREEN_HEIGHT + 10 || speed < 0)
            active = false;
    }
};// ------------------------------------------------
// 全域子彈管理實體
// ------------------------------------------------
AdvancedBullet advanced_bullets[MAX_BULLETS];

void shootAdvancedBullet(float x, float y, float v, float theta = 0.0f, float accel = 0.0f, float curve = 0.0f, float wave_amp = 0.0f, float wave_freq = 0.0f) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!advanced_bullets[i].active) {
            // 傳入新增的 wave_amp 和 wave_freq
            advanced_bullets[i].fire(x, y, v, theta, accel, curve, wave_amp, wave_freq);
            break;
        }
    }
}

void updateAdvancedBullets(float dt) {
    for (int i = 0; i < MAX_BULLETS; i++)
        advanced_bullets[i].update(dt);
}

void renderAdvancedBullets(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // 黃色子彈
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (advanced_bullets[i].active) {
            SDL_Rect rect = { (int)advanced_bullets[i].x, (int)advanced_bullets[i].y, 1, 1 };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}
struct BulletPreset {
    float bullet_speed = 200.0f;
    float bullet_accel = 0.0f;

    // ---- 波浪彈參數 ----
    float bullet_waveFreq = 0.0f; // 預設 0 = 不晃動
    float bullet_waveAmp  = 0.0f; // 預設 0 = 不晃動
};

// ------------------------------------------------
// GMS 彈幕控制類別
// ------------------------------------------------
class GMSBulletPattern {
public:
    // --- 參數宣告 ---
    int total_arrays = 1;
    int bullets = 10;
    float array_spread_total = PI * 2;
    float spread = PI / 4.0f;
    float base_angle = 0.0f;
    float start_angle = 0.0f;
    float bullet_speed = 150.0f;
    float bullet_accel = 0.0f;
    float bullet_curve = 0.0f;
    float x_offset = 0.0f;
    float y_offset = 0.0f;
    float object_width = 0.0f;
    float object_height = 0.0f;
    int fire_rate = 10;
    float spin = 0.0f;
    float spin_mod = 0.0f;
    float bullet_wave_amp = 0.0f;
    float bullet_wave_freq = 0.0f;
    float max_spin_spd = PI/2.0f;
    bool spin_reverse = false;

private:
    float shoot_timer = 0.0f;

public:
    // **初始化函式 (init)**: 對應 GMS 的 bhpg_pattern_init
    void init(
        int num_bullets, float bullet_spread, float initial_angle,
        int num_arrays, float array_spread_total_arg,
        float begin_spin, float spin_change_rate, bool reverse_spin, float max_spin,
        int rate_of_fire,
        float obj_width, float obj_height, float off_x, float off_y
    )
    {
        base_angle = 0.0f;
        shoot_timer = 0.0f;
        bullets = num_bullets;
        spread = bullet_spread;
        start_angle = initial_angle;
        total_arrays = num_arrays;
        array_spread_total = array_spread_total_arg;
        spin = begin_spin;
        spin_mod = spin_change_rate;
        spin_reverse = reverse_spin;
        max_spin_spd = max_spin;
        fire_rate = rate_of_fire;
        object_width = obj_width;
        object_height = obj_height;
        x_offset = off_x;
        y_offset = off_y;
    }

    // **更新與發射函式 (updateAndFire)**: 對應 GMS 的 bhpg_pattern_step
    void updateAndFire(float dt, float x, float y) {
        shoot_timer += dt;
        const float FPS = 60.0f;
        float fire_interval = fire_rate / FPS;

        if (shoot_timer >= fire_interval) {
            shoot_timer = 0.0f;

            // 角度步進計算 (控制擴散角度)
            int bb = std::max(1, bullets - 1);
            int aa = std::max(1, total_arrays - 1);

            float a_ang = (array_spread_total / aa); // 陣列間角度步進
            float b_ang = (spread / bb);             // 陣列內子彈角度步進

            for (int j = 0; j < total_arrays; j++) {
                for (int i = 0; i < bullets; i++) {
                    float current_angle = base_angle + (b_ang * i) + (a_ang * j) + start_angle;

                    // lengthdir 邏輯: 決定發射點
                    float xx = x + x_offset + object_width * cos(current_angle);
                    float yy = y + y_offset + object_height * sin(current_angle);

                    ::shootAdvancedBullet(
                        xx, yy,
                        bullet_speed,
                            current_angle,
                            bullet_accel,
                            bullet_curve,
                            bullet_wave_amp,  // <-- 傳遞波浪幅度
                            bullet_wave_freq   // 傳遞曲線/轉向率
                    );
                }
            }

            // 旋轉更新邏輯
            base_angle += spin;
            spin += spin_mod;

            if (spin_reverse) {
                if (spin < -max_spin_spd || spin > max_spin_spd) {
                    spin_mod = -spin_mod;
                }
            }
        }
    }
};

// ------------------------------------------------
// 全域實例化 (主彈幕控制器)
// ------------------------------------------------
GMSBulletPattern StarSpiral;

// ------------------------------------------------
// 彈幕設定函式 (對應您圖片中的螺旋效果)
// ------------------------------------------------
// ------------------------------------------------
// 彈幕設定函式 (加入波浪效果)
// ------------------------------------------------
void setupStarSpiralPattern() {
    // ... (init 函式呼叫保持不變) ...
    StarSpiral.init(
        1,         // num_bullets
        0.0f,      // bullet_spread
        0.0f,      // initial_angle
        12,        // num_arrays
        2 * PI,    // array_spread_total_arg
        0.01f,     // begin_spin (螺旋速度)
        0.01f,     // spin_change_rate
        true,     // reverse_spin
        0.1f,      // max_spin
        5,         // rate_of_fire
        0.0f,      // obj_width
        0.0f,      // obj_height
        0.0f,      // off_x
        0.0f       // off_y
    );

    // 額外設定子彈速度和飛行特性
    StarSpiral.bullet_speed = 150.0f;
    StarSpiral.bullet_curve = 0.0f; // 保持基礎轉向率為 0
    StarSpiral.bullet_accel = 0.0f;

    // **設定波浪效果**
    StarSpiral.bullet_wave_amp = 0.5f;   // <-- 波浪擺動的幅度 (越大擺動越劇烈)
    StarSpiral.bullet_wave_freq = 15.0f; // <-- 波浪擺動的頻率 (越大波長越短，擺動越快)
}
// ------------------------------------------------
// 主函式
// ------------------------------------------------
int main(int argc, char* argv[]) {
    // 1. 彈幕初始化 (在迴圈開始前設定參數)
    setupStarSpiralPattern();

    // SDL 初始化代碼
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(
        "SDL Bullet Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    float playerX = 400, playerY = 300; // 將玩家初始位置移到中心，更容易觀察彈幕

    while (running) {
        Uint32 current = SDL_GetTicks();
        float dt = (current - lastTime) / 1000.0f;
        lastTime = current;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_SPACE:
                        // 範例：切換速度
                        StarSpiral.bullet_speed = (StarSpiral.bullet_speed == 150.0f) ? 300.0f : 150.0f;
                        break;
                    case SDLK_RETURN:
                        // 範例：臨時加速旋轉
                        StarSpiral.spin = 0.1f;
                        break;
                }
            }
        }

        // ------------------------------------
        // 核心更新邏輯
        // ------------------------------------
        // 1. 執行 GMS 彈幕邏輯 (在 playerX, playerY 處發射)
        StarSpiral.updateAndFire(dt, playerX, playerY);

        // 2. 更新所有 AdvancedBullet 的位置和方向
        updateAdvancedBullets(dt);

        // ------------------------------------
        // 渲染
        // ------------------------------------
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // 渲染玩家 (白色方塊)
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_Rect player = { (int)playerX, (int)playerY, 20, 20 };
        SDL_RenderFillRect(renderer, &player);

        // 渲染 AdvancedBullet (黃色子彈)
        renderAdvancedBullets(renderer);

        SDL_RenderPresent(renderer);
    }

    // SDL 清理代碼
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
