#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>  // include glad to get all the required OpenGL headers
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <iostream>
#include <limits>

#include "util/timer.hpp"


class Texture
{
public:
    using GL_texture_filter_type = int;

private:
    static inline unsigned int s_textureUnitCount{ 0 };

    int imageWidth{};
    int imageHeight{};
    int nrChannels{};

    unsigned char* imageData{};

    GL_texture_filter_type magFilter{};
    GL_texture_filter_type minFilter{};
    GL_texture_filter_type wrapFilter{};

public:
    static inline constexpr unsigned int maxUnitNum{ std::numeric_limits<unsigned int>::max() };

    unsigned int textureUnitNum{ maxUnitNum };      // means no texture loaded
    unsigned int textureID;     // don't change this value outside of Texture class

    Texture(const Texture&) = default;

    // texture but basic material actually
    Texture(
        const unsigned char red=0x0,
        const unsigned char green=0x0,
        const unsigned char blue=0x0
    )
    {
        textureUnitNum = s_textureUnitCount++;

        nrChannels = 3;

        imageData = new unsigned char[3];

        imageData[0] = red;
        imageData[1] = green;
        imageData[2] = blue;

        imageWidth = 1;
        imageHeight = 1;

        generateTexture();

        delete[] imageData;
        imageData = nullptr;
    }

    Texture(
        unsigned char*& colorData,
        int width,
        int height,
        int channels=3,
        int minFilter=GL_LINEAR,
        int magFilter=GL_NEAREST,
        int wrap=GL_REPEAT
    )
        : imageData{ colorData }
        , imageWidth{ width }
        , imageHeight{ height }
        , nrChannels{ channels }
    {
        textureUnitNum = s_textureUnitCount++;

        if(!imageData)
            std::cerr << "Failed to load texture: " << (void*)imageData << '\n';
        else
            generateTexture(minFilter, magFilter, wrap);
    }

    Texture(
        const char* texFilePath,
        int minFilter=GL_LINEAR,
        int magFilter=GL_NEAREST,
        int wrap=GL_REPEAT,
        bool flipVertically=true
    )
    {
        // textureUnitNum = s_textureUnitCount++;

        stbi_set_flip_vertically_on_load(flipVertically);
        imageData = stbi_load(texFilePath, &imageWidth, &imageHeight, &nrChannels, 0);

        if(!imageData)
            std::cerr << "Failed to load texture: " << texFilePath << '\n';
        else
            generateTexture(minFilter, magFilter, wrap);
    
        stbi_image_free(imageData);
        imageData = nullptr;
    }

    void updateTexture(unsigned char* data, int width, int height, int numChannels=3)
    {
        util::Timer timer{ "updateTexture" };
        glBindTexture(GL_TEXTURE_2D, textureID);

        int format{ GL_RGB };
        if (numChannels > 3)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    void updateMagFilter(GL_texture_filter_type magFilter)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
    }

    void updateMinFilter(GL_texture_filter_type minFilter)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
    }

    void updateWrapFilter(GL_texture_filter_type wrap)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    }

    void updateFilters(GL_texture_filter_type magFilter, GL_texture_filter_type minFilter, GL_texture_filter_type wrap)
    {
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    }

private:
    // TODO: make a generateTexture() function that accepts parameters of texture wrap, texture min filter, and texture mag filter.
    void generateTexture(int minFilter=GL_LINEAR, int magFilter=GL_NEAREST, int wrap=GL_REPEAT)
    {
        // generate texture
        glGenTextures(1, &textureID);

        // bind texture
        glBindTexture(GL_TEXTURE_2D, textureID);

        // set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);

        // now generate texture from image
        int imageFormat{ GL_RGB };
        if (nrChannels < 3)
        {
            std::cerr << "Image's color channel is less than 3 (no function can handle that for now)" << '\n';
            return;
        }
        else if (nrChannels == 4)
            imageFormat = GL_RGBA;
 
        glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, imageWidth, imageHeight, 0, imageFormat, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
};


#endif
