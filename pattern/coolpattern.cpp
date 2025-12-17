#include <SDL2/SDL.h>
#include <iostream>
#include <cmath> 
#include <algorithm> // for std::max

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_BULLETS 1000000
#define PI 3.14159265
#define FPS 60.0f // 定義標準幀率

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
// 擴展子彈類別 (包含 accel 和 curve)
// ------------------------------------------------
class AdvancedBullet : public Vec {
public:
    float x, y;
    bool active;
    float direction;
    float speed;
    float accel;
    float curve;

    AdvancedBullet() : Vec(), direction(0), speed(0), accel(0), curve(0) {
        active = false;
    }

    void fire(float startX, float startY, float v, float theta, float a, float c) {
        x = startX;
        y = startY;
        direction = theta;
        speed = v;
        accel = a;
        curve = c;
        active = true;
        setSpeed(speed * cos(direction), speed * sin(direction));
    }

    void update(float dt) {
        if (!active) return;

        // 1. 曲線 (Curve): 改變子彈的方向
        direction += curve * dt;

        // 2. 加速度 (Accel): 改變子彈的速度大小
        speed += accel * dt;

        // 3. 重新計算速度向量
        vx = speed * cos(direction);
        vy = speed * sin(direction);

        x += vx * dt;
        y += vy * dt;

        // 邊界檢查
        if (x < 0 || x > SCREEN_WIDTH || y < 0 || y > SCREEN_HEIGHT || speed < 0)
            active = false;
    }
};

// ------------------------------------------------
// 全域子彈管理實體和輔助函式
// ------------------------------------------------
AdvancedBullet advanced_bullets[MAX_BULLETS];

void shootAdvancedBullet(float x, float y, float v, float theta = 0.0f, float accel = 0.0f, float curve = 0.0f) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!advanced_bullets[i].active) {
            advanced_bullets[i].fire(x, y, v, theta, accel, curve);
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
                        bullet_curve
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
// 模式控制函式 (實作 3 個轉折點邏輯)
// ------------------------------------------------
void control_pattern_phases(float total_time) {
    // === 階段 1: 緩慢螺旋 (0s ~ 5s) ===
    if (total_time < 5.0f) {
        // 初始溫和設定 (類似您圖片中的螺旋)
        StarSpiral.bullets = 1;
        StarSpiral.total_arrays = 10;
        StarSpiral.fire_rate = 8;
        StarSpiral.bullet_speed = 100.0f;
        StarSpiral.spin = 0.03f;
        StarSpiral.spin_mod = 0.0f;
        StarSpiral.bullet_accel = 0.0f;
        StarSpiral.bullet_curve = 0.0f;
        StarSpiral.array_spread_total = 2 * PI;
        StarSpiral.spread = 0.0f;
    }

    // === 階段 2: 爆炸性加速 (轉折 1: 5s ~ 10s) ===
    else if (total_time >= 5.0f && total_time < 10.0f) {
        // 1. 增加線條數量
        StarSpiral.total_arrays = 15;

        // 2. 啟動旋轉和子彈加速
        StarSpiral.spin_mod = 0.005f;       // 旋轉速度持續增加
        StarSpiral.bullet_accel = 80.0f;    // 子彈越飛越快
        StarSpiral.bullet_speed = 100.0f;   // 重設基礎速度

        StarSpiral.bullet_curve = 0.0f;
    }

    // === 階段 3: 收斂漩渦 (轉折 2: 10s ~ 15s) ===
    else if (total_time >= 10.0f && total_time < 15.0f) {
        // 1. 啟動強烈曲線，將子彈拉回中心 (負值收斂)
        StarSpiral.bullet_curve = -PI * 0.5f; // 強烈的順時針轉向，形成漩渦

        // 2. 旋轉開始反向減速 (製造混亂感)
        StarSpiral.spin_mod = -0.01f;
        StarSpiral.bullet_accel = -50.0f;   // 子彈減速

        StarSpiral.total_arrays = 20;
    }

    // === 階段 4: 穩定散射 (轉折 3: 15s 以後) ===
    else {
        // 1. 關閉所有動態效果
        StarSpiral.spin = 0.0f;
        StarSpiral.spin_mod = 0.0f;
        StarSpiral.bullet_curve = 0.0f;
        StarSpiral.bullet_accel = 0.0f;

        // 2. 穩定的大範圍散射 (Star pattern)
        StarSpiral.total_arrays = 1; // 單組陣列
        StarSpiral.bullets = 8;      // 8 顆子彈
        StarSpiral.spread = 2 * PI;  // 散射 360 度
        StarSpiral.fire_rate = 15;   // 慢下來
        StarSpiral.bullet_speed = 120.0f;
    }
}

// ------------------------------------------------
// 主函式
// ------------------------------------------------
int main(int argc, char* argv[]) {
    // 這裡我們不再需要 setupStarSpiralPattern()，因為 control_pattern_phases 會在 0s 時設置初始值。

    // SDL 初始化代碼
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow(
        "SDL Bullet Demo (Dynamic Phases)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT, 0
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool running = true;
    SDL_Event event;
    Uint32 lastTime = SDL_GetTicks();
    float playerX = 400, playerY = 300; // 將玩家初始位置移到中心
    float total_time = 0.0f; // <-- 時間追蹤變數

    while (running) {
        Uint32 current = SDL_GetTicks();
        float dt = (current - lastTime) / 1000.0f;
        lastTime = current;
        total_time += dt; // <-- 更新總運行時間

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;

            if (event.type == SDL_KEYDOWN) {
                switch(event.key.keysym.sym) {
                    case SDLK_SPACE:
                        // 範例：重設時間，重新開始模式
                        total_time = 0.0f;
                        break;
                    case SDLK_RETURN:
                        // 範例：臨時加速旋轉
                        StarSpiral.spin += 0.1f;
                        break;
                }
            }
        }

        // ------------------------------------
        // 核心更新邏輯
        // ------------------------------------
        // 1. 控制模式轉折！
        control_pattern_phases(total_time);

        // 2. 執行 GMS 彈幕邏輯
        StarSpiral.updateAndFire(dt, playerX, playerY);

        // 3. 更新子彈位置
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
