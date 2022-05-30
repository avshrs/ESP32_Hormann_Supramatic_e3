#pragma once
#include <Vector.h>

struct TX_Buffer{
    Vector<uint8_t> buf;
    int timeout = 0;
};
struct RX_Buffer{
    Vector<uint8_t> buf;
};



