#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <atomic>

// Simple Linux inotify based directory watcher used to monitor shader sources.
class ShaderWatcher {
public:
    ShaderWatcher(const std::vector<std::string>& directories,
                  std::function<void(const std::string&)> onChanged);
    ~ShaderWatcher();

    void start();
    void stop();

private:
    void watch();

    int m_inotifyFd = -1;
    std::vector<std::string> m_directories;
    std::unordered_map<int, std::string> m_watchDescriptors;
    std::function<void(const std::string&)> m_callback;
    std::thread m_thread;
    std::atomic<bool> m_running{false};
};

