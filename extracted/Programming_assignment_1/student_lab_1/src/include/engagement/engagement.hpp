#pragma once

#include <string>
#include <utility>

class Engagement {
public:
    int id;
    int postId;
    std::string username;
    std::string type;
    std::string comment;
    int timestamp;


    Engagement(int id, 
               int postId, 
               std::string username, 
               std::string type, 
               std::string comment, 
               int timestamp) 
        : id(id), 
          postId(postId), 
          username(std::move(username)), 
          type(std::move(type)), 
          comment(std::move(comment)), 
          timestamp(timestamp) {}

    std::string toCSV() const;
};