#include <iostream>
#include <string>
#include <map>
#include <stdexcept>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <bits/stdc++.h>
#include "suit.h"

const int BACKLOG = 10;

std::string getFormatStr(suitType suit);

// M06 part b lab - Modified suit colors by Samuel Ogar
// Changed ANSI escape codes to use different color combinations
// Reference: https://en.wikipedia.org/wiki/ANSI_escape_code

int main(int argc, char *argv[])
{
    int sockfd;
    int clientfd;
    addrinfo hints;
    addrinfo *servInfo;
    addrinfo *p;
    int rv;      // return value
    int yes = 1; // reuse port
    socklen_t sin_size;
    sockaddr_storage their_addr;
    char s[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP socket
    hints.ai_flags = AI_PASSIVE;     // use my IP;

    try
    {
        rv = getaddrinfo(NULL, "9431", &hints, &servInfo);
        if (rv != 0)
        {
            throw std::runtime_error("getaddrinfo error");
        }

        sockfd = socket(servInfo->ai_family, servInfo->ai_socktype, servInfo->ai_protocol);
        if (sockfd == -1)
        {
            throw std::invalid_argument("There was an error creating the socket");
        }

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        rv = bind(sockfd, servInfo->ai_addr, servInfo->ai_addrlen);
        if (rv == -1)
        {
            close(sockfd);
            throw std::invalid_argument("Server is unable to bind");
        }

        rv = listen(sockfd, BACKLOG);
        if (rv == -1)
        {
            close(sockfd);
            throw std::invalid_argument("Server is unable to listen");
        }

        std::cout << "server: waiting for connections..." << std::endl;

        while (true)
        {
            sin_size = sizeof(their_addr);
            clientfd = accept(sockfd, (sockaddr *)&their_addr, &sin_size);
            if (clientfd == -1)
            {
                throw std::invalid_argument("Error accpeting client");
            }

            inet_ntop(their_addr.ss_family, (struct sockaddr_in *)&their_addr, s, sizeof(s));
            std::cout << "server: got connection from " << s << std::endl;

            uint32_t val;
            rv = recv(clientfd, &val, sizeof(val), 0);
            if (rv == sizeof(val))
            {
                val = ntohl(val);
                std::cout << "receiving: " << val << std::endl;

                suitType suit = static_cast<suitType>(val);
                std::string response = getFormatStr(suit);

                val = htonl(response.length());
                rv = send(clientfd, &val, sizeof(val), 0);
                rv = send(clientfd, response.c_str(), response.length(), 0);
            }

            close(clientfd);
        }
    }
    catch (const std::runtime_error &e)
    {
        std::cerr << e.what() << " " << gai_strerror(rv) << '\n';
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}

std::string getFormatStr(suitType suit)
{
    // Modified color scheme by Samuel Ogar for Module 6 Part B Lab
    // ANSI Escape Code Format: \033[STYLE;BACKGROUNDm\033[FOREGROUNDm
    // 
    // Changes made:
    // - HEARTS: Changed to bright red text on black background (more dramatic)
    // - DIAMONDS: Changed to bright blue text on yellow background (vibrant contrast)
    // - CLUBS: Changed to green text on white background (fresh look)
    // - SPADES: Changed to magenta text on cyan background (bold and unique)
    
    static std::map<suitType, std::string> suitColors = {
        {suitType::HEARTS, "\033[1m\033[40m\033[91m"},      // Bold + Black background + Bright Red text
        {suitType::DIAMONDS, "\033[1m\033[43m\033[94m"},    // Bold + Yellow background + Bright Blue text
        {suitType::CLUBS, "\033[1m\033[47m\033[32m"},       // Bold + White background + Green text
        {suitType::SPADES, "\033[1m\033[46m\033[35m"}       // Bold + Cyan background + Magenta text
    };
    
    return suitColors[suit];
}
