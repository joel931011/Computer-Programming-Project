#include "LTexture.h"
#include <iostream>

LTexture::LTexture()
{
    // 初始化
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture()
{
    // 物件銷毀時確保釋放資源
    free();
}

bool LTexture::loadFromFile( std::string path, SDL_Renderer* renderer )
{
    // 如果原本有紋理，先釋放
    free();

    // 最終的紋理
    SDL_Texture* newTexture = NULL;

    // --- 關鍵修改：自動加上 img/ 路徑 ---
    std::string fullPath = "img/" + path;
    // 這樣你在 main 裡寫 "player.png"，這裡會讀取 "img/player.png"

    // 載入圖片到 Surface
    SDL_Surface* loadedSurface = IMG_Load( fullPath.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", fullPath.c_str(), IMG_GetError() );
    }
    else
    {
        // 設定 Color Key (去背)
        // 這行通常用於 BMP 或沒有透明通道的圖，將「青色 (0,255,255)」設為透明
        // 如果你用的是 PNG 且本身有透明背景，這行其實非必要，但留著無妨
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

        // 從 Surface 建立 Texture
        newTexture = SDL_CreateTextureFromSurface( renderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", fullPath.c_str(), SDL_GetError() );
        }
        else
        {
            // 取得圖片尺寸
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        // 釋放暫存的 Surface
        SDL_FreeSurface( loadedSurface );
    }

    // 回傳成功與否
    mTexture = newTexture;
    return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor, TTF_Font* font, SDL_Renderer* renderer )
{
    // 先釋放舊的
    free();

    // 渲染文字 Surface
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended( font, textureText.c_str(), textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        // 從 Surface 建立 Texture
        mTexture = SDL_CreateTextureFromSurface( renderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            // 取得文字圖片尺寸
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }

        // 釋放 Surface
        SDL_FreeSurface( textSurface );
    }

    // 回傳成功與否
    return mTexture != NULL;
}

void LTexture::free()
{
    // 如果紋理存在，則釋放它
    if( mTexture != NULL )
    {
        SDL_DestroyTexture( mTexture );
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
    // 設定紋理的顏色模組
    SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
    // 設定混合模式
    SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
    // 設定透明度
    SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
    // 設定渲染的目標矩形 (預設為圖片原始大小)
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };

    // 如果有指定 clip (裁切)，則目標矩形的大小要跟裁切的大小一樣
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    // 執行渲染 (使用 global renderer，這裡我們假設在 class 內部已經有參考，或者更標準的做法是把 renderer 傳進來)
    // 為了配合原本的設計，這裡我們需要確保 renderer 是可用的。
    // *注意*：比較好的設計是 render 函式也接收 SDL_Renderer* renderer，或者在 load 時綁定 renderer。
    // 但為了配合你的 main.cpp 呼叫方式 (gPlayerFront.render(x,y))，我們需要 renderer 變數。

    // 修正：上面的 main.cpp 裡 loadFromFile 有傳 renderer，但 render 沒傳。
    // 由於 SDL_RenderCopyEx 需要 renderer，通常有兩種解法：
    // 1. LTexture 內部存一個 mRenderer 指標。
    // 2. render() 函式多傳一個 renderer 參數。

    // 為了讓你的 main.cpp 最好寫，我們採用「最常見的 lazyfoo 寫法」：
    // 但因為 render 函式裡沒有 renderer，這在標準寫法上會編譯不過。
    // 解決方案：請在 main.cpp 裡把 gRenderer 宣告成 extern，或者修改 render 函式。

    // **這裡我推薦：修改 render 函式，讓它不依賴全域變數，而是透過外部傳入 renderer (最穩)，
    // 或者在 LTexture 內部記住 renderer。**

    // 為了方便，我們假設你的 main.cpp 裡有 `extern SDL_Renderer* gRenderer;`
    // 或者我們修改 render 函式如下：

    extern SDL_Renderer* gRenderer; // 引用 main.cpp 裡的全域渲染器
    SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}
