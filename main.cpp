#include <iostream>
#include <random>
#include <ctime>
#include <sstream>

#include "mandelbrot_set.h"
#include "render.h"


int getRandomNumber(int min, int max)
{
    static std::mt19937 mt{ static_cast<std::mt19937::result_type>(std::time(nullptr)) };
    return std::uniform_int_distribution{ min, max }(mt);
}

double getRandomNormal()
{
    int num{ getRandomNumber(0,std::numeric_limits<int>::max()) };
    float numNormalized{ static_cast<float>(num) / std::numeric_limits<int>::max() };
    return numNormalized;   
}

int main(int argc, char** argv)
{
    std::size_t width{ 400 };
    std::size_t height{ 400 };
    if (argc > 1)
    {
        if (std::string{ argv[1] } == "-h")
        {
            std::cout << "Usage: " << argv[0] << " <width, height> <iteration> <radius>\n";
            return 0;
        }

        char tmp{};
        std::stringstream ss{ argv[1] };
        ss >> width;
        ss >> tmp;
        ss >> height;
    }

    int iteration{ 20 };
    if (argc > 2)
    {
        std::stringstream ss{ argv[2] };
        ss >> iteration;
    }

    double radius{ 100.0 };
    if (argc > 3)
    {
        std::stringstream ss{ argv[3] };
        ss >> radius;
    }

    MandelbrotSet<double> set{ width, height };
    set.modifyCenter(-0.75, 0);
            
    RenderEngine::initialize(set, width, height, iteration, radius);
    while (!RenderEngine::shouldClose())
    {
        RenderEngine::render();
    }
}
