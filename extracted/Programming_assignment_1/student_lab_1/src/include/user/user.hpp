#pragma once

#include <string>
#include <utility>

class User {
public:
    int id;
    std::string username;
    std::string location;

    User(int id, std::string username, std::string location)
        : id(id), username(std::move(username)), location(std::move(location)) {}

    std::string toCSV() const;
};
