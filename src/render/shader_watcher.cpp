#include "shader_watcher.hpp"

#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>

#include <chrono>
#include <cstring>

ShaderWatcher::ShaderWatcher(const std::vector<std::string>& directories,
                             std::function<void(const std::string&)> onChanged)
    : m_directories(directories), m_callback(std::move(onChanged)) {}

ShaderWatcher::~ShaderWatcher() { stop(); }

void ShaderWatcher::start() {
    if (m_running) return;
    m_inotifyFd = inotify_init1(IN_NONBLOCK);
    if (m_inotifyFd < 0) return;

    for (const auto& dir : m_directories) {
        int wd = inotify_add_watch(m_inotifyFd, dir.c_str(),
                                   IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE);
        if (wd >= 0) m_watchDescriptors[wd] = dir;
    }

    m_running = true;
    m_thread = std::thread(&ShaderWatcher::watch, this);
}

void ShaderWatcher::stop() {
    if (!m_running) return;
    m_running = false;
    if (m_thread.joinable()) m_thread.join();
    for (auto& kv : m_watchDescriptors) {
        inotify_rm_watch(m_inotifyFd, kv.first);
    }
    m_watchDescriptors.clear();
    if (m_inotifyFd >= 0) {
        close(m_inotifyFd);
        m_inotifyFd = -1;
    }
}

void ShaderWatcher::watch() {
    constexpr size_t bufSize = sizeof(inotify_event) + NAME_MAX + 1;
    char buffer[bufSize];
    while (m_running) {
        int len = read(m_inotifyFd, buffer, sizeof(buffer));
        if (len <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        int i = 0;
        while (i < len) {
            auto* ev = reinterpret_cast<inotify_event*>(&buffer[i]);
            if (ev->len > 0 && (ev->mask & (IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE))) {
                auto it = m_watchDescriptors.find(ev->wd);
                if (it != m_watchDescriptors.end()) {
                    std::string full = it->second + "/" + ev->name;
                    m_callback(full);
                }
            }
            i += sizeof(inotify_event) + ev->len;
        }
    }
}

