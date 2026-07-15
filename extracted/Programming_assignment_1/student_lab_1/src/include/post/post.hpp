#pragma once

#include <string>
#include <utility>

class Post {
public:
    int id;
    std::string content;
    std::string username;
    int views;

    Post(int id, std::string content, std::string username, int views = 0)
        : id(id), content(std::move(content)), username(std::move(username)), views(views) {}

    std::string toCSV() const;
};
