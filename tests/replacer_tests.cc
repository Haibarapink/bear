#include "br_replacer.hpp"
#include "assert.hpp"
#include <vector>

void replacer_test() {
    lru_replacer<int, 5> replacer;
    // 1,2,3,4,5
    for (int i = 1; i <= 5; ++i) {
        assert(replacer.push(i));
    }
    // fail
    assert(!replacer.push(100));

    replacer.touch(2);
    replacer.touch(1);
    // 1 2 5 4 3
    std::vector<int> nums = {1,2,5,4,3};

    for (;!nums.empty();) {
        int back = nums.back();
        assert(back == replacer.pop().value_or(-1));
        nums.pop_back();
    }
}

int main(int, char**) {
    replacer_test();
}
