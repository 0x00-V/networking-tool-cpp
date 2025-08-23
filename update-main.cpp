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


    // --help (We can also use "-h" for shorthand)
    void help(std::string program_name)
    {
        std::cout << "Daniel's Port Scanner";
        std::cout << "\n\nSupported CLI Args:\n";
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
            std::cout << "Target Port: " << target_port << "\n";
        } else
        {
            std::cout << "Target Port Range: " << port_range << "\n";
        }
    }


    // To do:
    /*
    [] Port Scanner Functionality
    [] Improved Exception Handling
    [] CLI ARGS Input Validation
    */