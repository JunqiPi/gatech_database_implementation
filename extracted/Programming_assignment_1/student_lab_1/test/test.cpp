#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <dirent.h>
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>

#include "../src/flat_file/flat_file.cpp"

#define ASSERT_WITH_MESSAGE(condition, message)                                \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << "Assertion \033[1;31mFAILED\033[0m: " << message << " at "  \
                << __FILE__ << ":" << __LINE__ << std::endl;                   \
      std::abort();                                                            \
    }                                                                          \
  } while (0)

#ifndef MAIN_DEFINED
#define MAIN_DEFINED

// ---------- Helpers (timing, memory, CPU, copying) ----------

// Strictly Linux thread polling inside Docker
static int get_os_thread_count() {
#if defined(__linux__)
  int count = 0;
  DIR *dp = opendir("/proc/self/task");
  if (dp != nullptr) {
    struct dirent *ep;
    while ((ep = readdir(dp))) {
      if (ep->d_name[0] != '.')
        ++count;
    }
    closedir(dp);
  }
  return count;
#else
  return 1;
#endif
}

static size_t peakRSSkB() {
  struct rusage ru{};
  getrusage(RUSAGE_SELF, &ru);
  // On Linux, ru_maxrss is in kilobytes.
  return static_cast<size_t>(ru.ru_maxrss);
}

static double cpu_user_seconds() {
  struct rusage ru{};
  getrusage(RUSAGE_SELF, &ru);
  return ru.ru_utime.tv_sec + ru.ru_utime.tv_usec / 1e6;
}

static double cpu_sys_seconds() {
  struct rusage ru{};
  getrusage(RUSAGE_SELF, &ru);
  return ru.ru_stime.tv_sec + ru.ru_stime.tv_usec / 1e6;
}

class ScopedTimer {
  std::string label;
  std::chrono::steady_clock::time_point t0;
  double limit_sec; // if > 0, assert wall-time under this
public:
  explicit ScopedTimer(std::string s, double limit = -1.0)
      : label(std::move(s)), t0(std::chrono::steady_clock::now()),
        limit_sec(limit) {}
  ~ScopedTimer() {
    double sec =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - t0)
            .count();
    std::cout << label << ": " << sec << " s\n";
    if (limit_sec > 0) {
      ASSERT_WITH_MESSAGE(sec < limit_sec,
                          label + " too slow: " + std::to_string(sec) +
                              "s (limit " + std::to_string(limit_sec) + "s)");
    }
  }
};

// Function to copy files line by line
void copy_files(const std::vector<std::string> &input_files,
                const std::vector<std::string> &output_files) {
  for (size_t i = 0; i < input_files.size(); i++) {
    std::ifstream src(input_files[i]);
    std::ofstream dst(output_files[i], std::ios::trunc);
    ASSERT_WITH_MESSAGE(src.is_open(),
                        "copy_files: cannot open " + input_files[i]);
    ASSERT_WITH_MESSAGE(dst.is_open(),
                        "copy_files: cannot open " + output_files[i]);
    std::string line;
    while (std::getline(src, line)) {
      dst << line << "\n";
    }
  }
}

// Overwrite the 'views' column for a specific post_id in a CSV file (header
// preserved).
static void rewrite_post_views_file(const std::string &posts_csv_path,
                                    int post_id, int new_views) {
  std::ifstream in(posts_csv_path);
  ASSERT_WITH_MESSAGE(in.good(), "Cannot open " + posts_csv_path);
  std::string tmp = posts_csv_path + ".tmp";
  std::ofstream out(tmp);
  ASSERT_WITH_MESSAGE(out.good(), "Cannot open tmp for " + posts_csv_path);

  std::string line;
  bool header = true;
  while (std::getline(in, line)) {
    if (header) {
      out << line << "\n";
      header = false;
      continue;
    }
    std::stringstream ss(line);
    std::string id_str, content, username, views_str;
    std::getline(ss, id_str, ',');
    std::getline(ss, content, ',');
    std::getline(ss, username, ',');
    std::getline(ss, views_str, ',');
    if (!id_str.empty() && std::stoi(id_str) == post_id) {
      out << id_str << "," << content << "," << username << "," << new_views
          << "\n";
    } else {
      out << line << "\n";
    }
  }
  in.close();
  out.close();
  std::remove(posts_csv_path.c_str());
  int rc = std::rename(tmp.c_str(), posts_csv_path.c_str());
  ASSERT_WITH_MESSAGE(rc == 0, "rename failed for " + posts_csv_path);
}

