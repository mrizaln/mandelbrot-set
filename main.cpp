#include <iostream>
#include <complex>
#include <random>
#include <ctime>
#include <cmath>
#include <sstream>

#include "grid.h"


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

class MandelbrotSet: public Grid<std::complex<double>>
{
private:
    Grid_type m_linearSet{};
    Grid_type m_mandelbrotSet{};

public:
    MandelbrotSet(
        std::size_t width,
        std::size_t height
    )
        : Grid(width, height)
        , m_linearSet{ std::vector<std::vector<Cell_type>>(height, std::vector<Cell_type>(width)) }
        , m_mandelbrotSet{ std::vector<std::vector<Cell_type>>(height, std::vector<Cell_type>(width)) }
    {
    }

    void generateLinearSet()
    {
        // from -2 to 2 (of y component)
        const double yDelta{ 4/static_cast<double>(m_height) };

        // preserve 1:1 ratio on the graph
        const double aspectRatio{ static_cast<double>(m_width)/m_height };
        const double xDelta{ 4/static_cast<double>(m_width) * aspectRatio };

        for (std::size_t y{ 0 }; y < m_height; ++y)
        {
            for (std::size_t x{ 0 }; x < m_width; ++x)
            {
                const std::complex<double> offset{ -2.0 * aspectRatio, -2.0 };
                m_linearSet[y][x] = { static_cast<double>(x)*xDelta + xDelta/2.0, static_cast<double>(y)*yDelta + yDelta/2.0 };
                m_linearSet[y][x] += offset;
            }
        }
    }

    std::vector<std::vector<bool>> generateMandelbrotSet(std::size_t iteration)
    {
        generateLinearSet();

        // iterative
        for (std::size_t i{ 0 }; i < iteration; ++i)
        {
            apply(m_linearSet, [&](Cell_type& cell, Cell_type& other){
                cell = cell*cell + other;
            });
        }

        // // recursive
        // m_grid = m_linearSet;

        // std::function<Cell_type(std::size_t, Cell_type&)> z_func{
        //     [&](std::size_t iteration, Cell_type& cell)->Cell_type {
        //         if (iteration == 0)
        //             return 0;
        //         return z_func(iteration-1, cell) * z_func(iteration-1, cell) + cell;
        //     }
        // };

        // auto apply_z_func{
        //     [&](Cell_type& cell)->void{
        //         cell = z_func(iteration, cell);
        //     }
        // };

        // apply(apply_z_func);

        std::vector<std::vector<bool>> mandelbrotSet{ std::vector<std::vector<bool>>(m_height, std::vector<bool>(m_width)) };
        for (std::size_t y{ 0 }; y < m_height; ++y)
            for (std::size_t x{ 0 }; x < m_width; ++x)
                mandelbrotSet[y][x] = isMandelbrotSet(x, y);

        return mandelbrotSet;
    }

    bool isMandelbrotSet(std::size_t x, std::size_t y)
    {
        const auto& real{ m_grid[y][x].real() };
        const auto& imag{ m_grid[y][x].imag() };
        double magnitude{ std::sqrt(real*real + imag*imag) };

        constexpr double boundary{ 2.0 };

        // std::cout << m_grid[y][x] << '\t' << magnitude << '\t' << (magnitude <= boundary && magnitude > 0) << '\n';

        return magnitude <= boundary && magnitude > 0;
    }
};

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