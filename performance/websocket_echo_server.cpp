//
// Created by Ivan Shynkarenka on 16.03.2017
//

#include "server/asio/service.h"
#include "server/asio/websocket_server.h"

#include <atomic>
#include <iostream>

#include "../../modules/cpp-optparse/OptionParser.h"

using namespace CppServer::Asio;

class EchoSession;

class EchoServer : public WebSocketServer<EchoServer, EchoSession>
{
public:
    using WebSocketServer<EchoServer, EchoSession>::WebSocketServer;

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Server caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

class EchoSession : public WebSocketSession<EchoServer, EchoSession>
{
public:
    using WebSocketSession<EchoServer, EchoSession>::WebSocketSession;

protected:
    void onReceived(WebSocketMessage message) override
    {
        // Resend the message back to the client
        Send(message);
    }

    void onError(int error, const std::string& category, const std::string& message) override
    {
        std::cout << "Session caught an error with code " << error << " and category '" << category << "': " << message << std::endl;
    }
};

int main(int argc, char** argv)
{
    auto parser = optparse::OptionParser().version("1.0.0.0");

    parser.add_option("-h", "--help").help("Show help");
    parser.add_option("-p", "--port").action("store").type("int").set_default(4444).help("Server port. Default: %default");

    optparse::Values options = parser.parse_args(argc, argv);

    // Print help
    if (options.get("help"))
    {
        parser.print_help();
        parser.exit();
    }

    // Server port
    int port = options.get("port");

    std::cout << "Server port: " << port << std::endl;

    // Create a new Asio service
    auto service = std::make_shared<Service>();

    // Start the service
    std::cout << "Asio service starting...";
    service->Start();
    std::cout << "Done!" << std::endl;

    // Create a new echo server
    auto server = std::make_shared<EchoServer>(service, InternetProtocol::IPv4, port);

    // Start the server
    std::cout << "Server starting...";
    server->Start();
    std::cout << "Done!" << std::endl;

    std::cout << "Press Enter to stop the server or '!' to restart the server..." << std::endl;

    // Perform text input
    std::string line;
    while (getline(std::cin, line))
    {
        if (line.empty())
            break;

        // Restart the server
        if (line == "!")
        {
            std::cout << "Server restarting...";
            server->Restart();
            std::cout << "Done!" << std::endl;
            continue;
        }
    }

    // Stop the server
    std::cout << "Server stopping...";
    server->Stop();
    std::cout << "Done!" << std::endl;

    // Stop the service
    std::cout << "Asio service stopping...";
    service->Stop();
    std::cout << "Done!" << std::endl;

    return 0;
}
