#ifndef RENDER_H
#define RENDER_H

// #define DEBUG

#include <algorithm>        // for std::min
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tile/tile.h>

#include "./mandelbrot_set.h"


namespace RenderEngine
{
    GLFWwindow* initializeWindow();

    void framebuffer_size_callback(GLFWwindow*, int, int);
    void cursor_position_callback(GLFWwindow*, double, double);
    void scroll_callback(GLFWwindow*, double, double);
    void key_callback(GLFWwindow*, int, int, int, int);

    int shouldClose();
    void resetCamera(bool=false);
    void processInput(GLFWwindow*);
    void updateStates();
    void updateDeltaTime();


//=================================================================================================

    namespace configuration
    {
        int width{ 800 };
        int height{ 600 };
        float aspectRatio{ 800/static_cast<float>(600) };
    }

    namespace timing
    {
        float lastFrame{};
        float deltaTime{};
        float sumTime{};
    }

    namespace mouse
    {
        float lastX{};
        float lastY{};
        bool firstMouse { true };
        bool captureMouse{ false };

        bool leftButtonPressed{};
        bool rightButtonPressed{};
    }

    namespace view
    {
        struct Point2D { double x{}; double y{}; } position{};
        double speed{};
        double zoom{};
    }

    namespace simulation
    {
        bool pause{ false };
        int iteration{ 5 };
    }

    namespace data
    {
        using Data_type = MandelbrotSet;

        Data_type* dataPtr{};
        GLFWwindow* window{};
        Tile* tile{};
    }


//=================================================================================================

    int initialize(data::Data_type& data, int width=800, int height=600, int iteration=5)
    {
        configuration::width = width;
        configuration::height = height;

        data::window = initializeWindow();
        if (!data::window)
        {
            std::cout << "There's an error when creating window.\n";
            glfwTerminate();
            return -1;
        }

        data::dataPtr = &data;

        data::tile = new Tile{
            2.0f,
            "./shaders/shader.vs", "./shaders/shader.fs",   // Shader
            { 0x0, 0x0, 0x0 }                               // Texture
        };

        simulation::iteration = iteration;

        view::speed = 1.0f;

        resetCamera(true);

        return 0;
    }

