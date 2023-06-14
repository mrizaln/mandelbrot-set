#ifndef RENDER_H
#define RENDER_H

// #define DEBUG

#include <algorithm>    // for std::min
#include <iostream>
#include <vector>
#include <array>
#include <cstddef>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <tile/tile.h>

#include "./mandelbrot_set.h"

#include "util/timer.hpp"

namespace RenderEngine
{
    GLFWwindow* initializeWindow(const std::string&);

    void framebuffer_size_callback(GLFWwindow*, int, int);
    void cursor_position_callback(GLFWwindow*, double, double);
    void scroll_callback(GLFWwindow*, double, double);
    void key_callback(GLFWwindow*, int, int, int, int);

    int  shouldClose();
    void resetCamera(bool = false);
    void processInput(GLFWwindow*);
    void updateStates();
    void updateDeltaTime();
    void updateTitle();

    //=================================================================================================

    // aliases
    using Value_type       = double;
    using Data_type        = MandelbrotSet<Value_type>;
    using Pixel_type       = std::array<unsigned char, 4>;
    using TextureData_type = UnrolledMatrix<Pixel_type>;

    namespace configuration
    {
        int         width{ 800 };
        int         height{ 600 };
        float       aspectRatio{ 800 / static_cast<float>(600) };
        std::string windowName{ "Mandelbrot Set" };
    }

    namespace timing
    {
        double lastFrame{};
        double deltaTime{};
        double sumTime{};
    }

    namespace mouse
    {
        float lastX{};
        float lastY{};
        bool  firstMouse{ true };
        bool  captureMouse{ false };

        bool leftButtonPressed{};
        bool rightButtonPressed{};
    }

    namespace view
    {
        struct Point2D
        {
            Value_type x{};
            Value_type y{};
        } position{};
        Value_type speed{};
        Value_type zoom{};
    }

    namespace simulation
    {
        bool       pause{ false };
        int        iteration{ 5 };
        Value_type radius{ 100.0 };
    }

    namespace data
    {
        Data_type*  dataPtr{};
        GLFWwindow* window{};
        Tile*       tile{};
    }

    //=================================================================================================

    int initialize(Data_type& data, int width = 800, int height = 600, int iteration = 5, Value_type radius = 100.0)
    {
        configuration::width  = width;
        configuration::height = height;

        data::window = initializeWindow(configuration::windowName);
        if (!data::window) {
            std::cout << "There's an error when creating window.\n";
            glfwTerminate();
            return -1;
        }

        data::dataPtr = &data;

        data::tile = new Tile{
            2.0f,
            "./resources/shaders/shader.vs",
            "./resources/shaders/shader.fs",    // Shader
            { '\00', '\00', '\00' }             // Texture
        };

        simulation::iteration = iteration;
        simulation::radius    = radius;

        view::zoom       = 1.0;
        view::speed      = 1.0;
        view::position.x = data.getXCenter();
        view::position.y = data.getYCenter();

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

    GLFWwindow* initializeWindow(const std::string& windowName)
    {
        // initialize glfw
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // window creation
        GLFWwindow* window{ glfwCreateWindow(configuration::width, configuration::height, windowName.c_str(), NULL, NULL) };
        if (!window) {
            std::cerr << "Failed to create GLFW window\n";
            return nullptr;
        }
        glfwMakeContextCurrent(window);

        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
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
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
        configuration::aspectRatio = width / static_cast<float>(height);
        configuration::width       = width;
        configuration::height      = height;
    }

    // cursor position callback
    void cursor_position_callback(GLFWwindow* window, double xPos, double yPos)
    {
        if (!mouse::captureMouse)
            return;

        if (mouse::firstMouse) {
            mouse::lastX      = xPos;
            mouse::lastY      = yPos;
            mouse::firstMouse = false;
        }

        double xOffset{ static_cast<float>(xPos) - mouse::lastX };
        double yOffset{ mouse::lastY - static_cast<float>(yPos) };

        view::position.x += xOffset * view::speed / (200.0f * view::zoom);
        view::position.y += yOffset * view::speed / (200.0f * view::zoom);

        mouse::lastX = xPos;
        mouse::lastY = yPos;
    }

    // scroll callback
    void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
    {
        constexpr float multiplier{ 1.1f };
        // view::zoom = (yOffset > 0 ? view::zoom * multiplier : view::zoom / multiplier );
        data::dataPtr->magnify(
            (yOffset > 0 ? multiplier : 1 / multiplier)
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
        if (key == GLFW_KEY_C && action == GLFW_PRESS) {
            // toggle
            mouse::captureMouse = !mouse::captureMouse;

            if (mouse::captureMouse)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                mouse::firstMouse = true;
            }
        }

        // set view target to box
        if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS) {
            resetCamera();
        }

        if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
            simulation::pause = !simulation::pause;
        }

