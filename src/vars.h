#pragma once

struct TX_Buffer{
    uint8_t buf[10];
    size_t size;
    int timeout = 0;
};
struct RX_Buffer{
    uint8_t buf[10];
    size_t size;
};



