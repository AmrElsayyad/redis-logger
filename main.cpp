#include <iostream>
#include <string>
#include <sw/redis++/redis++.h>

using namespace sw::redis;

int main(int argc, char **argv) {
    ConnectionOptions opts;
    opts.host = "127.0.0.1";
    opts.port = 6379;
    opts.socket_timeout = std::chrono::milliseconds(100);
    
    // Create an Redis object, which is movable but NOT copyable.
    auto redis = Redis(opts);

    // Create a Subscriber.
    auto sub = redis.subscriber();

    // Set callback functions.
    sub.on_message([](std::string const& channel, std::string const& msg) {
        std::cout << channel << ": " << msg << "\n";
    });

    sub.on_pmessage([](std::string const& pattern, std::string const& channel, std::string const& msg) {
        std::cout << "[" << pattern << "] " << channel << ": " << msg << "\n";
    });

    sub.on_meta([](Subscriber::MsgType type, OptionalString channel, long long num) {
        if (channel) {
            switch (type) {
            case Subscriber::MsgType::SUBSCRIBE:
                std::cout << num << ") Subscribed to " << channel.value() << std::endl;
                break;
            case Subscriber::MsgType::PSUBSCRIBE:
                std::cout << num << ") Subscribed to [" << channel.value() << "]\n";
                break;   
            case Subscriber::MsgType::UNSUBSCRIBE:
            case Subscriber::MsgType::PUNSUBSCRIBE:
                std::cout << num << ") Unsubscribed to " << channel.value() << std::endl;
                break;
            case Subscriber::MsgType::MESSAGE:
            case Subscriber::MsgType::PMESSAGE:
                std::cout << num << ") New message in " << channel.value() << std::endl;
                break;
            }
        }
    });

    // Subscribe to channels and patterns.
    sub.subscribe("channel1");
    sub.subscribe({"channel2", "channel3"});

    sub.psubscribe("pattern*");

    // Consume messages in a loop.
    while (true) {
        try {
            sub.consume();
        } catch (const TimeoutError &e) {
            continue;
        } catch (const Error &e) {
            std::cout << e.what() << std::endl;
        }
    }

    return 0;
}