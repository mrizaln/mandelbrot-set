#ifndef GRID_H
#define GRID_H

#include <vector>
#include <functional>
#include <algorithm>
#include <iostream>

template <typename T>
class Grid
{
public:
    using Cell_type = T;
    using Grid_type = std::vector<std::vector<Cell_type>>;

protected:
    Grid_type m_grid{};

    std::size_t m_width{};
    std::size_t m_height{};  

public:
    Grid(
        std::size_t width,
        std::size_t height
    )
        : m_width{ width }
        , m_height{ height }
        , m_grid{ std::vector<std::vector<Cell_type>>(height, std::vector<Cell_type>(width)) }
    {
    }

    virtual ~Grid() = default;

    void apply(std::function<void(Cell_type&)> func)
    {
        for (std::vector<Cell_type>& row : m_grid)
            for (Cell_type& cell : row)
                func(cell);
    }

    // modify m_grid by operating it with grid_other element by element
    void apply(Grid_type& grid_other, std::function<void(Cell_type&, Cell_type&)> func)
    {
        if ((grid_other.size() != m_grid.size()) || (grid_other[0].size() != m_grid[0].size()))
            throw std::length_error{ "grid_other size is not the same as m_grid!" };
        
        for (std::size_t y{ 0 }; y < m_height; ++y)
            for (std::size_t x{ 0 }; x < m_width; ++x)
                func(m_grid[y][x], grid_other[y][x]);
    }

    virtual std::ostream& print(std::ostream& out) const
    {
        for (const std::vector<Cell_type>& row : m_grid)
        {
            for (const Cell_type& cell : row)
                std::cout << cell << ' ';
            std::cout << '\n';
        }
        return out;
    }

    const auto& data() const { return m_grid; }
    auto& base() { return m_grid; }

    friend std::ostream& operator<<(std::ostream& out, const Grid& grid)
    {
        return grid.print(out);
    }
};



#endif