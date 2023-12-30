#include "br_io.hpp"

#include <fcntl.h>

#include <iostream>
// TT

void create_tmp_file() {
    int fd = open("./hello_world.txt", O_RDWR | O_CREAT);
    chmod("./hello_world.txt", S_IRUSR | S_IWUSR);
    assert(fd != -1);
    write(fd, "hello world", strlen("hello world"));
    close(fd);
}

void read_test() {
    sync_io_mgr mgr;
    int fd = open("./hello_world.txt", O_RDWR);
    assert(fd != -1);
    mgr.open(fd);
    uint8_t hello[1024] = {0};
    uint8_t world[1024] = {0};
    mgr.read(hello, 1024, 0);
    mgr.read(world, 1024, 6);
    std::string_view hello_str ((char*)hello, 5);
    std::string_view world_str ((char*)world, 5);

    mgr.write((uint8_t*)"gigity", strlen("gigity"), 1024);
    mgr.close();
}

void clean_tmp_file() {
    remove("./hello_world.txt");
}


int main(int , char**) {
    create_tmp_file();
    read_test();
    clean_tmp_file();
}