#include "market_data_publisher.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

// TCP streaming publisher: sends the same line printed to console over a TCP socket
static constexpr int DEFAULT_PORT = 9000;

MarketDataPublisher::MarketDataPublisher()
    : participant_(nullptr), publisher_(nullptr), topic_(nullptr), writer_(nullptr),
      server_fd_(-1), client_fd_(-1) {
}

MarketDataPublisher::~MarketDataPublisher() {
    shutdown();
}

bool MarketDataPublisher::initialize() {
    // Create TCP server socket on localhost:9000
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "Failed to create server socket" << std::endl;
        return false;
    }

    int opt = 1;
    setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(DEFAULT_PORT);

    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind server socket (is port in use?)" << std::endl;
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    if (listen(server_fd_, 1) < 0) {
        std::cerr << "Failed to listen on server socket" << std::endl;
        close(server_fd_);
        server_fd_ = -1;
        return false;
    }

    // Set non-blocking accept so publisher doesn't block if no subscriber connected
    int flags = fcntl(server_fd_, F_GETFL, 0);
    fcntl(server_fd_, F_SETFL, flags | O_NONBLOCK);

    client_fd_ = -1;

    std::cout << "TCP Publisher initialized on 127.0.0.1:" << DEFAULT_PORT << " (non-blocking accept)" << std::endl;
    std::cout << "Waiting for a subscriber to connect..." << std::endl;
    return true;
}

static void accept_if_pending(int server_fd, int &client_fd) {
    if (client_fd != -1) return;
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int fd = accept(server_fd, (struct sockaddr*)&caddr, &len);
    if (fd >= 0) {
        client_fd = fd;
        // set client to non-blocking
        int flags = fcntl(client_fd, F_GETFL, 0);
        fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);
        std::cout << "Subscriber connected from " << inet_ntoa(caddr.sin_addr) << ":" << ntohs(caddr.sin_port) << std::endl;
    }
}

void MarketDataPublisher::publishRecord(const MBORecord& record) {
    // Build a textual representation (same as the previous console output)
    char buf[512];
    int n = snprintf(buf, sizeof(buf), "%llu|%u|%u|%u|%llu|%llu\n",
                     (unsigned long long)record.ts_event,
                     (unsigned)record.price,
                     (unsigned)record.size,
                     (unsigned)record.side,
                     (unsigned long long)record.order_id,
                     (unsigned long long)record.sequence);

    // Print to console as before
    std::cout << "[DDS] Publishing: " << buf;

    // Try to accept a pending connection if none
    if (server_fd_ >= 0) accept_if_pending(server_fd_, client_fd_);

    // Send to connected client if exists
    if (client_fd_ != -1) {
        ssize_t sent = send(client_fd_, buf, n, 0);
        if (sent < 0) {
            // error -> close client_fd
            close(client_fd_);
            client_fd_ = -1;
            std::cerr << "Subscriber disconnected (send failed)." << std::endl;
        }
    }
}

void MarketDataPublisher::shutdown() {
    if (client_fd_ != -1) {
        close(client_fd_);
        client_fd_ = -1;
    }
    if (server_fd_ != -1) {
        close(server_fd_);
        server_fd_ = -1;
    }
    std::cout << "TCP Publisher shutdown complete" << std::endl;
}
