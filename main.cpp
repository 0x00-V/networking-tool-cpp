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


    void help(std::string program_name)
    {
        std::cout << "0x00V's Scanner - Version 0.05\n";
        std::cout << "Usage: " << program_name << " [Options]\n";
        std::cout << "\n-----------------------\nTarget Specification\n-----------------------\n  Can pass hostnames and IP addresses.\n  [-t] --target=<hostname/IP address>\n";
        std::cout << "\n-----------------------\nPort Specification\n-----------------------\n  Can pass single port or a range of ports.\n  [-p] --port<1-65535>\n  [-pr] --port-range=<1-65535>\n";
        std::cout << "\n-----------------------\nPort Specification\n-----------------------\n  --banner-grabbing - Enable banner grabbing\n";
        std::cout << "Example Usage:\n" << program_name << "-t=google.com -pr=1-65535\n";
    }

    void process_target(std::string &target_address)
    {
        struct addrinfo hints{}, *res;
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int status = getaddrinfo(target_address.c_str(), nullptr, &hints, &res);
        if(status != 0)
        {
            freeaddrinfo(res);
            std::cout << "getaddrinfo error: " << gai_strerror(status) << "\n";
            std::cout << "Invalid Host Name (Please exclude http/https).\n";
            exit(EXIT_FAILURE);
        }

        char ipstr[INET_ADDRSTRLEN];
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
        target_address = ipstr;
        freeaddrinfo(res);
    }

    void process_port_range(std::string &port_range, int &port_range_start, int &port_range_end)
    {
        size_t dashIndex = port_range.find('-');
        if(dashIndex == std::string::npos)
        {
            std::cout << "[E-PR] Could not find \"-\" inside of your port range.\n";
            exit(EXIT_FAILURE);
        }
        std::string str_port_start = port_range.substr(0, dashIndex);
        std::string str_port_end = port_range.substr(dashIndex+1);
        std::stringstream(str_port_start) >> port_range_start;
        std::stringstream(str_port_end) >> port_range_end;

        if(port_range_end > 65535 || port_range_start < 1)
        {
            std::cout << "[E-PR] Please enter a port range within 1-65535.\n";
            exit(EXIT_FAILURE);
        } else if (port_range_start > port_range_end)
        {
            std::cout << "[E-PR] Starting port range should be less than or equal to ending port range.\n";
        }
    }


    void port_scan(std::string &target_address, int &port, int timeout_s, bool banner_grabbing)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0)
        {
            std::cout << "[E-SOCK] Failed to create socket";
            exit(EXIT_FAILURE);
        } 

        // non-blocking sock
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, target_address.c_str(), &addr.sin_addr);

        int res = connect(sockfd, (sockaddr*)&addr, sizeof(addr));
        if(res < 0 && errno != EINPROGRESS)
        {
            close(sockfd);
            return;
        }

        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(sockfd, &writefds);
        timeval timeout{timeout_s, 0};
        res = select(sockfd + 1, nullptr, &writefds, nullptr, &timeout);

        if(res <= 0)
        {
            close(sockfd);
            return;
        }

        int socket_error;
        socklen_t len = sizeof(socket_error);
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &socket_error, &len);
        if(socket_error != 0 )
        {
            close(sockfd);
            return;
        }

        std::cout << "[+] " << port << " open\n";
        if(banner_grabbing == true)
        {
            auto try_read = [&](int timeout_s) -> std::string 
        {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            timeval t{timeout_s, 0};
            int sel = select(sockfd + 1, &readfds, nullptr, nullptr, &t);
            if(sel > 0 && FD_ISSET(sockfd, &readfds))
            {
                char buf[1024];
                int n = read(sockfd, buf, sizeof(buf)-1);
                if(n > 0) {buf[n] = '\0'; return std::string(buf);}
            }
            return "";
        };

        std::string banner = try_read(1);

        if(banner.empty())
        {
            const char* probe = nullptr;
            switch(port) {
            case 21: probe = "USER anonymous\r\n"; break;
            case 25: probe = "EHLO example.com\r\n"; break;
            case 80: probe = "HEAD / HTTP/1.0\r\n\r\n"; break;
            case 6379: probe = "PING\r\n"; break;
        }
        if(probe) write(sockfd, probe, strlen(probe));
        banner = try_read(2);
    }

    if(!banner.empty()) std::cout << "[Banner] " << banner << "\n";
        }

    close(sockfd);
}
  

    int main(int argc, char *argv[]) {
        if(argc == 1)
        {
            std::cout << "[E-NP] Please provide valid options. \n(Hint: Try \"" << argv[0] << " -h (or --help)\")\n";
            return 0;
        }

        int target_port = 0;
        std::string target_address = "not set";
        std::string port_range = "0";
        bool banner_grabbing = false;


        for(int i = 1; i < argc; i++) {
            std::string arg = argv[i];
            

            if(arg == "--help" || arg == "-h") {
                help(argv[0]);
                return 0;
            }else if(arg.rfind("--target=", 0) == 0) {
                target_address = arg.substr(9);
            }else if(arg.rfind("-t=", 0) == 0) {
                target_address = arg.substr(3);
            }else if(arg.rfind("--port=", 0) == 0) {
                try {
                    target_port = std::stoi(arg.substr(7));
                } catch (const std::invalid_argument& e) {
                    std::cout << "[E-STOI-P] Please enter a numeric value for the port.\n";
                    return 0;
                } catch (const std::out_of_range& e) {
                    std::cout << "[E-STOI-P] Port value is out of range.\n";
                    return 0;
                }
            } else if(arg.rfind("-p=", 0) == 0) {
                try {
                    target_port = std::stoi(arg.substr(3));
                } catch (const std::invalid_argument& e) {
                    std::cout << "[E-STOI-P] Please enter a numeric value for the port.\n";
                    return 0;
                } catch (const std::out_of_range& e) {
                    std::cout << "[E-STOI-P] Port value is out of range.\n";
                    return 0;
                }
            }else if(arg.rfind("--port-range=", 0) == 0) {
                port_range = arg.substr(13);
                size_t dash = port_range.find('-');
                if(dash == std::string::npos) { std::cout << "Invalid port range: " << port_range << "\n"; }
            }else if(arg.rfind("-pr=", 0) == 0) {
                port_range = arg.substr(4);
                size_t dash = port_range.find('-');
                if(dash == std::string::npos) { std::cout << "Invalid port range: " << port_range << "\n"; }
            }else if(arg.rfind("--banner-grabbing", 0) == 0)
            {
                banner_grabbing = true;
            }else {
                std::cout << "[E-UKA] Unknown argument: " << argv[i] << "\n(Hint: Try \"" << argv[0] << " -h (or --help)\")\n";
                return 0;
            }
        }

        if(target_address == "not set")
        {
            std::cout << "[E-T] Please provide a target address (e.g. -t=192.168.0.1 or -t=www.example.com).\n";
            return 0;
        } else if (target_port == 0 && port_range == "0")
        {
            std::cout << "[E-P-PR] Please provide a target port (e.g. -p=80) or port range (e.g. -pr=1-100).\n";
            return 0;
        } 
        process_target(target_address);
        std::cout << "Target Address: " << target_address << "\n";
        if(target_port != 0)
        {   
            std::cout << "Initiating port scan on " << target_address << " on port " << target_port <<  ".\n";
            port_scan(target_address, target_port, 1, banner_grabbing);
        } else
        {
            int port_range_start, port_range_end;
            process_port_range(port_range, port_range_start, port_range_end);
            std::cout << "Initiating port scan on " << target_address << " on ports " << port_range_start << " - "<<port_range_end << ".\n";
            for(int i = port_range_start; i <= port_range_end; i++)
            {
                port_scan(target_address, i, 1, banner_grabbing);
            }
        }

        
    }

