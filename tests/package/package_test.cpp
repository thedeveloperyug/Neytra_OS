// package_test.cpp - GTest suite for Repository, Downloader, Installer, PackageManager.
// Spins up a tiny in-process HTTP server (a forked child using raw sockets) so the
// test exercises the real HTTP client and real `tar` extraction end to end, without
// depending on external network access.
#include "Downloader.hpp"
#include "Installer.hpp"
#include "PackageManager.hpp"
#include "ProcessManager.hpp"
#include "Repository.hpp"
#include "SocketManager.hpp"
#include "Logger.hpp"

#include <gtest/gtest.h>

#include <arpa/inet.h>
#include <csignal>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

// Reads a whole file into a string (used to load the tar archive we build as a fixture).
std::string readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// Minimal one-shot HTTP/1.1 server: accepts a single connection, ignores the
// request, and replies with a 200 OK containing `body`.
void runFakeHttpServer(int listenFd, const std::string& body) {
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);
    const int connFd = accept(listenFd, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
    if (connFd >= 0) {
        char discard[4096];
        read(connFd, discard, sizeof(discard));  // don't care about the request itself

        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n"
                  << "Content-Length: " << body.size() << "\r\n"
                  << "Connection: close\r\n\r\n"
                  << body;
        const std::string responseText = response.str();
        write(connFd, responseText.data(), responseText.size());
        close(connFd);
    }
    close(listenFd);
    _exit(0);
}

}  // namespace

class PackageManagerTest : public ::testing::Test {
protected:
    void SetUp() override {
        removeArtifacts();  // clean slate in case a previous run crashed mid-test

        mkdir(workDir_.c_str(), 0755);
        {
            std::ofstream f(workDir_ + "/hello.txt", std::ios::trunc);
            f << "hello";
        }
        const ProcessId tarPid = processes_.spawn("tar", {"-cf", archivePath_, "-C", workDir_, "hello.txt"});
        int tarExit = -1;
        ASSERT_GE(tarPid, 0);
        ASSERT_TRUE(processes_.wait(tarPid, tarExit));
        ASSERT_EQ(tarExit, 0) << "failed to build fixture tar archive";
        const std::string archiveBytes = readFile(archivePath_);

        // --- Start the fake HTTP server on an ephemeral port. ---
        const int listenFd = socket(AF_INET, SOCK_STREAM, 0);
        const int reuse = 1;
        setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
        sockaddr_in serverAddr{};
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        serverAddr.sin_port = 0;  // let the kernel choose a free port
        bind(listenFd, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
        socklen_t addrLen = sizeof(serverAddr);
        getsockname(listenFd, reinterpret_cast<sockaddr*>(&serverAddr), &addrLen);
        port_ = ntohs(serverAddr.sin_port);
        listen(listenFd, 1);

        serverPid_ = fork();
        ASSERT_GE(serverPid_, 0) << "fork failed";
        if (serverPid_ == 0) {
            runFakeHttpServer(listenFd, archiveBytes);
            _exit(0);  // unreachable
        }
        close(listenFd);  // parent doesn't need its copy

        {
            std::ofstream index(indexPath_, std::ios::trunc);
            index << "testpkg|1.0|http://127.0.0.1:" << port_ << "/testpkg.tar\n";
        }
        ASSERT_TRUE(repository_.loadFromFile(indexPath_));
    }

    void TearDown() override {
        if (serverPid_ > 0) {
            kill(serverPid_, SIGTERM);  // in case an earlier failure left it blocked in accept()
            int status = 0;
            waitpid(serverPid_, &status, 0);
        }
        removeArtifacts();
    }

    void removeArtifacts() {
        std::error_code ec;
        std::filesystem::remove_all(workDir_, ec);
        std::filesystem::remove(archivePath_, ec);
        std::filesystem::remove(indexPath_, ec);
        std::filesystem::remove_all(installRoot_, ec);
    }

    Logger& log_ = Logger::instance();
    ProcessManager processes_{log_};
    SocketManager sockets_{log_};
    Downloader downloader_{sockets_, log_};
    Installer installer_{processes_, log_};
    Repository repository_{log_};
    const std::string workDir_ = "/tmp/neytra_package_test_src";
    const std::string archivePath_ = "/tmp/neytra_package_test.tar";
    const std::string indexPath_ = "/tmp/neytra_package_test_index";
    const std::string installRoot_ = "/tmp/neytra_package_test_install";
    PackageManager packages_{repository_, downloader_, installer_, log_, installRoot_};
    pid_t serverPid_ = -1;
    int port_ = -1;
};

TEST_F(PackageManagerTest, InstallsAndRemovesPackageEndToEnd) {
    ASSERT_TRUE(repository_.find("testpkg").has_value());

    ASSERT_TRUE(packages_.install("testpkg"));
    EXPECT_TRUE(packages_.isInstalled("testpkg"));
    EXPECT_EQ(readFile(installRoot_ + "/hello.txt"), "hello");

    packages_.remove("testpkg");
    EXPECT_FALSE(packages_.isInstalled("testpkg"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    Logger::instance().setMinLevel(LogLevel::Error);
    return RUN_ALL_TESTS();
}
