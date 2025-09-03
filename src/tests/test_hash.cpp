#include <test_hash.h>

void Tester::test_hash_function() {
    MazeRenderConfig config ={
        false,
        0,
        5,
        5
    };
    assert(config.hash() == config.hash());
    MazeRenderConfig config2 = {
        false,
        0,
        5,
        5
    };
    assert(config.hash() == config2.hash());
    MazeRenderConfig config3 = {
        true,
        3,
        10,
        2
    };
    assert(config.hash() != config3.hash());
}

