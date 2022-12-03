#include <iostream>
#include <random>
#include <ctime>
#include <sstream>

#include "mandelbrot_set.h"


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
    int width{ 40 };
    int height{ 40 };
    if (argc > 1)
    {
        char tmp{};
        std::stringstream ss{ argv[1] };
        ss >> width;
        ss >> tmp;
        ss >> height;
    }

    int iteration( 10 );
    if (argc > 2)
    {
        std::stringstream ss{ argv[2] };
        ss >> iteration;
    }

    MandelbrotSet set{ width, height };

    auto mandelbrotSet{ set.generateMandelbrotSet(iteration) };
    for (auto row{ mandelbrotSet.rbegin() }; row != mandelbrotSet.rend(); ++row)
    {
        for (const auto& cell: *row)
        {
            switch (cell)
            {
            case 0: std::cout << "  "; break;
            case 1: std::cout << "##"; break;
            }
        }
        std::cout << '\n';
    }
            

    // set.apply([&](std::complex<double>& cell)->void{
    //     cell = { getRandomNormal(), getRandomNormal() };
    // });

    // std::cout << set;
}