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


// --target=0.0.0.0, or --target=google.com (We can also use "-t" for shorthand)
void target(const std::string &value) {
    std::cout << "Target: " << value << "\n";
}


// --port=80 (We can also use "-p" for shorthand)
void port(int value){
    std::cout << "Port: " << value << "\n";
}

// --port-range=1-65535 (We can also use "-pr" for shorthand)
void port_range(int start, int end) {
    std::cout << "Port range: " << start << "-" << end << "\n";
}


// Port scanning function
void port_scan()
{
     std::cout << "Work in progress";
}







int main(int argc, char *argv[]) {
    if(argc == 1)
    {
        std::cout << "Incorrect Usage.\n" << "(Hint: Try \"" << argv[0] << " -h (or --help)\")\n";
    }
    for(int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if(arg == "--help" || arg == "-h") {
            help(argv[0]);
        }else if(arg.rfind("--target=", 0) == 0) {
            std::string value = arg.substr(9);
            target(value);
        }else if(arg.rfind("-t=", 0) == 0) {
            std::string value = arg.substr(3);
            target(value);
        }else if(arg.rfind("--port=", 0) == 0) {
            int value = std::stoi(arg.substr(7));
            port(value);
        }else if(arg.rfind("-p=", 0) == 0) {
            int value = std::stoi(arg.substr(3));
            port(value);
        }else if(arg.rfind("--port-range=", 0) == 0) {
            std::string range = arg.substr(13);
            size_t dash = range.find('-');
            if(dash != std::string::npos) {
                int start = std::stoi(range.substr(0, dash));
                int end = std::stoi(range.substr(dash+1));
                port_range(start, end);
            }else {
                std::cerr << "Invalid port range: " << range << "\n";
            }
        }else if(arg.rfind("-pr=", 0) == 0) {
            std::string range = arg.substr(4);
            size_t dash = range.find('-');
            if(dash != std::string::npos) {
                int start = std::stoi(range.substr(0, dash));
                int end = std::stoi(range.substr(dash+1));
                port_range(start, end);
            }else {
                std::cerr << "Invalid port range: " << range << "\n";
            }
        }else {
            std::cerr << "Unknown argument: " << arg << "\n(Hint: Try \"" << arg[0] << " -h (or --help)\")\n";
        }
    }
}


// To do:
/*
[] Port Scanner Functionality
[] Improved Exception Handling
[] CLI ARGS Input Validation
*/