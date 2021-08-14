#include <vector>
#include <iostream>
#include <SFML/Network.hpp>

void listenToConnection(sf::TcpListener *listener, sf::SocketSelector *selector, std::vector<sf::TcpSocket *>*clients)
{
    sf::TcpSocket *socket;
    socket = new sf::TcpSocket;
    sf::Packet receivePacket;

    listener->accept(*socket);
    selector->add(*socket);
    clients->push_back(socket);
    if (socket->receive(receivePacket) == sf::Socket::Done) {
        std::cout << "Unauthentified user connected." << std::endl;
    }
}

void listenToSocket(sf::SocketSelector *selector, std::vector<sf::TcpSocket *>*clients)
{
    sf::Packet receivePacket;

    for (int i = 0; i < clients->size(); i++) {
        if (selector->isReady(*(*clients)[i])) {
            if ((*clients)[i]->receive(receivePacket) == sf::Socket::Done) {
                std::cout << "User " << i << " sent a packet! Sending it to everyone..." << receivePacket.getDataSize() << std::endl;
                for (int j = 0; j < clients->size(); j++)
                    if (i != j)
                        (*clients)[j]->send(receivePacket);
            }
        }
    }
}

int main(void)
{
    sf::TcpListener listener;
    sf::SocketSelector selector;
    std::vector<sf::TcpSocket *>clients;

    listener.listen(5000);
    selector.add(listener);
    while (true) {
        if (selector.wait()) {
            if (selector.isReady(listener))
                listenToConnection(&listener, &selector, &clients);
            else
                listenToSocket(&selector, &clients);
        }
    }
    return 0;
}
