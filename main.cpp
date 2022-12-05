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
    std::size_t width{ 40 };
    std::size_t height{ 40 };
    if (argc > 1)
    {
        char tmp{};
        std::stringstream ss{ argv[1] };
        ss >> width;
        ss >> tmp;
        ss >> height;
    }

    int iteration{ 5 };
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

    MandelbrotSet<long double> set{ width, height };
    set.modifyCenter(-0.75, 0);
            
    RenderEngine::initialize(set, width, height, iteration, radius);
    while (!RenderEngine::shouldClose())
    {
        RenderEngine::render();
    }
}