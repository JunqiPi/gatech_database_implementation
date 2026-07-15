#include "user.hpp"

std::string User::toCSV() const {
    return std::to_string(id) + "," + username + "," + location + "\n";
}