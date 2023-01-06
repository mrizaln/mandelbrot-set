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
    using Value_type = T;
    using Cell_type = std::complex<Value_type>;
    using Grid_type = UnrolledMatrix<Cell_type>;

private:
    Grid_type m_grid{};

    std::size_t m_width{};
    std::size_t m_height{};

    Value_type m_xCenter{ 0.0 };
    Value_type m_yCenter{ 0.0 };

    Value_type m_magnification{ 1.0 };

    Value_type m_xDelta{};
    Value_type m_yDelta{};


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
        // const Value_type yDelta{ (4/static_cast<Value_type>(m_height))/m_magnification };
        m_yDelta = (4/static_cast<Value_type>(m_height))/m_magnification;

        // preserve 1:1 ratio on the graph
        const Value_type aspectRatio{ static_cast<Value_type>(m_width)/m_height };
        // const Value_type xDelta{ (4/static_cast<Value_type>(m_width) * aspectRatio)/m_magnification };
        m_xDelta = (4/static_cast<Value_type>(m_width) * aspectRatio)/m_magnification;

        for (std::size_t y{ 0 }; y < m_height; ++y)
        {
            for (std::size_t x{ 0 }; x < m_width; ++x)
            {
                const Cell_type offset{
                    m_xCenter - (2.0*aspectRatio)/m_magnification,
                    m_yCenter - 2.0/m_magnification
                };
                // m_grid(x, y) = { static_cast<Value_type>(x)*xDelta + xDelta/2.0, static_cast<Value_type>(y)*yDelta + yDelta/2.0 };
                m_grid(x, y) = { static_cast<Value_type>(x)*m_xDelta + m_xDelta/2.0, static_cast<Value_type>(y)*m_yDelta + m_yDelta/2.0 };
                m_grid(x, y) += offset;
            }
        }

        // std::cout << m_linearSet;
    }

    UnrolledMatrix<std::pair<Value_type, int>> generateMandelbrotSet(std::size_t iteration,  Value_type radius=1000.0)
    {
        generateLinearSet();

        auto magnitude_squared{ [&](const Cell_type& c){
            return c.real() * c.real() + c.imag() * c.imag();
        } };

        auto escapeCount{ [&](std::pair<Value_type, int>, const Cell_type& other)->std::pair<Value_type, int> {
            Cell_type c{ other };
            Cell_type Z{ c };
            Cell_type der{ 1 };

            int i{ 0 };
            Value_type squareModulus{};
            for (; i < iteration; ++i)
            {
                if ((squareModulus = std::norm(Z)) > radius*radius)
                    return { squareModulus, i };

                constexpr Cell_type mul{ 2.0, 2.0 };
                constexpr Value_type eps{ 1.0 };
                if ((std::norm(der = der*mul*Z)) < eps*eps)
                    return { 0.0, iteration };

                Z = Z*Z + c;
            }
            return { squareModulus, i };
        } };

        UnrolledMatrix<std::pair<Value_type, int>> mandelbrotSet{ m_width, m_height };

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
    const Value_type getXDelta() const { return m_xDelta; }
    const Value_type getYDelta() const { return m_yDelta; }
    const Value_type getXCenter() const { return m_xCenter; }
    const Value_type getYCenter() const { return m_yCenter; }
    const Value_type getMagnification() const { return m_magnification; }
    const auto& getGrid() const { return m_grid; }

    void modifyDimension(const std::size_t width, const std::size_t height)
    {
        m_width = width;
        m_height = height;

        // updateGrids();
    }

    void modifyCenter(const Value_type xPos, const Value_type yPos)
    {
        m_xCenter = xPos;
        m_yCenter = yPos;

        // updateGrids();
    }

    void magnify(const Value_type magnitude)
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
