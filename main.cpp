#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <string.h>
#include <sstream>
#include <sys/select.h>
#include <netdb.h>

void getUserOptions(std::string &input_address, int &port_range_start, int &port_range_end)
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

    std::cout << "Enter a port range (e.g. 1-65535): ";
    std::string port;
    std::cin >> port;

    // extract nums from port input
    size_t dashIndex = port.find('-');
    if(dashIndex == std::string::npos)
    {
        freeaddrinfo(res);
        std::cerr << "Invalid port range.\nHINT: 1-65535.\n";
        exit(EXIT_FAILURE);
    }
    std::string str_port_start = port.substr(0, dashIndex);
    std::string str_port_end = port.substr(dashIndex + 1);
    std::stringstream(str_port_start) >> port_range_start;
    std::stringstream(str_port_end) >> port_range_end;
    
    if(port_range_start > port_range_end)
    {
        freeaddrinfo(res);
        std::cerr << "Start port must be less than or equal to end port.\n";
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(res);
    std::cout << "Selected: " << port << "\n";
    

}


void port_scan(std::string SERVER_IP, int PORT, int timeout_s=5) {
    //std::cout << "Initiating scan on " << SERVER_IP << PORT << ".\n";
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
        }
    } else if (res == 0)
    {
        std::cout << "[!] " << PORT << " timed out/closed.\n";
    } 
    close(sockfd);

}

int main() {

    std::string input_address = "";
    int port_range_start, port_range_end;

    getUserOptions(input_address, port_range_start, port_range_end);

    std::cout <<  "Initiating scan on " << input_address << " on ports " << port_range_start << " - " << port_range_end << ".\n";
    for(int i = port_range_start; i <= port_range_end; i++)
    {
        port_scan(input_address, i);
    }
    

    return 0;
}

