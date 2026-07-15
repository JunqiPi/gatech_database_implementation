// Please do not use any external libraries beyond the ones listed below
// Including any other STD libs not listed below.
#include <algorithm>
#include <atomic>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <random>
#include <regex>
#include <set>
#include <signal.h>
#include <sstream>
#include <string>
#include <string_view>
#include <sys/resource.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../include/engagement/engagement.hpp"
#include "../include/post/post.hpp"
#include "../include/user/user.hpp"

using namespace std;

inline void assert_with_message(bool condition, const std::string &message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

namespace fs = std::filesystem;

namespace {

  string trim(const string &str) {
    size_t begin=0, end=str.size();
    while (begin < end && isspace(str[begin])) begin++;
    while (end > begin && isspace(str[end-1])) end--;
    return str.substr(begin, end - begin);
  }

  bool parseInt(const string &str, int &result) {
    string s = trim(str);
    if (s.empty()) return false;
    try {
      size_t idx;
      result = stoi(s, &idx);
      return idx == s.size();
    } catch (const invalid_argument &) {
      return false;
    } catch (const out_of_range &) {
      return false;
    }    
  }

  // Helper functions to parse CSV files into maps of unique_ptrs
  map<int, unique_ptr<User>> parseUsers(const string &filePath, string &header) {
    map<int, unique_ptr<User>> users;
    ifstream in(filePath, ios::binary);
    assert_with_message(in.is_open(), "Failed to open users CSV file: " + filePath);

    string line;
    if (getline(in, line)) {
      header = line; // Store the header
    } else {
      throw runtime_error("Users CSV file is empty: " + filePath);
    }

    while (getline(in, line)) {
      if (line.empty()) continue; // Skip empty lines
      stringstream ss(line);
      string idStr, username, location;

      if (!getline(ss, idStr, ',') || !getline(ss, username, ',') || !getline(ss, location)) {
        continue; // Skip malformed lines
      }

      int id;
      if (!parseInt(idStr, id)) {
        continue; // Skip lines with invalid ID
      }
      users[id] = make_unique<User>(id, trim(username), trim(location));
    }
    return users;
  }

  // Similar parsing functions for posts and engagements
  map<int, unique_ptr<Post>> parsePosts(const string &filePath, string &header) {
    map<int, unique_ptr<Post>> posts;
    ifstream in(filePath, ios::binary);
    assert_with_message(in.is_open(), "Failed to open posts CSV file: " + filePath);

    string line;
    if (getline(in, line)) {
      header = line; // Store the header
    } else {
      throw runtime_error("Posts CSV file is empty: " + filePath);
    }

    while (getline(in, line)) {
      if (line.empty()) continue; // Skip empty lines
      stringstream ss(line);
      string idStr, content, username, viewsStr;

      if (!getline(ss, idStr, ',') || !getline(ss, content, ',') || !getline(ss, username, ',') || !getline(ss, viewsStr)) {
        continue;
      }

      int id, views;
      if (!parseInt(idStr, id) || !parseInt(viewsStr, views)) {
        continue;
      }
      posts[id] = make_unique<Post>(id, trim(content), trim(username), views);
    }
    return posts;
  }

  // Similar parsing functions for posts and engagements
  map<int, unique_ptr<Engagement>> parseEngagements(const string &filePath, string &header) {
    map<int, unique_ptr<Engagement>> engagements;
    ifstream in(filePath, ios::binary);
    assert_with_message(in.is_open(), "Failed to open engagements CSV file: " + filePath);

    string line;
    if (getline(in, line)) {
      header = line; // Store the header
    } else {
      throw runtime_error("Engagements CSV file is empty: " + filePath);
    }

    while (getline(in, line)) {
      if (line.empty()) continue; // Skip empty lines
      stringstream ss(line);
      string idStr, postIdStr, username, type, comment, timestampStr;

      if (!getline(ss, idStr, ',') || !getline(ss, postIdStr, ',') || !getline(ss, username, ',') || !getline(ss, type, ',') || !getline(ss, comment, ',') || !getline(ss, timestampStr)) {
        continue; // Skip malformed lines
      }

      int id, postId, timestamp;
      if (!parseInt(idStr, id) || !parseInt(postIdStr, postId) || !parseInt(timestampStr, timestamp)) {
        continue; // Skip lines with invalid integers
      }
      engagements[id] = make_unique<Engagement>(id, postId, trim(username), trim(type), trim(comment), timestamp);
    }
    return engagements;
  }
}

class FlatFile {
private:
  std::map<int, std::unique_ptr<User>> users;
  std::map<int, std::unique_ptr<Post>> posts;
  std::map<int, std::unique_ptr<Engagement>> engagements;

  string usersPath, postsPath, engagementsPath;
  string usersHeader = "id,username,location";
  string postsHeader = "id,content,username,views";
  string engagementsHeader = "id,postId,username,type,comment,timestamp";

  map<string, int> usernameToId;
  mutex dbMutex;

  atomic<unsigned long> tempCounter{0};

  // Helper function to commit new data to the shared maps and update headers if provided
  void commit(map<int, unique_ptr<User>> &&newUsers,
              string newUserHeader,
              map<int, unique_ptr<Post>> &&newPosts,
              string newPostHeader,
              map<int, unique_ptr<Engagement>> &&newEngagements,
              string newEngagementHeader) {
    users = move(newUsers);
    posts = move(newPosts);
    engagements = move(newEngagements);
    if (!newUserHeader.empty()) {
      usersHeader = move(newUserHeader);
    }
    if (!newPostHeader.empty()) {
      postsHeader = move(newPostHeader);
    }
    if (!newEngagementHeader.empty()) {
      engagementsHeader = move(newEngagementHeader);
    }

    usernameToId.clear();
    for (const auto &entry : users) {
      usernameToId[entry.second->username] = entry.first;
    }
  }

  // Helper function to write CSV data atomically by writing to a temp file and renaming
  void writeAtomicallyToCSV(const string &filePath, const string &header, const string &body) {
    string temp = filePath + ".tmp." + to_string(getpid()) + "." + to_string(tempCounter++);
    ofstream out(temp, ios::binary | ios::trunc);
    assert_with_message(out.is_open(), "Failed to open temporary file for writing: " + temp);

    out << header << "\n" << body;
    out.flush();

    assert_with_message(out.good(), "Failed to write to temporary file: " + temp);
    out.close();

    fs::rename(temp, filePath);
  }

  // Helper functions to convert current maps to CSV body strings
  string usersToCSVVector() const {
    string body;
    for (const auto &entry : users) {
      body += entry.second->toCSV();
    }

    return body;
  }

  string postsToCSVVector() const {
    string body;
    for (const auto &entry : posts) {
      body += entry.second->toCSV();
    }

    return body;
  }

  string engagementsToCSVVector() const {
    string body;
    for (const auto &entry : engagements) {
      body += entry.second->toCSV();
    }

    return body;
  }

public:
  FlatFile(std::string users_csv_path,
           std::string posts_csv_path,
           std::string engagements_csv_path) {
    usersPath = users_csv_path;
    postsPath = posts_csv_path;
    engagementsPath = engagements_csv_path;
  }

  ~FlatFile() = default;

  /**
   * @brief Single-threaded load of users, posts, and engagements from CSVs.
   *
   * @details
   *  - Reads from the instance paths (users_path, posts_path,
   * engagements_path).
   *  - Skip the first header line; ignore empty/malformed rows rather than
   * throwing.
   *  - Parse into temporary maps, then swap into shared maps under mutexes.
   *  - Ensure referential integrity across tables
   *
   * @thread_safety  Safe to call concurrently; the final commit is serialized
   * by internal mutexes.
   * @throws Aborts via ASSERT_WITH_MESSAGE if a CSV cannot be opened.
   * @complexity  O(U + P + E) over rows read, plus I/O.
   */
  void loadFlatFile() {
    string newUserHeader, newPostHeader, newEngagementHeader;
    auto newUsers = parseUsers(usersPath, newUserHeader);
    auto newPosts = parsePosts(postsPath, newPostHeader);
    auto newEngagements = parseEngagements(engagementsPath, newEngagementHeader);

    scoped_lock lock(dbMutex);
    commit(move(newUsers), newUserHeader, move(newPosts), newPostHeader, move(newEngagements), newEngagementHeader);
  }

  /**
   * @brief Parallel loader for users, posts, and engagements from CSVs.
   *
   * @details
   *  - Spawn one short-lived thread per CSV file; each thread parses its CSV
   * file into local containers.
   *  - Parse into temporary maps, then swap into shared maps under mutexes.
   *
   * @thread_safety Safe to call concurrently; the final commit is serialized by
   * internal mutexes.
   * @throws Aborts via ASSERT_WITH_MESSAGE if any CSV cannot be opened.
   * @complexity  O(U + P + E) total work; wall time reduced by parallel
   * I/O/parse.
   */
  void loadMultipleFlatFilesInParallel() {
    map<int, unique_ptr<User>> newUsers;
    map<int, unique_ptr<Post>> newPosts;
    map<int, unique_ptr<Engagement>> newEngagements;
    string newUserHeader, newPostHeader, newEngagementHeader;

    thread loadUsers([&]() {
      newUsers = parseUsers(usersPath, newUserHeader);
    });
    thread loadPosts([&]() {
      newPosts = parsePosts(postsPath, newPostHeader);
    });
    thread loadEngagements([&]() {
      newEngagements = parseEngagements(engagementsPath, newEngagementHeader);
    });

    loadUsers.join();
    loadPosts.join();
    loadEngagements.join();

    scoped_lock lock(dbMutex);
    commit(move(newUsers), newUserHeader, move(newPosts), newPostHeader, move(newEngagements), newEngagementHeader);
  }

  /** IMPORTANT HINTS ABOUT WRITES AND ATOMICITY:
   *  - Keep header rows for the persisted / permanent / durable part of your
   * FlatFile implementation.
   *  - A new instance of FlatFile should be able to load the durable modified
   * CSV and get back to the same states.
   *  - All durable CSV writes should be atomic (you can accomplish this by
   * creating a unique temporary file when loading in the same directory,
   * copying on write (copy new info into the temp file), and doing an atomic
   * swap for durable CSV rewrites).
   *  - Ensure atomic visibility of updates. To accomplish this, we must acquire
   * locks before updating the global maps so the system always sees a
   * consistent snapshot of users, posts, and engagements. Readers should never
   * see partial updates.
   */

  /**
   * @brief Atomically increment a post’s view count and persist to CSV.
   * @param post_id Target post id.
   * @param views_count Amount to add (may be >1).
   * @return true on success; false if post_id not found.
   * @thread_safety Writers are serialized via internal mutex.
   * @side_effects Rewrites posts CSV with the updated row.
   */
  bool updatePostViews(int post_id,
                       int views_count) {
    scoped_lock lock(dbMutex);
    auto it = posts.find(post_id);
    if (it == posts.end()) return false;
    it->second->views += views_count;

    writeAtomicallyToCSV(postsPath, postsHeader, postsToCSVVector());

    return true;
  }

  /**
   * @brief Append a new engagement and persist to CSV.
   * @param record Engagement to add.
   * @thread_safety Serialized by internal mutexes.
   * @side_effects Appends to engagements CSV; ignores rows failing foreign-key
   * checks, e.g. ensure every engagement.postId exists in posts.
   */
  void addEngagementRecord(Engagement &record) {
    scoped_lock lock(dbMutex);

    if (posts.find(record.postId) == posts.end() || usernameToId.find(record.username) == usernameToId.end()) {
      return;
    }

    ofstream out(engagementsPath, ios::binary | ios::app);
    assert_with_message(out.is_open(), "Failed to open engagements CSV file for appending: " + engagementsPath);
    out << record.toCSV();
    out.flush();
    assert_with_message(out.good(), "Failed to write to engagements CSV file: " + engagementsPath);
    out.close();

    engagements[record.id] = make_unique<Engagement>(record);
  }

  /**
   * @brief All comments by a user, ordered by (post_id, comment).
   * @param user_id User id.
   * @return Vector of <post_id, comment>.
   * @thread_safety Reads are synchronized.
   */
  std::vector<std::pair<int, std::string>>
  getAllUserComments(int user_id) {
    scoped_lock lock(dbMutex);

    auto userIt = users.find(user_id);
    if (userIt == users.end()) {
      return {};
    }
    
    const string &username = userIt->second->username;
    vector<pair<int, string>> comments;
    for (const auto &entry : engagements) {
      const auto &engagement = entry.second;
      if (engagement!= nullptr && 
        engagement->username == username && 
        engagement->type == "comment") 
      {
        comments.push_back({engagement->postId, engagement->comment});
      }
    }


    sort(comments.begin(), comments.end(), [](const auto &a, const auto &b) {
      if (a.first == b.first) {
        return a.second < b.second;
      }
      return a.first < b.first;
    });

    return comments;
  }

  /**
   * @brief Count likes/comments for users in a location.
   * @param location Exact location string.
   * @return <likes_count, comments_count>.
   * @thread_safety Reads are synchronized.
   */
  std::pair<int, int>
  getAllEngagementsByLocation(std::string location) {
    scoped_lock lock(dbMutex);

    set<string> usernamesInLocation;

    for (const auto &entry : users) {
      const auto &user = entry.second;
      if (user != nullptr && user->location == location) {
        usernamesInLocation.insert(user->username);
      }
    }

    int likesCount = 0;
    int commentsCount = 0;

    for (const auto &entry : engagements) {
      const auto &engagement = entry.second;
      if (usernamesInLocation.count(engagement->username) == 0) continue;
      if (engagement->type == "like") {
        likesCount++;
      } else if (engagement->type == "comment") {
        commentsCount++;
      }
    }

    return {likesCount, commentsCount};
  }

  /**
   * @brief Rename a user everywhere and persist to all CSVs.
   * @param user_id Target user id.
   * @param new_username New username.
   * @return true if user exists and all rewrites succeed; otherwise false.
   * @thread_safety Serialized updates; readers see a consistent state after
   * commit.
   * @side_effects Rewrites users, posts, and engagements CSVs.
   */
  bool updateUserName(int user_id,
                      std::string new_username) {
    scoped_lock lock(dbMutex);

    auto userIt = users.find(user_id);
    if (userIt == users.end()) {
      return false;
    }

    string oldUsername = userIt->second->username;
    if (oldUsername == new_username) {
      return true; // No change needed
    }

    userIt->second->username = new_username;
    for (auto &postEntry : posts) {
      auto &post = postEntry.second;
      if (post != nullptr && post->username == oldUsername) {
        post->username = new_username;
      }
    }
    for (auto &engagementEntry : engagements) {
      auto &engagement = engagementEntry.second;
      if (engagement != nullptr && engagement->username == oldUsername) {
        engagement->username = new_username;
      }
    }

    writeAtomicallyToCSV(usersPath, usersHeader, usersToCSVVector());
    writeAtomicallyToCSV(postsPath, postsHeader, postsToCSVVector());
    writeAtomicallyToCSV(engagementsPath, engagementsHeader, engagementsToCSVVector());

    return true;
  }

  // Accessors
  std::map<int, std::unique_ptr<User>> &getUsers() { return users; }
  std::map<int, std::unique_ptr<Post>> &getPosts() { return posts; }
  std::map<int, std::unique_ptr<Engagement>> &getEngagements() {
    return engagements;
  }
};