        if (key == GLFW_KEY_R && action == GLFW_PRESS) {
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

    void updateStates()
    {
        util::Timer timer{ "updateStates" };
        {
            // update dimension and position
            util::Timer timer1{ "modifyDimension and Center" };
            data::dataPtr->modifyDimension(configuration::width, configuration::height);
            data::dataPtr->modifyCenter(view::position.x, view::position.y);
        }

        // generate new texture based on mandelbrot set
        auto iteration{ simulation::iteration * std::sqrt(std::log(1 + view::zoom)) };
        // auto  radius{ simulation::radius / std::sqrt(std::log(1 + view::zoom)) };
        auto  radius{ simulation::radius };
        auto& imageData{ data::dataPtr->generateTexture(iteration, radius) };
        auto* imageDataPtr{ &imageData.base().front().front() };
        data::tile->m_texture.updateTexture(imageDataPtr, data::dataPtr->getWidth(), data::dataPtr->getHeight(), sizeof(Pixel_type));

        // output something
        if (!util::Timer::s_doPrint) {
            std::cout << std::format("It : {}\n", (int)iteration);
            std::cout << std::format("Rad: {}\n", radius);
            std::cout << std::format("Mag: {}\n", data::dataPtr->getMagnification());
            std::cout << std::format("Dim: {} | {}\n", data::dataPtr->getWidth(), data::dataPtr->getHeight());
            std::cout << std::format("Loc: {} | {}\n", data::dataPtr->getXCenter(), data::dataPtr->getYCenter());
            std::cout << std::format("d  : {} | {}\n", data::dataPtr->getXDelta(), data::dataPtr->getYDelta());

            std::cout << "\033[6A";    // move cursor up 6 lines
            std::cout << "\033[0J";    // clear from cursor to end of screen
        }
        updateTitle();
    }

    // for continuous input
    void processInput(GLFWwindow* window)
    {
        // view movement
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            view::position.y += view::speed * timing::deltaTime / view::zoom;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            view::position.y -= view::speed * timing::deltaTime / view::zoom;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            view::position.x += view::speed * timing::deltaTime / view::zoom;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            view::position.x -= view::speed * timing::deltaTime / view::zoom;
        }

        // speed
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            view::speed *= 1.01;
        else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            view::speed /= 1.01;
    }

    // record frame draw time
    void updateDeltaTime()
    {
        float currentFrame{ static_cast<float>(glfwGetTime()) };
        timing::deltaTime  = currentFrame - timing::lastFrame;
        timing::lastFrame  = currentFrame;
        timing::sumTime   += timing::deltaTime;
    }

    void updateTitle()
    {
        static int      counter{ 0 };
        static float    sum{ 0 };
        constexpr float timeInterval{ 1.0f };    // print every this time interval in seconds

        {
            counter++;
            if (sum >= timeInterval) {
                float       time{ sum / counter };
                float       fps{ 1 / time };
                std::string title{ std::format("{} [FPS: {:.1f} | {:.2f} ms]", configuration::windowName, fps, time * 1000) };
                glfwSetWindowTitle(data::window, title.c_str());
                sum     = 0.0f;
                counter = 0;
            }
            sum += timing::deltaTime;
        }
    }
}

#endif
