#include "engagement.hpp"

std::string Engagement::toCSV() const {
    return std::to_string(id) + "," + 
           std::to_string(postId) + "," + 
           username + "," + 
           type + "," + 
           comment + "," + 
           std::to_string(timestamp) + "\n";
}