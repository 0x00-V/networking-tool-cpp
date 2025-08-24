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
        std::cout << "List";
        std::cout << "(-t) --target=\n(-p) --port=\n(-pr) --port-range=\nMore options coming soon!\n";
        std::cout << "Example Usage:\n" << program_name << "-t=google.com -pr=1-65535\n";
    }

    void processTarget(std::string &target_address)
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

    void processPortRange(std::string &port_range, int &port_range_start, int &port_range_end)
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


    void port_scan(std::string &target_address, int &port, int timeout_s = 1)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd < 0)
        {
            std::cout << "[E-SOCK] Failed to create socket";
            exit(EXIT_FAILURE);
        }

        // non-blocking (run socket asynchronously so we can implement timeout)
        int flags = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if(inet_pton(AF_INET, target_address.c_str(), &server_addr.sin_addr) <= 0)
        {
            close(sockfd);
            std::cout << "[E-NOCONN] Invalid address.\n";
            exit(EXIT_FAILURE);
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
            close(sockfd);
            std::cout << "[-] " << port << " is closed.\n"; 
            exit(EXIT_FAILURE);
        } 

        res = select(sockfd+1, nullptr, &writefds, nullptr, &timeout);
        if(res > 0)
        {
            int so_error;
            socklen_t len = sizeof(so_error);
            getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
            if(so_error == 0)
            {
                std::cout << "[+] " << port << " is open.\n";
            }
        } else if (res == 0)
        {
            std::cout << "[?] " << port << " timed out/closed.\n";
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
        processTarget(target_address);
        std::cout << "Target Address: " << target_address << "\n";
        if(target_port != 0)
        {   
            std::cout << "Initiating port scan on " << target_address << " on port " << target_port <<  ".\n";
            port_scan(target_address, target_port);
        } else
        {
            int port_range_start, port_range_end;
            processPortRange(port_range, port_range_start, port_range_end);
            std::cout << "Initiating port scan on " << target_address << " on ports " << port_range_start << " - "<<port_range_end << ".\n";
            for(int i = port_range_start; i <= port_range_end; i++)
            {
                port_scan(target_address, i);
            }
        }

        
    }

