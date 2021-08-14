#include "NetworkRecorder.hpp"

bool connectToServer(sf::TcpSocket *socket, sf::IpAddress ip)
{
    sf::Packet sendPacket;

    if (socket->connect(ip, 5000) == sf::Socket::Done) {
        std::cout << "Connected!" << std::endl;
        socket->setBlocking(false);
        socket->send(sendPacket);
    }
    else {
        std::cout << "Server down!" << std::endl;
        return false;
    }
    return true;
}

int main(void)
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML NetworkRecorder");
    sf::Packet receivePacket;
    sf::Event event;
    sf::IpAddress ip = sf::IpAddress::getLocalAddress();
    NetworkRecorder input;

    if (!sf::SoundRecorder::isAvailable())
        return EXIT_FAILURE;

    if (!connectToServer(&input.socket, ip))
        return 84;

    input.start();

    while (window.isOpen()) {
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed) {
                if (input.isRunning()) {
                    std::cout << "Listening and recording..." << std::endl;
                    input.stop();
                }
                else {
                    std::cout << "Stopped." << std::endl;
                    input.start();
                }
            }
        }
        if (input.socket.receive(receivePacket) == sf::Socket::Done) {
            input.receiveData(receivePacket);
            receivePacket.clear();
        }
        window.clear(input.isRunning() ? sf::Color::White : sf::Color::Black);
        window.display();
    }

    return EXIT_SUCCESS;
}
