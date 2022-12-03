#ifndef MANDELBROT_SET_H
#define MANDELBROT_SET_H

#include <complex>
#include <cmath>

#include "./unrolled_matrix.h"

class MandelbrotSet
{
public:
    using Cell_type = std::complex<double>;
    using Grid_type = UnrolledMatrix<Cell_type>;

private:
    Grid_type m_grid{};
    Grid_type m_linearSet{};
    Grid_type m_mandelbrotSet{};

    std::size_t m_width{};
    std::size_t m_height{};

public:
    MandelbrotSet(
        std::size_t width,
        std::size_t height
    )
        : m_width{ width }
        , m_height{ height }
        , m_grid{ width, height }
        , m_linearSet{ width, height }
        , m_mandelbrotSet{ width, height }
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
                m_linearSet(x, y) = { static_cast<double>(x)*xDelta + xDelta/2.0, static_cast<double>(y)*yDelta + yDelta/2.0 };
                m_linearSet(x, y) += offset;
            }
        }
    }

    std::vector<std::vector<bool>> generateMandelbrotSet(std::size_t iteration)
    {
        generateLinearSet();

        // iterative
        for (std::size_t i{ 0 }; i < iteration; ++i)
        {
            m_grid.apply(m_linearSet, [&](Cell_type& cell, Cell_type& other)->Cell_type{
                return cell*cell + other;
            });
        }

        std::vector<std::vector<bool>> mandelbrotSet{ std::vector<std::vector<bool>>(m_height, std::vector<bool>(m_width)) };
        for (std::size_t y{ 0 }; y < m_height; ++y)
            for (std::size_t x{ 0 }; x < m_width; ++x)
                mandelbrotSet[y][x] = isMandelbrotSet(x, y);

        return mandelbrotSet;
    }

    bool isMandelbrotSet(std::size_t x, std::size_t y)
    {
        const auto& real{ m_grid(x, y).real() };
        const auto& imag{ m_grid(x, y).imag() };
        double magnitude{ std::sqrt(real*real + imag*imag) };

        constexpr double boundary{ 2.0 };

        // std::cout << m_grid[y][x] << '\t' << magnitude << '\t' << (magnitude <= boundary && magnitude > 0) << '\n';

        return magnitude <= boundary && magnitude > 0;
    }
};


#endif