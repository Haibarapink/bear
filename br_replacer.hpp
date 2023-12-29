#pragma once
#include <list>
#include <map>
#include <optional>

//  list的插入不会造成迭代器失效
template<typename element, size_t cap>
class lru_replacer {
    using list_iter = typename std::list<element>::iterator;
public:
    lru_replacer() = default;
    ~lru_replacer() = default;

    bool push(element e) {
        if (coord_.find(e) != coord_.end())
            return touch(e);

        // pop first pls
        if (l_.size() == cap) {
            return false;
        }
        l_.push_front(e);
        coord_.emplace(std::move(e), l_.begin());
        return true;
    }

    bool touch(const element& e) {
        auto iter = coord_.find(e);
        if (iter == coord_.end()) {
            return false;
        }
        l_.erase(iter->second);
        coord_.erase(iter);
        l_.push_front(e);
        coord_.emplace(e, l_.begin());
        return true;
    }

    auto pop() -> std::optional<element> {
        std::optional<element> res;
        if (l_.empty()) {
            return std::nullopt;
        }
        auto e = l_.back();
        coord_.erase(e);
        l_.pop_back();
        return std::optional<element>{e};
    }

private:
    std::list<element> l_;
    std::map <element, list_iter> coord_;
};