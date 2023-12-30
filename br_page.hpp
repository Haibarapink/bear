#pragma once

#include <cstring>
#include <cassert>
#include <vector>
#include <memory>
#include <ostream>
#include <string_view>

#include "br_io.hpp"
#include "br_defs.hpp"
#include "br_replacer.hpp"

struct page_id {
    static constexpr int INVALID_PAGE = -1;
    static constexpr int INVALID_FD = -1;

    int fd{INVALID_FD};
    int page_no{INVALID_PAGE};

    page_id() {}
    page_id(int p_fd, int p_pg) :fd(p_fd), page_no(p_pg) {}

    friend bool operator< (const page_id&l, const page_id& r) {
        return l.page_no > r.page_no;
    }

    bool operator== ( const page_id& other) {
        return this->page_no == other.page_no && this->fd == other.fd;
    }

    friend std::ostream &operator<<(std::ostream& os, const page_id& self) {
        os << "pid( " << self.fd << " " << self.page_no << ")";
        return os;
    }
};

struct page {
    page_id pid;
    bool dirty{false};
    uint8_t *data{nullptr};
    size_t pin_num{0};

    page() = default;
    ~page() = default;

    void pin() {
        pin_num++;
    }

    void unpin() {
        assert(pin_num != 0);
        pin_num--;
    }

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
    friend class pager_mgr;
public:
    pager(int fd):fd_(fd) {
        buffer_ = std::unique_ptr<uint8_t>(new uint8_t[page_cfg::PAGE_COUNT * page_cfg::PAGE_SIZE]);
        for (auto i = 0; i < page_cfg::PAGE_COUNT; ++i) {
            page p;
            p.dirty = false;
            p.data = buffer_.get() + i * page_cfg::PAGE_SIZE;
            pages_.push_back(p);
            free_pages_.push_back(i);
        }
        auto ok = store_mgr_.open(fd);
        if (!ok) {
            throw std::runtime_error{"store_mgr_ can not open fd"};
        }
    }

    ~pager() = default;

    pager& operator=(const pager&) = delete;
    pager(const pager&) = delete;

    std::optional<page*> fetch(page_id pid, bool existed) {
        if (auto iter = coord_.find(pid); iter != coord_.end()) {
            return &pages_[iter->second];
        }

        page *p;
        int idx = -1;
        if (!free_pages_.empty()) {
            // 1. 先从free_list中得到
            auto which= free_pages_.back();
            free_pages_.pop_back();
            p = &pages_[which];
            idx = static_cast<int>(which);
        } else {
            // 2. 淘汰一个页面
            auto op = rpl_.pop();
            if (op.has_value()) {
                idx = static_cast<int>(op.value());
                p = &pages_[idx];
            } else {
                return std::nullopt;
            }
            if (p->dirty) {
                write(p);
                p->dirty = false;
            }
        }

        p->pid = pid;
        p->pin();
        coord_.emplace(p->pid, idx);
        if (existed) {
            // read page
            // TODO handle error
            read(p);
        }
        return p;
    }

    void unpin(page* p, bool dirty = false) {
        assert(p);
        if (auto iter = coord_.find(p->pid); iter != coord_.end()) {
            assert(iter->first.fd == p->pid.fd && iter->first.page_no == p->pid.page_no);
            auto idx = iter->second;
            assert(rpl_.push(idx));
            if (dirty) p->mark_dirty();
            p->unpin();
        }
    }

private:
    bool read(page* p) {
        assert(p->pid.page_no != page_id::INVALID_PAGE && p->pid.fd == fd_ && fd_ != page_id::INVALID_FD);
        return store_mgr_.read(p->data, page_cfg::PAGE_SIZE, page_cfg::PAGE_SIZE *p->pid.page_no);
    }
    bool write(page* p) {
        return store_mgr_.write(p->data, page_cfg::PAGE_SIZE, p->pid.page_no * page_cfg::PAGE_SIZE);
    }
private:
    int fd_ = page_id::INVALID_FD;
    std::unique_ptr<uint8_t> buffer_;
    std::vector<page> pages_;
    lru_replacer<std::size_t, page_cfg::PAGE_COUNT> rpl_;
    std::map<page_id, size_t> coord_;

    std::list<std::size_t> free_pages_;
    sync_io_mgr store_mgr_;
};

class pager_mgr {
public:
private:
};
