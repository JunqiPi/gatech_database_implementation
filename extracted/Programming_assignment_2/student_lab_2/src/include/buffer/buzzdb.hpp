#pragma once

#include "buffer/buffer_manager.hpp"
#include "buffer/exceptions.hpp"

class BuzzDB {
public:
    BufferManager buffer_manager;
    BuzzDB() : buffer_manager() {}
};
