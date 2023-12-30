#pragma once

#include <cstddef>

struct page_cfg {
    static constexpr std::size_t PAGE_SIZE = 4096;
    static constexpr std::size_t PAGE_COUNT = 16;
};