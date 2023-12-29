#pragma once

#include <cstring>
#include <cassert>
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

    template <size_t page_size>
    static void clean(page p) {
        memset(p.data, 0, page_size);
    }
};

class pager {
    friend class pager_test;
    friend class storage_mgr;
public:
    pager() {
        buffer_ = std::unique_ptr<uint8_t>(new uint8_t[page_cfg::PAGE_COUNT * page_cfg::PAGE_SIZE]);
        for (auto i = 0; i < page_cfg::PAGE_COUNT; ++i) {
            page p;
            p.dirty = false;
            p.data = buffer_.get() + i * page_cfg::PAGE_SIZE;
            pages_.push_back(p);
            free_pages_.push_back(i);
        }
    }

    ~pager() = default;

    pager& operator=(const pager&) = delete;
    pager(const pager&) = delete;

    std::optional<page> fetch(page_id pid, bool existed) {
        page p;
        int idx = -1;
        if (!free_pages_.empty()) {
            // 1. 先从free_list中得到
            auto which= free_pages_.back();
            free_pages_.pop_back();
            p = pages_[which];
            idx = static_cast<int>(which);
        } else {
            // 2. 淘汰一个页面
            auto op = rpl_.pop();
            if (op.has_value()) {
                idx = static_cast<int>(op.value());
                p = pages_[idx];
            } else {
                return std::nullopt;
            }
        }

        p.pid = pid;
        if (existed) {
            // read page
            // TODO handle error
            read(p);
        }
        return p;
    }
private:
    bool read(page p) {
        assert(p.pid.page_no != page_id::INVALID_PAGE && p.pid.fd == fd_ && fd_ != page_id::INVALID_FD);

    }
private:
    int fd_ = page_id::INVALID_FD;

    std::unique_ptr<uint8_t> buffer_;
    std::vector<page> pages_;
    lru_replacer<std::size_t, page_cfg::PAGE_COUNT> rpl_;

    std::list<std::size_t> free_pages_;
};
