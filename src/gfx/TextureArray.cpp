#include "TextureArray.hpp"
#include <glad/gl.h>
#include "../util/GLHelper.hpp"
#include <SDL3_image/SDL_image.h>
#include <vector>

// Function to crop an SDL surface to a specific region
SDL_Surface* cropSurface(SDL_Surface* surface, int x, int y, int width, int height) {
    // Create a new surface with the desired dimensions
    SDL_Surface* croppedSurface = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGB24);

    if (croppedSurface == nullptr) {
        std::cerr << "SDL_CreateRGBSurface failed: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    // Set blend mode to copy
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
    SDL_SetSurfaceBlendMode(croppedSurface, SDL_BLENDMODE_NONE);

    // Copy pixel data from the original surface to the cropped surface
    SDL_Rect srcRect = { x, y, width, height };
    SDL_Rect dstRect = { 0, 0, width, height };
    SDL_BlitSurface(surface, &srcRect, croppedSurface, &dstRect);

    return croppedSurface;
}

TextureArray::TextureArray(const char* path)
{
    glCall(glGenTextures(1, &id));
    glCall(glBindTexture(GL_TEXTURE_2D_ARRAY, id));
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    glCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    glCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCall(glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

    SDL_Surface* surface = IMG_Load(path);
    if (surface == nullptr) {
        std::cerr << "SDL_CreateRGBSurface failed: " << SDL_GetError() << std::endl;
    }
    else {
        int tileW = 8;           // number of pixels in a row of 1 tile
        int tileH = 8;

        int tilesX = 16;
        int tilesY = 16;
        int imageCount = tilesX * tilesY;
        glCall(glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
            tileW, tileH, imageCount, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
        for (int iy = 0; iy < tilesY; ++iy)
        {
            for (int ix = 0; ix < tilesX; ++ix)
            {
                int x = ix * tileW;
                int y = iy * tileH;
                SDL_Surface* cropped = cropSurface(surface, x, y, tileW, tileH);
                int i = iy * tilesX + ix;
                glCall(glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,
                    0, 0, i,
                    tileW, tileH, 1,
                    GL_RGBA, GL_UNSIGNED_BYTE, cropped->pixels));
                SDL_DestroySurface(cropped);
            }
        }
        SDL_DestroySurface(surface);
    }
}

void TextureArray::use(GLenum texture)
{
    glCall(glActiveTexture(texture));
    glCall(glBindTexture(GL_TEXTURE_2D_ARRAY, this->id));
}

TextureArray::~TextureArray()
{
}