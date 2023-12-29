#pragma once

#include <vector>
#include <memory>
#include <ostream>
#include <string_view>

#include "br_defs.hpp"
#include "br_replacer.hpp"

struct page_id {
    static constexpr int INVALID_PAGE = -1;
    static constexpr int INVALID_FD = -1;

    int fd{INVALID_FD};
    int page_no{INVALID_PAGE};

    page_id() {}
    page_id(int p_fd, int p_pg) :fd(p_fd), page_no(p_pg) {}

    friend std::ostream &operator<<(std::ostream& os, const page_id& self) {
        os << "pid( " << self.fd << " " << self.page_no << ")";
        return os;
    }
};

struct page {
    page_id pid;
    bool dirty{false};
    uint8_t *data{nullptr};

    void mark_dirty() {
        dirty = true;
    }
};

class pager {
public:

private:
    std::unique_ptr<uint8_t> buffer_;
    std::vector<page> pages_;
    lru_replacer<std::size_t, page_cfg::PAGE_COUNT> rpl_;

    std::list<std::size_t> free_pages_;

};
