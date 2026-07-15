#include "post.hpp"

std::string Post::toCSV() const {
    return std::to_string(id) + "," + content + "," + username + "," + std::to_string(views) + "\n";
}