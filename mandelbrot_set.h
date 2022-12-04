#ifndef MANDELBROT_SET_H
#define MANDELBROT_SET_H

#include <complex>
#include <cmath>
#include <utility>      // std::pair

#include "./unrolled_matrix.h"

class MandelbrotSet
{
public:
    using Cell_type = std::complex<double>;
    using Grid_type = UnrolledMatrix<Cell_type>;

private:
    Grid_type m_grid{};
    Grid_type m_linearSet{};

    std::size_t m_width{};
    std::size_t m_height{};

    double m_xCenter{ 0.0 };
    double m_yCenter{ 0.0 };

    double m_magnification{ 1.0 };


public:
    MandelbrotSet(
        std::size_t width,
        std::size_t height
    )
        : m_width{ width }
        , m_height{ height }
        , m_grid{ width, height }
        , m_linearSet{ width, height }
    {
    }

    void generateLinearSet()
    {
        m_linearSet = { m_width, m_height };

        // from -2 to 2 (of y component)
        const double yDelta{ (4/static_cast<double>(m_height))/m_magnification };

        // preserve 1:1 ratio on the graph
        const double aspectRatio{ static_cast<double>(m_width)/m_height };
        const double xDelta{ (4/static_cast<double>(m_width) * aspectRatio)/m_magnification };

        for (std::size_t y{ 0 }; y < m_height; ++y)
        {
            for (std::size_t x{ 0 }; x < m_width; ++x)
            {
                const std::complex<double> offset{
                    m_xCenter - (2.0*aspectRatio)/m_magnification,
                    m_yCenter - 2.0/m_magnification
                };
                m_linearSet(x, y) = { static_cast<double>(x)*xDelta + xDelta/2.0, static_cast<double>(y)*yDelta + yDelta/2.0 };
                m_linearSet(x, y) += offset;
            }
        }

        // std::cout << m_linearSet;
    }

    std::vector<std::vector<bool>> generateMandelbrotSet(std::size_t iteration)
    {
        generateLinearSet();
        m_grid = { m_width, m_height };

        m_grid.apply(m_linearSet, [&](Cell_type& cell, Cell_type& other)->Cell_type{
            Cell_type temp { cell };
            for (int i{ 0 }; i < iteration; ++i)
                temp = temp*temp + other;
            return temp;
        });

        std::vector<std::vector<bool>> mandelbrotSet{ std::vector<std::vector<bool>>(m_height, std::vector<bool>(m_width)) };
        for (std::size_t y{ 0 }; y < m_height; ++y)
            for (std::size_t x{ 0 }; x < m_width; ++x)
                mandelbrotSet[y][x] = isMandelbrotSet(x, y);

        return mandelbrotSet;
    }

    std::vector<bool> generateMandelbrotSet_unrolled(std::size_t iteration)
    {
        generateLinearSet();
        m_grid = { m_width, m_height };

        m_grid.apply(m_linearSet, [&](Cell_type& cell, Cell_type& other)->Cell_type{
            Cell_type temp { cell };
            for (int i{ 0 }; i < iteration; ++i)
                temp = temp*temp + other;
            return temp;
        });

        std::vector<bool> mandelbrotSet(m_width*m_height);
        for (std::size_t y{ 0 }; y < m_height; ++y)
        {
            for (std::size_t x{ 0 }; x < m_width; ++x)
            {
                std::size_t idx{ y * m_width + x };
                mandelbrotSet[idx] = isMandelbrotSet(x, y);
            }
        }

        return mandelbrotSet;
    }

    bool isMandelbrotSet(std::size_t x, std::size_t y)
    {
        const auto& real{ m_grid(x, y).real() };
        const auto& imag{ m_grid(x, y).imag() };
        double magnitude_squared{ real*real + imag*imag };

        constexpr double boundary_squared{ 4.0 };

        // std::cout << m_grid[y][x] << '\t' << magnitude << '\t' << (magnitude <= boundary && magnitude > 0) << '\n';

        return magnitude_squared <= boundary_squared;
    }

    const std::size_t getWidth() const { return m_width; }
    const std::size_t getHeight() const { return m_height; }
    const std::pair<std::size_t, std::size_t> getDimension() const { return { m_width, m_height }; }
    const double getXCenter() const { return m_xCenter; }
    const double getYCenter() const { return m_yCenter; }
    const double getMagnification() const { return m_magnification; }

    void modifyDimension(const std::size_t width, const std::size_t height)
    {
        m_width = width;
        m_height = height;

        // updateGrids();
    }

    void modifyCenter(const double xPos, const double yPos)
    {
        m_xCenter = xPos;
        m_yCenter = yPos;

        // updateGrids();
    }

    void magnify(const double magnitude)
    {
        m_magnification *= magnitude;

        // updateGrids();
    }

    void updateGrids()
    {
        m_grid = { m_width, m_height };
        m_linearSet = { m_width, m_height };
    }
};


#endif