    void render()
    {
        glClearColor(0.1f, 0.1f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw
        //------
        // use shader
        data::tile->m_shader.use();
        data::tile->draw();
        glfwSwapBuffers(data::window);
        //------

        // input
        glfwPollEvents();
        processInput(data::window);

        // delta time
        updateDeltaTime();

        // update states
        updateStates();    // update grid if timing::sumTime > delay
    }



//=================================================================================================

    GLFWwindow* initializeWindow()
    {
        // initialize glfw
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // window creation
        GLFWwindow* window{ glfwCreateWindow(configuration::width, configuration::height, "Traffic Model", NULL, NULL) };
        if (!window)
        {
            std::cerr << "Failed to create GLFW window\n";
            return nullptr;
        }
        glfwMakeContextCurrent(window);

        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to initialize glad\n";
            return nullptr;
        }

        // set callbacks
        //--------------
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetCursorPosCallback(window, cursor_position_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetKeyCallback(window, key_callback);

        // glEnable(GL_DEPTH_TEST);         // depth test
        // glEnable(GL_BLEND);              // transparency

        return window;
    }


    // window resize callback
    void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height);
        configuration::aspectRatio = width / static_cast<float>(height);
        configuration::width = width;
        configuration::height = height;
    }


    // cursor position callback
    void cursor_position_callback(GLFWwindow* window, double xPos, double yPos)
    {
        if (!mouse::captureMouse)
            return;

        if (mouse::firstMouse)
        {
            mouse::lastX = xPos;
            mouse::lastY = yPos;
            mouse::firstMouse = false;
        }

        float xOffset { static_cast<float>(xPos) - mouse::lastX };
        float yOffset { mouse::lastY - static_cast<float>(yPos) };

        view::position.x += xOffset * view::speed / (200.0f*view::zoom);
        view::position.y += yOffset * view::speed / (200.0f*view::zoom);

        mouse::lastX = xPos;
        mouse::lastY = yPos;
    }


    // scroll callback
    void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
    {
        constexpr float multiplier{ 1.1f };
        // view::zoom = (yOffset > 0 ? view::zoom * multiplier : view::zoom / multiplier );
        data::dataPtr->magnify(
            (yOffset > 0 ? multiplier : 1/multiplier)
        );
        view::zoom = data::dataPtr->getMagnification();
    }

    // key press callback (for 1 press)
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        // close window
        if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // toggle capture mouse
        if (key == GLFW_KEY_C && action == GLFW_PRESS)
        {
            // toggle
            mouse::captureMouse = !mouse::captureMouse;

            if (mouse::captureMouse)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            else
            {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                mouse::firstMouse = true;
            }
        }

        // set view target to box
        if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
        {
            resetCamera();
        }

        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        {
            simulation::pause = !simulation::pause;
        }

        if (key == GLFW_KEY_R && action == GLFW_PRESS)
        {
            resetCamera(true);
        }
    }


    int shouldClose()
    {
        if (data::window)
            return glfwWindowShouldClose(data::window);
        else
            return false;
    }

    void resetCamera(bool resetZoom)
    {        
        // center view
        view::position = { 0.0f, 0.0f };

        if (resetZoom)
            view::zoom = 1.0f;
    }

    // rgba texture (4 channels)
    unsigned char* newTexture()
    {
        constexpr int numOfChannels{ 4 };
        auto mandelbrotSet{ data::dataPtr->generateMandelbrotSet_unrolled(simulation::iteration * std::log(1+view::zoom)) };

        unsigned char* imageData = new unsigned char[configuration::width*configuration::height*numOfChannels];
        for (std::size_t i{ 0 }; i < configuration::width*configuration::height; ++i)
        {
            bool cell{ mandelbrotSet[i] };
            switch (cell)
            {
            case true:
                imageData[numOfChannels*i+0] = 0x19;
                imageData[numOfChannels*i+1] = 0x19;
                imageData[numOfChannels*i+2] = 0x1c;
                imageData[numOfChannels*i+3] = 0xff;      // alpha
                break;

            case false:
                imageData[numOfChannels*i+0] = 0xe3;
                imageData[numOfChannels*i+1] = 0xe3;
                imageData[numOfChannels*i+2] = 0xe6;
                imageData[numOfChannels*i+3] = 0xff;      // alpha
                break;
            }
        }
        return imageData;
    }

    void updateStates()
    {
        // update dimension and position
        data::dataPtr->modifyDimension(configuration::width, configuration::height);
        data::dataPtr->modifyCenter(view::position.x, view::position.y);

        // generate new texture based on mandelbrot set
        constexpr int numOfChannels{ 4 };
        auto imageData = newTexture();      // 4 channels
        data::tile->m_texture.updateTexture(imageData, configuration::width, configuration::height, numOfChannels);
        delete[] imageData;
        imageData = nullptr;

        std::cout << "It : " << simulation::iteration * std::log(std::sqrt(1+view::zoom)) << '\n';
        std::cout << "Mag: " << data::dataPtr->getMagnification() <<'\n';
        std::cout << "Loc: " << data::dataPtr->getXCenter() << '/' << data::dataPtr->getYCenter() << '\n';
        std::cout << "Dim: " << data::dataPtr->getWidth() << '/' << data::dataPtr->getHeight() << '\n';
        std::cout << "Vp : " << configuration::width << '/' << configuration::height << '\n';
        std::cout << '\n';
    }

    // for continuous input
    void processInput(GLFWwindow* window)
    {
        // view movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            view::position.y += view::speed * timing::deltaTime / view::zoom;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            view::position.y -= view::speed * timing::deltaTime / view::zoom;

        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            view::position.x += view::speed * timing::deltaTime / view::zoom;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            view::position.x -= view::speed * timing::deltaTime / view::zoom;
        }

        // speed
        if      (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            view::speed *= 1.01;
        else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            view::speed /= 1.01;
    }


    // record frame draw time
    void updateDeltaTime()
    {
        float currentFrame{ static_cast<float>(glfwGetTime()) };
        timing::deltaTime = currentFrame - timing::lastFrame;
        timing::lastFrame = currentFrame;
        timing::sumTime += timing::deltaTime;
    }
}


#endif