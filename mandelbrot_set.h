#ifndef MANDELBROT_SET_H
#define MANDELBROT_SET_H

#include <complex>
#include <cmath>
#include <utility>      // std::pair

#include "./unrolled_matrix.h"

// any T that can apply to std::complex<T>
template <typename T=double>
class MandelbrotSet
{
public:
    using Cell_type = std::complex<T>;
    using Grid_type = UnrolledMatrix<Cell_type>;

private:
    Grid_type m_grid{};

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
    {
    }

    void generateLinearSet()
    {
        m_grid = { m_width, m_height };

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
                m_grid(x, y) = { static_cast<double>(x)*xDelta + xDelta/2.0, static_cast<double>(y)*yDelta + yDelta/2.0 };
                m_grid(x, y) += offset;
            }
        }

        // std::cout << m_linearSet;
    }

    UnrolledMatrix<std::pair<double, int>> generateMandelbrotSet(std::size_t iteration, double radius=1000.0)
    {
        generateLinearSet();

        auto magnitude_squared{ [&](const Cell_type& c){
            return c.real() * c.real() + c.imag() * c.imag();
        } };

        auto escapeCount{ [&](std::pair<double, int>, const Cell_type& other)->std::pair<double, int> {
            Cell_type temp{ 0 };
            int i{ 0 };
            double squareModulus{ magnitude_squared(temp) };
            for (; i < iteration; ++i)
            {
                const double boundary{ radius*radius };
                if ((squareModulus = magnitude_squared(temp)) > boundary)
                    return { squareModulus, i };
                temp = temp*temp + other;
            }
            return { squareModulus, i };
        } };

        UnrolledMatrix<std::pair<double, int>> mandelbrotSet{ m_width, m_height };
        
        std::transform(
            std::execution::par_unseq,
            mandelbrotSet.base().begin(),       // in_1 begin
            mandelbrotSet.base().end(),         // in_1 end
            m_grid.base().begin(),              // in_2 begin
            mandelbrotSet.base().begin(),       // out  begin
            escapeCount
        );

        return mandelbrotSet;
    }

    const std::size_t getWidth() const { return m_width; }
    const std::size_t getHeight() const { return m_height; }
    const std::pair<std::size_t, std::size_t> getDimension() const { return { m_width, m_height }; }
    const double getXCenter() const { return m_xCenter; }
    const double getYCenter() const { return m_yCenter; }
    const double getMagnification() const { return m_magnification; }
    const auto& getGrid() const { return m_grid; }

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
    }
};


#endif