// Quick referential-integrity sweep
// ensure every engagement.postId exists in posts.
static bool check_no_dangling_post_ids(
    const std::map<int, std::unique_ptr<Engagement>> &eng,
    const std::map<int, std::unique_ptr<Post>> &posts) {
  for (const auto &kv : eng) {
    int pid = kv.second->postId;
    if (posts.find(pid) == posts.end())
      return false;
  }
  return true;
}

int main(int argc, char *argv[]) {
  bool execute_all = false;
  std::string selected_test = "-1";
  int seed = std::chrono::system_clock::now().time_since_epoch().count();

  if (argc < 2) {
    execute_all = true;
  } else {
    selected_test = argv[1];
  }

  // duplicate copies of test input files to avoid modifying originals
  std::vector<std::string> input_files = {
      "../data/users.csv",
      "../data/posts.csv",
      "../data/engagements.csv",
  };
  std::vector<std::string> output_files = {
      "../data/users_copy.csv",
      "../data/posts_copy.csv",
      "../data/engagements_copy.csv",
  };
  srand(seed);

  // Test 1: LoadCSV [Single threaded]
  if (execute_all || selected_test == "1") {
    std::cout << "Executing Test 1: Single threaded loadFlatFile" << std::endl;

    auto start_time = std::chrono::steady_clock::now();
    FlatFile flatFile(input_files[0], input_files[1], input_files[2]);
    flatFile.loadFlatFile();
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_seconds = end_time - start_time;
    std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

    std::set<std::string> usernames, post_usernames;
    std::set<int> engagement_post_ids, post_ids;
    for (auto &entry : flatFile.getUsers()) {
      usernames.insert(entry.second->username);
    }
    for (auto &entry : flatFile.getPosts()) {
      post_ids.insert(entry.second->id);
      post_usernames.insert(entry.second->username);
    }
    for (auto &entry : flatFile.getEngagements()) {
      engagement_post_ids.insert(entry.second->postId);
    }

    ASSERT_WITH_MESSAGE(usernames.size() == 10000,
                        "users size mismatch. Expected: 10000 Actual: " +
                            std::to_string(usernames.size()));
    ASSERT_WITH_MESSAGE(post_usernames.size() == 4000,
                        "posts users size mismatch. Expected: 4000 Actual: " +
                            std::to_string(post_usernames.size()));
    ASSERT_WITH_MESSAGE(flatFile.getEngagements().size() == 10000,
                        "engagements size mismatch. Expected: 10000 Actual: " +
                            std::to_string(flatFile.getEngagements().size()));

    ASSERT_WITH_MESSAGE(usernames.find("knorman") != usernames.end(),
                        "user knorman not found in flatfile DB");
    ASSERT_WITH_MESSAGE(usernames.find("wchaney") != usernames.end(),
                        "user wchaney not found in flatfile DB");
    ASSERT_WITH_MESSAGE(usernames.find("richardbishop") != usernames.end(),
                        "user richardbishop not found in flatfile DB");

    ASSERT_WITH_MESSAGE(engagement_post_ids.find(1496) !=
                            engagement_post_ids.end(),
                        "post 1496 missing");
    ASSERT_WITH_MESSAGE(engagement_post_ids.find(6936) !=
                            engagement_post_ids.end(),
                        "post 6936 missing");
    ASSERT_WITH_MESSAGE(engagement_post_ids.find(4826) !=
                            engagement_post_ids.end(),
                        "post 4826 missing");

    std::vector<std::string> username_intersection;
    std::vector<int> post_ids_intersection;
    username_intersection.reserve(
        std::min(usernames.size(), post_usernames.size()));
    post_ids_intersection.reserve(
        std::min(post_ids.size(), engagement_post_ids.size()));

    set_intersection(usernames.begin(), usernames.end(), post_usernames.begin(),
                     post_usernames.end(),
                     back_inserter(username_intersection));

    set_intersection(post_ids.begin(), post_ids.end(),
                     engagement_post_ids.begin(), engagement_post_ids.end(),
                     back_inserter(post_ids_intersection));

    ASSERT_WITH_MESSAGE(username_intersection.size() == post_usernames.size(),
                        "username mismatch in posts and users.");
    ASSERT_WITH_MESSAGE(post_ids_intersection.size() ==
                            engagement_post_ids.size(),
                        "post ids mismatch in posts and engagements.");

    std::cout << "Test 1: PASSED" << std::endl;
  }

  // Test 2: Parallel loader must spawn multiple threads
  if (execute_all || selected_test == "2") {
    std::cout << "Executing Test 2: Parallel loader speedup vs serial\n";
    copy_files(input_files, output_files);

    std::atomic<int> peak_threads_seen{0};
    std::atomic<bool> monitor_active{true};

    // 1. Boot up an OS Concurrency Monitor
    std::thread monitor([&]() {
      while (monitor_active.load()) {
        int current_count = get_os_thread_count();
        int peak = peak_threads_seen.load();

        while (current_count > peak &&
               !peak_threads_seen.compare_exchange_weak(peak, current_count))
          ;

        std::this_thread::sleep_for(std::chrono::microseconds(500));
      }
    });

    // 2. Execute Student Code
    int base_threads = get_os_thread_count(); // Should be ~2 (Main + Monitor)
    FlatFile b(output_files[0], output_files[1], output_files[2]);
    b.loadMultipleFlatFilesInParallel(); // Student should spawn ~3 threads!

    // 3. Close Monitor safely
    monitor_active.store(false);
    monitor.join();

    // 4. Assert Structural Compliance
    ASSERT_WITH_MESSAGE(peak_threads_seen.load() > base_threads,
                        "No multi-threading detected! You must spawn "
                        "std::threads to load data in parallel.");

    // Final equivalence check: both paths load the same cardinalities.
    {
      FlatFile a(output_files[0], output_files[1], output_files[2]);
      a.loadFlatFile();
      ASSERT_WITH_MESSAGE(a.getUsers().size() == b.getUsers().size(),
                          "users size mismatch between serial and parallel");
      ASSERT_WITH_MESSAGE(a.getPosts().size() == b.getPosts().size(),
                          "posts size mismatch between serial and parallel");
      ASSERT_WITH_MESSAGE(
          a.getEngagements().size() == b.getEngagements().size(),
          "engagements size mismatch between serial and parallel");
    }

    std::cout << "Test 2: PASSED\n";
  }

  // Test 3: GetAllUserComments
  if (execute_all || selected_test == "3") {
    std::cout << "Executing Test 3: GetAllUserComments" << std::endl;

    // invalid user id
    {
      FlatFile flatFile(input_files[0], input_files[1], input_files[2]);
      flatFile.loadFlatFile();

      int user_id = 10001 + (std::rand() % 1000);
      auto found_comments = flatFile.getAllUserComments(user_id);
      ASSERT_WITH_MESSAGE(found_comments.empty(),
                          "Invalid user id should return empty comments");
    }

    // valid user id with added comments and ordering
    {
      std::vector<std::pair<int, std::string>> expected_comments;
      int user_id = -1;

      copy_files(input_files, output_files);

      {
        FlatFile flatFile(output_files[0], output_files[1], output_files[2]);
        flatFile.loadFlatFile();

        // find a valid user id with comments
        std::map<std::string, int> user_names;
        std::vector<int> user_ids;
        for (auto &entry : flatFile.getEngagements()) {
          if (entry.second->type == "comment")
            user_names[entry.second->username]++;
        }
        for (auto &entry : flatFile.getUsers()) {
          if (user_names[entry.second->username] > 1)
            user_ids.push_back(entry.second->id);
        }

        user_id = user_ids[std::rand() % user_ids.size()];
        std::cout << "Testing for a valid user_id: " << user_id << std::endl;

        expected_comments = flatFile.getAllUserComments(user_id);

        // add two comments to test ordering ties
        Engagement record1 = Engagement(100010, expected_comments[0].first + 1,
                                        flatFile.getUsers()[user_id]->username,
                                        "comment", "comment1", 100);
        flatFile.addEngagementRecord(record1);
        expected_comments.push_back({record1.postId, record1.comment});

        Engagement record2 = Engagement(100011, expected_comments[0].first,
                                        flatFile.getUsers()[user_id]->username,
                                        "comment", "comment2", 101);
        flatFile.addEngagementRecord(record2);
        expected_comments.push_back({record2.postId, record2.comment});

        std::sort(expected_comments.begin(), expected_comments.end());
      }

      {
        FlatFile flatFile(output_files[0], output_files[1], output_files[2]);
        flatFile.loadFlatFile();

        auto found_comments = flatFile.getAllUserComments(user_id);
        ASSERT_WITH_MESSAGE(
            found_comments == expected_comments,
            "Expected and actual comments mismatch (size/order must match)");
      }
    }

    std::cout << "Test 3: PASSED" << std::endl;
  }

  // Test 4: GetAllEngagementsByLocation
  if (execute_all || selected_test == "4") {
    std::cout << "Executing Test 4: GetAllEngagementsByLocation" << std::endl;

    std::string location = "InvalidLocation";
    std::pair<int, int> expected_engagements;
    {
      FlatFile flatFile(input_files[0], input_files[1], input_files[2]);
      flatFile.loadFlatFile();
      expected_engagements = flatFile.getAllEngagementsByLocation(location);
      ASSERT_WITH_MESSAGE(expected_engagements.first == 0 &&
                              expected_engagements.second == 0,
                          "Invalid location should return 0 engagements");
    }

    {
      copy_files(input_files, output_files);

      FlatFile flatFile(output_files[0], output_files[1], output_files[2]);
      flatFile.loadFlatFile();

      std::vector<std::string> locations;
      std::string username = "";

      for (auto &entry : flatFile.getUsers()) {
        locations.push_back(entry.second->location);
      }

      location = locations[std::rand() % locations.size()];
      for (auto &entry : flatFile.getUsers()) {
        if (entry.second->location == location) {
          username = entry.second->username;
          break;
        }
      }
      std::cout << "Testing for a valid location: " << location << std::endl;
      expected_engagements = flatFile.getAllEngagementsByLocation(location);

      for (int i = 0; i < rand() % 20 + 1; i++) {
        int type = rand() % 2;
        Engagement record =
            Engagement(100010 + i, 1, username, type ? "like" : "comment",
                       type ? "None" : "Howdy!", 100);
        flatFile.addEngagementRecord(record);
        if (type)
          ++expected_engagements.first;
        else
          ++expected_engagements.second;
      }

      auto actual_engagements = flatFile.getAllEngagementsByLocation(location);
      ASSERT_WITH_MESSAGE(actual_engagements == expected_engagements,
                          "Expected vs actual engagements mismatch");
    }

    {
      FlatFile flatFile(output_files[0], output_files[1], output_files[2]);
      flatFile.loadFlatFile();
      auto actual_engagements = flatFile.getAllEngagementsByLocation(location);
      ASSERT_WITH_MESSAGE(actual_engagements == expected_engagements,
                          "Mismatch after reload");
    }

    std::cout << "Test 4: PASSED" << std::endl;
  }

  // Test 5: UpdateUserName
  if (execute_all || selected_test == "5") {
    std::cout << "Executing Test 5: UpdateUserName" << std::endl;

    {
      FlatFile flatFile(input_files[0], input_files[1], input_files[2]);
      flatFile.loadFlatFile();
      int user_id = 100001 + (std::rand() % 1000);
      bool status = flatFile.updateUserName(user_id, "new_username");
      ASSERT_WITH_MESSAGE(!status, "Invalid user id should return false");
    }

    int user_id = -1;
    int prev_posts_count = 0, prev_engagements_count = 0;
    std::string old_username;
    std::string new_username = "new_username";
    {
      copy_files(input_files, output_files);

      FlatFile flatFile(output_files[0], output_files[1], output_files[2]);
      flatFile.loadFlatFile();

      std::map<std::string, int> posts_count, engagements_count;
      for (auto &entry : flatFile.getPosts())
        posts_count[entry.second->username]++;
      for (auto &entry : flatFile.getEngagements())
        engagements_count[entry.second->username]++;

      std::vector<int> user_ids;
      for (auto &entry : flatFile.getUsers()) {
        if (posts_count[entry.second->username] >= 1 &&
            engagements_count[entry.second->username] >= 1) {
          user_ids.push_back(entry.second->id);
        }
      }
      user_id = user_ids[std::rand() % user_ids.size()];
      old_username = flatFile.getUsers()[user_id]->username;

      std::cout << "Testing for valid user_id: " << user_id << std::endl;
      prev_posts_count = posts_count[old_username];
      prev_engagements_count = engagements_count[old_username];

      bool status = flatFile.updateUserName(user_id, new_username);
      ASSERT_WITH_MESSAGE(status, "Valid user id should return true");
    }

    {
      FlatFile flatFile(output_files[0], output_files[1], output_files[2]);
      flatFile.loadFlatFile();

      ASSERT_WITH_MESSAGE(flatFile.getUsers()[user_id]->username ==
                              new_username,
                          "Username not updated in users.csv");

      int new_posts_count = 0, new_engagements_count = 0;
      for (auto &entry : flatFile.getPosts()) {
        auto uname = entry.second->username;
        ASSERT_WITH_MESSAGE(uname != old_username,
                            "Old username still exists in posts.");
        if (uname == new_username)
          ++new_posts_count;
      }
      for (auto &entry : flatFile.getEngagements()) {
        auto uname = entry.second->username;
        ASSERT_WITH_MESSAGE(uname != old_username,
                            "Old username still exists in engagements.");
        if (uname == new_username)
          ++new_engagements_count;
      }

      ASSERT_WITH_MESSAGE(prev_posts_count == new_posts_count,
                          "Posts count changed after username update.");
      ASSERT_WITH_MESSAGE(prev_engagements_count == new_engagements_count,
                          "Engagements count changed after username update.");
    }

    std::cout << "Test 5: PASSED" << std::endl;
  }

  // Test 6: In-place rewrite must survive line length increases
  if (execute_all || selected_test == "6") {
    std::cout << "Executing Test 6: Line-growth safety\n";
    copy_files(input_files, output_files);

    const int target_post = 19; // reuse a known-good post id
    // Force starting views to a 2-digit value
    // so increments cross into 3+ digits
    rewrite_post_views_file(output_files[1], target_post, 95);

    // Now  enough to cross 100 (and beyond)
    {
      FlatFile ff(output_files[0], output_files[1], output_files[2]);
      ff.loadFlatFile();
      int start = ff.getPosts()[target_post]->views;
      int total_add = 0;
      for (int k = 0; k < 10; ++k) {
        // +1 ten times; 95 -> 105
        ff.updatePostViews(target_post, 1);
        total_add += 1;
      }
      // Reload from disk to detect CSV corruption
      FlatFile ff2(output_files[0], output_files[1], output_files[2]);
      ff2.loadFlatFile();
      int got = ff2.getPosts()[target_post]->views;
      ASSERT_WITH_MESSAGE(got == start + total_add,
                          "Digit-growth rewrite corrupted file. expected " +
                              std::to_string(start + total_add) + " got " +
                              std::to_string(got));
    }
    std::cout << "Test 6: PASSED\n";
  }

  // Test 7: UpdatePostViews_ValidPostId (concurrent)
  if (execute_all || selected_test == "7") {
    std::cout << "Executing Test 7: [ATOMICITY] UpdatePostViews_ValidPostId"
              << std::endl;

    // parameters used below; change here if you tweak the test
    const int kThreads = 10;
    const int kUpdatesPerThread = 10; // j = 0..9, adds (j+1)

    // helper to compute total  robustly
    auto per_thread_inc = []() {
      int s = 0;
      for (int j = 0; j < kUpdatesPerThread; ++j)
        s += (j + 1);
      return s; // 55 with current settings
    };
    const int total_increment = kThreads * per_thread_inc();

    int post_id_to_update = 19;
    int initial_views = 0;
    {
      copy_files(input_files, output_files);

      FlatFile flatFile(output_files[0], output_files[1], output_files[2]);
      flatFile.loadFlatFile();

      auto &posts = flatFile.getPosts();
      ASSERT_WITH_MESSAGE(posts.count(post_id_to_update),
                          "Test setup error: post_id not found");
      initial_views = posts[post_id_to_update]->views;

      // Clear, direct assertion on the initial value
      ASSERT_WITH_MESSAGE(
          initial_views >= 0,
          "Post should not start with a negative number of views");

      std::vector<std::thread> threads;
      threads.reserve(kThreads);
      for (int i = 0; i < kThreads; ++i) {
        threads.emplace_back([&flatFile, post_id_to_update] {
          for (int j = 0; j < kUpdatesPerThread; ++j) {
            bool ok = flatFile.updatePostViews(post_id_to_update, j + 1);
            ASSERT_WITH_MESSAGE(ok, "updatePostViews failed");
          }
        });
      }
      for (auto &t : threads)
        t.join();
    }

    // Re-open to verify persisted value == initial + total_increment
    {
      FlatFile flatFile(output_files[0], output_files[1], output_files[2]);
      flatFile.loadFlatFile();
      int final_views = flatFile.getPosts()[post_id_to_update]->views;

      ASSERT_WITH_MESSAGE(final_views == initial_views + total_increment,
                          "Post views not updated correctly. Expected " +
                              std::to_string(initial_views + total_increment) +
                              " got " + std::to_string(final_views));
    }
    std::cout << "Test 7: PASSED" << std::endl;
  }

  // Test 8: UpdatePostViews_InvalidPostId
  if (execute_all || selected_test == "8") {
    std::cout << "Executing Test 8: UpdatePostViews_InvalidPostId" << std::endl;

    try {
      FlatFile flatFile(input_files[0], input_files[1], input_files[2]);
      flatFile.loadFlatFile();

      ASSERT_WITH_MESSAGE(!flatFile.getPosts().empty(), "Precondition failed: Database posts were not loaded");

      bool status = flatFile.updatePostViews(100001 + (std::rand() % 1000), 1);
      ASSERT_WITH_MESSAGE(!status, "Invalid post id should return false");
    } catch (const std::exception &e) {
      std::cout << "Exception occurred: " << e.what() << std::endl;
      throw;
    }

    std::cout << "Test 8: PASSED" << std::endl;
  }

  // Test 9: Resource ceilings (wall time, CPU, memory) during load
  if (execute_all || selected_test == "9") {
    std::cout << "Executing Test 9: Performance/resource guardrails\n";

    auto kb_to_mb = [](size_t kb) { return static_cast<double>(kb) / 1024.0; };
    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(2);

    size_t before_kb = peakRSSkB();
    double u0 = cpu_user_seconds();
    double s0 = cpu_sys_seconds();

    {
      // ScopedTimer prints wall time unconditionally on destruction.
      ScopedTimer timer("Full loadFlatFile wall", 30.0); // 30s wall limit
      FlatFile flatFile(input_files[0], input_files[1], input_files[2]);
      flatFile.loadFlatFile();
      ASSERT_WITH_MESSAGE(!flatFile.getPosts().empty(), "Precondition failed: No records loaded to profile");
    }

    size_t after_kb = peakRSSkB();
    double u1 = cpu_user_seconds();
    double s1 = cpu_sys_seconds();

    double user_cpu = u1 - u0;
    double sys_cpu = s1 - s0;
    size_t delta_kb = (after_kb > before_kb) ? (after_kb - before_kb) : 0;

    // Always print a resource report, even if assertions will pass.
    std::cout << "  CPU (user): " << user_cpu << " s\n"
              << "  CPU (sys) : " << sys_cpu << " s\n"
              << "  Peak Memory Usage  : +" << kb_to_mb(delta_kb) << " MB"
              << "  (from " << kb_to_mb(before_kb) << " MB to "
              << kb_to_mb(after_kb) << " MB)\n";

    // Keep the guardrails
    ASSERT_WITH_MESSAGE(user_cpu < 2, "Excessive user CPU time: " +
                                          std::to_string(user_cpu) + "s");
    ASSERT_WITH_MESSAGE(sys_cpu < 1, "Excessive sys CPU time: " +
                                         std::to_string(sys_cpu) + "s");
    ASSERT_WITH_MESSAGE(delta_kb < 25 * 1024,
                        "Peak RSS increased by > 25 MB: " +
                            std::to_string(delta_kb / 1024) + " MB");

    std::cout << "Test 9: PASSED\n";
  }

  // Test 10: Reader/Writer concurrency sanity
  if (execute_all || selected_test == "10") {
    std::cout
        << "Executing Test 10: [ISOLATION] Reader/Writer concurrency sanity"
        << std::endl;

    copy_files(input_files, output_files);
    FlatFile ff(output_files[0], output_files[1], output_files[2]);
    ff.loadFlatFile();

    const int target_post = 19;
    const int writer_iters = 200;

    // Capture baseline so we assert monotonic persistence.
    ASSERT_WITH_MESSAGE(ff.getPosts().count(target_post),
                        "Target post not found");
    int base_views = ff.getPosts().at(target_post)->views;

    std::atomic<bool> writer_done{false};

    std::thread writer([&]() {
      for (int i = 0; i < writer_iters; ++i) {
        bool ok = ff.updatePostViews(target_post, 1);
        ASSERT_WITH_MESSAGE(ok, "Writer failed to ");
      }
      writer_done.store(true, std::memory_order_release);
    });

    std::vector<std::thread> readers;
    for (int t = 0; t < 8; ++t) {
      readers.emplace_back([&]() {
        while (!writer_done.load(std::memory_order_acquire)) {
          auto p =
              ff.getAllEngagementsByLocation("NowhereLikely"); // cheap read
          ASSERT_WITH_MESSAGE(p.first >= 0 && p.second >= 0,
                              "Negative counts?");
          std::this_thread::yield(); // keep CPU friendly
        }
      });
    }

    writer.join();
    for (auto &r : readers)
      r.join();

    FlatFile ff2(output_files[0], output_files[1], output_files[2]);
    ff2.loadFlatFile();
    ASSERT_WITH_MESSAGE(ff2.getPosts().count(target_post),
                        "Missing target post after writer");
    int after = ff2.getPosts().at(target_post)->views;

    ASSERT_WITH_MESSAGE(after >= base_views + writer_iters,
                        "Writer s not persisted: expected at least " +
                            std::to_string(base_views + writer_iters) +
                            " got " + std::to_string(after));

    std::cout << "Test 10: PASSED\n";
  }

  // Test 11: Crash-during-write durability (best-effort)
  if (execute_all || selected_test == "11") {
    std::cout
        << "Executing Test 11: [DURABILITY] Crash-during-write durability\n";
#if defined(__linux__)
    copy_files(input_files, output_files);
    const int pid_target = 19;

    // Put views near a boundary to encourage wider lines during write
    rewrite_post_views_file(output_files[1], pid_target, 99);

    pid_t child = fork();
    ASSERT_WITH_MESSAGE(child >= 0, "fork failed");

    if (child == 0) {
      // CHILD: loop writes for a while
      FlatFile ff(output_files[0], output_files[1], output_files[2]);
      ff.loadFlatFile();
      for (int i = 0; i < 1000000; ++i) {
        ff.updatePostViews(pid_target, 1);
        if ((i % 1000) == 0) { /* give parent time window */
          usleep(1000);
        }
      }
      _exit(0);
    } else {
      // PARENT: kill child mid-flight
      usleep(50000); // 50ms
      kill(child, SIGKILL);
      usleep(20000); // settle

      // Now try to reload; file must be parseable and value not garbage
      FlatFile ff2(output_files[0], output_files[1], output_files[2]);
      try {
        ff2.loadFlatFile();
      } catch (...) {
        ASSERT_WITH_MESSAGE(false, "CSV became unparseable after crash");
      }
      auto it = ff2.getPosts().find(pid_target);
      ASSERT_WITH_MESSAGE(it != ff2.getPosts().end(),
                          "Target post missing after crash");
      int v = it->second->views;
      ASSERT_WITH_MESSAGE(v >= 99, "Views regressed to below starting value");
      // NOTE: We cannot assert exact value due to kill timing; we only assert
      // it isn't torn/unreadable.
    }
    std::cout << "Test 11: PASSED\n";
#else
    std::cout << "Skipping Test 11 on non-Linux platform.\n";
#endif
  }

  // Test 12: Referential integrity (no dangling engagement.postId)
  if (execute_all || selected_test == "12") {
    std::cout << "Executing Test 12: [CONSISTENCY] Referential integrity\n";
    copy_files(input_files, output_files);

    // Load, then append an invalid engagement that references a non-existent
    // postId.
    {
      FlatFile ff(output_files[0], output_files[1], output_files[2]);
      ff.loadFlatFile();

      // Find a username to reuse
      std::string uname = ff.getUsers().begin()->second->username;
      int nonexistent_post = 99999999;
      Engagement bad(100000001, nonexistent_post, uname, "comment", "bad-fk",
                     123);
      ff.addEngagementRecord(bad);
    }

    // Reload and run an RI sweep: this must fail in a "bad" implementation.
    {
      FlatFile ff(output_files[0], output_files[1], output_files[2]);
      ff.loadFlatFile();
      bool ok = check_no_dangling_post_ids(ff.getEngagements(), ff.getPosts());
      ASSERT_WITH_MESSAGE(ok, "Detected dangling engagement.postId; "
                              "implementation should prevent or clean this");
    }
    std::cout << "Test 12: PASSED\n";
  }

  // Test 13: No type corruption — stoi over numeric fields should never throw
  if (execute_all || selected_test == "13") {
    std::cout << "Executing Test 13: [CONSISTENCY] Type sanity for numerics"
              << std::endl;

    {
      FlatFile flatFile(input_files[0], input_files[1], input_files[2]);
      flatFile.loadFlatFile();
      ASSERT_WITH_MESSAGE(!flatFile.getUsers().empty() && !flatFile.getPosts().empty(),
                          "Precondition failed: Database must be loaded to verify parsed type sanity");
    }

    auto check_numeric_columns = [](const std::string &path,
                                    int numeric_cols_mask) {
      std::ifstream f(path);
      ASSERT_WITH_MESSAGE(f.is_open(), "Cannot open " + path);
      std::string line;
      bool header = true;
      while (std::getline(f, line)) {
        if (header) {
          header = false;
          continue;
        }
        if (line.empty())
          continue;
        std::stringstream ss(line);
        std::vector<std::string> cols;
        std::string cell;
        while (std::getline(ss, cell, ','))
          cols.push_back(cell);
        for (size_t i = 0; i < cols.size(); ++i) {
          if (numeric_cols_mask & (1 << i)) {
            try {
              (void)std::stoi(cols[i]);
            } catch (...) {
              ASSERT_WITH_MESSAGE(
                  false, "Non-numeric where numeric required in " + path);
            }
          }
        }
      }
    };

    // users: id (0) numeric
    check_numeric_columns(input_files[0], (1 << 0));
    // posts: id (0), views (3) numeric
    check_numeric_columns(input_files[1], (1 << 0) | (1 << 3));
    // engagements: id (0), postId (1), timestamp (5) numeric
    check_numeric_columns(input_files[2], (1 << 0) | (1 << 1) | (1 << 5));

    std::cout << "Test 13: PASSED\n";
  }

  // Cleanup copies created by tests
  for (auto &file : output_files) {
    std::remove(file.c_str());
    std::string tmp = file + ".tmp";
    std::remove(tmp.c_str());
  }
}
#endif