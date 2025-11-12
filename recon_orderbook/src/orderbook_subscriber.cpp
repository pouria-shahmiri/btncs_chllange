// Simple TCP client subscriber - connects to publisher TCP server and logs received lines
#include "orderbook_subscriber.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>

static constexpr const char* PUBLISHER_HOST = "127.0.0.1";
static constexpr int PUBLISHER_PORT = 9000;

OrderbookSubscriber::OrderbookSubscriber()
    : participant_(nullptr), subscriber_(nullptr), topic_(nullptr), reader_(nullptr),
      running_(false), recordCount_(0) {
}

OrderbookSubscriber::~OrderbookSubscriber() {
    shutdown();
}

bool OrderbookSubscriber::initialize() {
    std::cout << "=== Orderbook Subscriber Initialization ===" << std::endl;
    std::cout << "Publisher address: " << PUBLISHER_HOST << ":" << PUBLISHER_PORT << std::endl;
    std::cout << "(will keep trying to connect if publisher is not available)" << std::endl;
    std::cout << "=========================================" << std::endl;

    // No connection attempted here; listen() will handle reconnects.
    participant_ = (void*)(intptr_t)(-1);
    running_ = true;
    recordCount_ = 0;
    return true;
}

void OrderbookSubscriber::listen() {
    std::cout << "Listening for incoming market data...\n" << std::endl;
    char buf[4096];
    std::string partial;

    int sock = -1;
    struct sockaddr_in serv_addr;
    std::memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PUBLISHER_PORT);
    inet_pton(AF_INET, PUBLISHER_HOST, &serv_addr.sin_addr);

    while (running_) {
        // Ensure we have a connected socket
        if (sock < 0) {
            sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock < 0) {
                std::perror("socket");
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            // Try to connect; if fail, close and retry after delay
            if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
                close(sock);
                sock = -1;
                // publisher may not be up yet; wait and retry
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            // Set non-blocking so recv doesn't block indefinitely
            int flags = fcntl(sock, F_GETFL, 0);
            fcntl(sock, F_SETFL, flags | O_NONBLOCK);

            // store active socket in participant_ for shutdown visibility
            participant_ = (void*)(intptr_t)sock;

            std::cout << "Connected to publisher." << std::endl;
        }

        // Read available data
        ssize_t n = recv(sock, buf, sizeof(buf)-1, 0);
        if (n > 0) {
            buf[n] = '\0';
            partial += buf;
            size_t pos;
            while ((pos = partial.find('\n')) != std::string::npos) {
                std::string line = partial.substr(0, pos);
                partial.erase(0, pos+1);
                if (!line.empty()) {
                    recordCount_++;
                    std::cout << "[" << std::setfill('0') << std::setw(8) << recordCount_ << "] " << line << std::endl;
                }
            }
        } else if (n == 0) {
            // clean disconnect by peer
            std::cout << "Publisher closed the connection. Will retry connecting..." << std::endl;
            close(sock);
            sock = -1;
            participant_ = (void*)(intptr_t)(-1);
            partial.clear();
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        } else {
            // n < 0
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // no data available currently
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            } else {
                // other error -> treat as disconnect and retry
                std::perror("recv");
                std::cout << "Socket error, will retry connecting..." << std::endl;
                close(sock);
                sock = -1;
                participant_ = (void*)(intptr_t)(-1);
                partial.clear();
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
        }
    }

    if (sock >= 0) close(sock);
    std::cout << "\nListener stopped. Total records: " << recordCount_ << std::endl;
}

void OrderbookSubscriber::shutdown() {
    if (running_) {
        running_ = false;
    }
    int sock = (int)(intptr_t)participant_;
    if (sock >= 0) close(sock);
    std::cout << "Subscriber shutdown complete." << std::endl;
}
