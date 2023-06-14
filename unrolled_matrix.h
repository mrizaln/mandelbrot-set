#ifndef GRID_H
#define GRID_H

#include <iterator>
#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>
#include <execution>

#include "util/timer.hpp"

template <typename T>
class UnrolledMatrix
{
public:
    using Element_type = T;

private:
    std::vector<Element_type> m_mat{};

    std::size_t m_width{};
    std::size_t m_height{};

public:
    UnrolledMatrix() = default;

    UnrolledMatrix(
        std::size_t width,
        std::size_t height
    )
        : m_mat{ std::vector<Element_type>(width * height) }
        , m_width{ width }
        , m_height{ height }
    {
    }

    UnrolledMatrix(
        std::vector<Element_type>&& mat,
        std::size_t                 width,
        std::size_t                 height
    )
        : m_mat{ mat }
        , m_width{ width }
        , m_height{ height }
    {
    }

    ~UnrolledMatrix() = default;

    void apply(std::function<Element_type(Element_type&)> func)
    {
        std::transform(std::execution::par_unseq, m_mat.begin(), m_mat.end(), m_mat.begin(), func);
        // for (auto& element: m_mat)
        //{
        //     func(element);
        // }
    }

    template <typename T_other>
    void apply(UnrolledMatrix<T_other>& mat_other, std::function<Element_type(Element_type&, T_other&)> func)
    {
        util::Timer timer{ "apply on matrix" };
        auto [width, height]{ mat_other.getSize() };
        std::transform(std::execution::par_unseq, m_mat.begin(), m_mat.end(), mat_other.base().begin(), m_mat.begin(), func);
        // for (std::size_t y{ 0 }; y < height; ++y)
        //{
        //     for (std::size_t x{ 0 }; x < width; ++x)
        //     {
        //         func(this->getElement(x,y), mat_other.getElement(x,y));
        //     }
        // }
    }

    Element_type& getElement(std::size_t col, std::size_t row)
    {
        if (col < 0 || row < 0 || col >= m_width || row >= m_height)
            throw std::range_error{ "out of bound" };

        std::size_t idx{ row * m_width + col };
        return m_mat[idx];
    }

    const Element_type& getElement(std::size_t col, std::size_t row) const
    {
        if (col < 0 || row < 0 || col >= m_width || row >= m_height)
            throw std::range_error{ "out of bound" };

        std::size_t idx{ row * m_width + col };
        return m_mat[idx];
    }

    Element_type&       operator()(std::size_t col, std::size_t row) { return getElement(col, row); }
    const Element_type& operator()(std::size_t col, std::size_t row) const { return getElement(col, row); }

    // return pair of width, height
    std::pair<std::size_t, std::size_t> getSize() const { return { m_width, m_height }; }

    std::size_t getLength() const { return m_width * m_height; }

    const auto& data() const { return m_mat; }
    auto&       base() { return m_mat; }

    friend std::ostream& operator<<(std::ostream& out, const UnrolledMatrix& grid)
    {
        for (int y{ 0 }; y < grid.m_height; ++y) {
            for (int x{ 0 }; x < grid.m_width; ++x) {
                std::cout << grid(x, y) << ' ';
            }
            std::cout << '\n';
        }
        return out;
    }
};

#endif
