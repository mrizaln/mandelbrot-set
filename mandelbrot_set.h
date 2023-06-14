#ifndef MANDELBROT_SET_H
#define MANDELBROT_SET_H

#include <complex>
#include <cmath>
#include <format>
#include <future>
#include <utility>    // std::pair

#include "./unrolled_matrix.h"
#include "util/timer.hpp"

// any T that can apply to std::complex<T>
template <typename T = double>
class MandelbrotSet
{
public:
    using Value_type       = T;
    using Cell_type        = std::complex<Value_type>;
    using Grid_type        = UnrolledMatrix<Cell_type>;
    using Pixel_type       = std::array<unsigned char, 4>;
    using TextureData_type = UnrolledMatrix<Pixel_type>;

private:
    TextureData_type m_texture{};

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
        , m_texture{ width, height }
    {
    }

    Cell_type getGridValue(std::size_t xPos, std::size_t yPos)
    {
        // from -2 to 2 (of y component)
        m_yDelta = (4 / static_cast<Value_type>(m_height)) / m_magnification;

        // preserve 1:1 ratio on the graph
        const Value_type aspectRatio{ static_cast<Value_type>(m_width) / m_height };
        m_xDelta = (4 / static_cast<Value_type>(m_width) * aspectRatio) / m_magnification;

        const Cell_type offset{
            m_xCenter - (2.0 * aspectRatio) / m_magnification,
            m_yCenter - 2.0 / m_magnification
        };
        Cell_type value{ static_cast<Value_type>(xPos) * m_xDelta + m_xDelta / 2.0, static_cast<Value_type>(yPos) * m_yDelta + m_yDelta / 2.0 };
        return value + offset;
    }

    TextureData_type& generateTexture(std::size_t iteration, Value_type radius = 1000.0)
    {
        util::Timer timer{ "generateMandelbrotSet" };

        // chunking
        std::size_t length{ m_width * m_height };
        std::size_t chunkNumber{ std::thread::hardware_concurrency() };
        std::size_t chunkSize{ length / chunkNumber };

        std::vector<std::future<void>> futures;
        for (std::size_t i{ 0 }; i < chunkNumber; i++) {
            auto startPos{ chunkSize * i };
            auto endPos{ startPos + chunkSize };
            if (endPos >= length) endPos = length;

            futures.emplace_back(std::async(std::launch::async, [this, &iteration, &radius, i, startPos, endPos] {
                util::Timer timer{ std::format("chunk {}", i) };
                for (std::size_t start{ startPos }; start < endPos; start++) {
                    std::size_t xPos{ start % m_width };
                    std::size_t yPos{ start / m_width };
                    Cell_type   c{ getGridValue(xPos, yPos) };
                    Cell_type   Z{ c };
                    Cell_type   der{ 1 };

                    std::size_t                               i{ 0 };
                    std::optional<std::pair<Value_type, int>> iterationData;
                    Value_type                                squareModulus{};
                    for (; i < iteration; ++i) {
                        if ((squareModulus = std::norm(Z)) > radius * radius) {
                            iterationData = { squareModulus, i };
                            break;
                        }

                        constexpr Cell_type  mul{ 2.0, 2.0 };
                        constexpr Value_type eps{ 0.1 };
                        if ((std::norm(der = der * mul * Z)) < eps * eps) {
                            iterationData = { 0.0, iteration };
                            break;
                        }

                        Z = Z * Z + c;
                    }

                    const auto& [value, iter]{ iterationData.has_value() ? iterationData.value() : std::pair{ squareModulus, (int)i } };

                    // generate number [0x00, 0xff]
                    const auto getColor{ [&iter, &value, &iteration](Value_type mul) -> unsigned char {
                        if (iter == iteration)
                            return 0x00;

                        // auto&  R{ value };
                        // auto   V{ std::max(0.0, 2 * std::log(R) / std::pow(2, iter)) };    // [0,idk)
                        // auto   x{ std::log(V) / std::log(1 + iter / 2) };
                        auto   x{ iter };

                        constexpr Value_type offset{ 0.2 };
                        const auto color{ static_cast<unsigned char>(0xff * (1 + (offset) / 2 - (1 - offset) * std::cos(mul * x)) / 2) };
                        return color;
                    } };

                    unsigned char r{ getColor(1 / (7.0 * std::pow(3.0, 0.25))) };
                    unsigned char g{ getColor(1 / (3.0 * std::sqrt(2.0))) };
                    unsigned char b{ getColor(1 / (2.0 * std::log(5.0))) };
                    unsigned char a{ 0xff };

                    m_texture.base()[start] = { r, g, b, a };
                }
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }

        return m_texture;
    }

    std::size_t                               getWidth() const { return m_width; }
    std::size_t                               getHeight() const { return m_height; }
    const std::pair<std::size_t, std::size_t> getDimension() const { return { m_width, m_height }; }
    const Value_type                          getXDelta() const { return m_xDelta; }
    const Value_type                          getYDelta() const { return m_yDelta; }
    const Value_type                          getXCenter() const { return m_xCenter; }
    const Value_type                          getYCenter() const { return m_yCenter; }
    const Value_type                          getMagnification() const { return m_magnification; }

    void modifyDimension(const std::size_t width, const std::size_t height)
    {
        if (m_width == width && m_height == height)
            return;
        m_width  = width;
        m_height = height;

        m_texture = { m_width, m_height };
    }

    void modifyCenter(const Value_type xPos, const Value_type yPos)
    {
        m_xCenter = xPos;
        m_yCenter = yPos;
    }

    void magnify(const Value_type magnitude)
    {
        m_magnification *= magnitude;
    }
};

#endif
