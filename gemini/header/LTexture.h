#ifndef LTEXTURE_H
#define LTEXTURE_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <stdio.h>

class LTexture
{
    public:
        // 初始化變數
        LTexture();

        // 釋放記憶體
        ~LTexture();

        // 從檔案載入圖片
        // 注意：這裡會自動加上 "img/" 前綴
        bool loadFromFile( std::string path, SDL_Renderer* renderer );

        // 從文字字串建立圖片 (用於 UI 分數/血量)
        bool loadFromRenderedText( std::string textureText, SDL_Color textColor, TTF_Font* font, SDL_Renderer* renderer );

        // 釋放紋理資源
        void free();

        // 設定顏色調製 (Color Modulation) - 可選，用於改變圖片色調
        void setColor( Uint8 red, Uint8 green, Uint8 blue );

        // 設定混合模式 (Blending) - 用於透明度處理
        void setBlendMode( SDL_BlendMode blending );

        // 設定 Alpha 值 (透明度 0-255)
        void setAlpha( Uint8 alpha );

        // 渲染紋理到螢幕上
        // x, y: 螢幕座標
        // clip: 裁切範圍 (用於精靈圖 Sprite Sheet)
        // angle: 旋轉角度
        // center: 旋轉中心點
        // flip: 翻轉 (水平/垂直)
        void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

        // 取得圖片寬度
        int getWidth();

        // 取得圖片高度
        int getHeight();

    private:
        // 實際的硬體紋理
        SDL_Texture* mTexture;

        // 圖片的寬高
        int mWidth;
        int mHeight;
};

#endif
