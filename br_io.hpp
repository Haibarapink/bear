#pragma once

#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <optional>
#include <string_view>
#include <cinttypes>

struct file_info {
    static std::optional<size_t> size(const char* c_str) {
        struct stat st {};
        int ok = stat(c_str, &st);
        if (ok == -1) {
            return std::nullopt;
        }
        return st.st_size;
    }
    static std::optional<size_t> size(int fd) {
        struct stat st {};
        int ok = fstat(fd, &st);
        if (ok == -1) {
            return std::nullopt;
        }
        return st.st_size;
    }
};

// TODO support readv
class sync_io_mgr {
public:
    sync_io_mgr() = default;
    ~sync_io_mgr() = default;

    bool open(int fd) {
        fd_ = fd;
    }
    bool read(uint8_t* data, size_t len, size_t offset) {
        assert(fd_ != -1);

        off_t sk_ok = lseek(fd_, offset, SEEK_SET);
        if (sk_ok == -1) {
            return false;
        }
        auto fz = file_info::size(fd_);
        if (!fz.has_value())
            return false;
        size_t remain_sz = fz.value() - offset;
        if (len > remain_sz) len = remain_sz;
        int rd_ok = ::read(fd_, (void*)data, remain_sz);
        if (rd_ok == -1) {
            return false;
        }
        // 多出部分设置为0
        if (len != remain_sz)
            memset(data +remain_sz, 0, len - remain_sz);
        return true;
    }

    bool write(uint8_t* data, size_t len, size_t offset) {
        assert(fd_ != -1);
        auto fz = file_info::size(fd_);
        if (!fz.has_value())
            return false;
        off_t sk_ok = lseek(fd_, offset, SEEK_SET);
        if (sk_ok == -1) {
            return false;
        }
        auto wr_ok = ::write(fd_, (void*)data, len);
        if (wr_ok == -1) {
            return false;
        }

        return true;
    }

    void close() {
        // nothing
    }
private:
    int fd_ = -1;   // fd不应该从这里关闭
};

// ... unused
class mmap_io_mgr {
public:
    mmap_io_mgr() = default;

    bool open(int fd) {
        struct stat st {};
        int ok = fstat(fd, &st);
        if (ok != 0) {
            return false;
        }

        void* res = mmap(0, st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        void* fail_code = (void*)-1;
        if (memcmp((void*)&res, (void*)&fail_code, sizeof(void*)) == 0) {
            return false;
        }
        data_ = (uint8_t*)res;
    }

    void close() {
        assert(fd_ != -1);
        struct stat st {};
        int ok = fstat(fd_, &st);
        if (ok != 0) {
            return;
        }
        munmap(data_, st.st_size);
    }

private:
    int fd_ = -1;
    uint8_t* data_;
};