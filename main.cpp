#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <sys/select.h>
#include <netdb.h>

void getUserOptions(std::string &input_address, int &port)
{

    std::cout << "Enter IP/Hostname: ";
    std::cin >> input_address;

    struct addrinfo hints{}, *res;
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(input_address.c_str(), nullptr, &hints, &res);
    if(status != 0)
    {
        freeaddrinfo(res);
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
        std::cout << "(Hint: Host name, not full URL).\n";
        exit(EXIT_FAILURE);
    }
    
    // Converting to string IP for main func
    char ipstr[INET_ADDRSTRLEN];
    struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
    inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));

    input_address = ipstr;
    std::cout << "Selected: " << input_address << "\n";

    std::cout << "Enter a port number: ";
    std::cin >> port;
    if(port < 1 || port > 65535)
    {
        freeaddrinfo(res);
        std::cerr << "Invalid port number. (Between 1-65535)\n";
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    std::cout << "Selected: " << port << "\n";
    

}


void port_scan(std::string SERVER_IP, int PORT, int timeout_s=5) {
    std::cout << "Initiating scan on " << SERVER_IP << PORT << ".\n";
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        std::cout << "Failed to create socket";
        return;
    }
    
    //non-blocking socket (This is so we can add a timeout)
    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, SERVER_IP.c_str(), &server_addr.sin_addr) <= 0)
    {
        std::cout << "Invalid address.\n";
        close(sockfd);
        return;
    }

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(sockfd, &writefds);

    struct timeval timeout;
    timeout.tv_sec = timeout_s;
    timeout.tv_usec = 0;

    int res = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(res < 0 && errno != EINPROGRESS)
    {
        std::cout << "[-] " << PORT << " is closed.\n";
        close(sockfd);
        return;
    }

    res = select(sockfd+1, nullptr, &writefds, nullptr, &timeout);
    if(res > 0)
    {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if(so_error == 0)
        {
            std::cout << "[+] "<< PORT << " is open.\n";
        } else
        {
            std::cout << "[-] " << PORT << " is closed.\n";
        }
    } else if (res == 0)
    {
        std::cout << "[!] " << PORT << " timed out/closed.\n";
    } 
    close(sockfd);

}

int main() {

    std::string input_address = "";
    int port = 0;

    getUserOptions(input_address, port);
    port_scan(input_address, port);

    return 0;
}


/*
To-Do:
Simple port scanner to get grasp of socket.h [X] - DONE
Port Range []
Make nice options menu []
Banner Grabbing []
OS Discovery []
Web Scraper []
*/