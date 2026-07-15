#pragma once

#include <exception>

class method_not_implemented_exception final : public std::exception {
public:
    const char* what() const noexcept override { return "Method not implemented!"; }
};

// =============================================================================
// buffer_full_error Exception - DO NOT MODIFY
// =============================================================================

class buffer_full_error : public std::exception {
public:
    const char* what() const noexcept override { return "buffer is full"; }
};
