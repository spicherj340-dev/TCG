#include "httplib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "json.hpp"
#include "User.cpp"
using json = nlohmann::json;
using namespace std;

vector<User*> gamePlayers; // our players


std::string load_file(const std::string& path){
    std::ifstream f(path);
    std::stringstream buffer;
    buffer << f.rdbuf();
    return buffer.str();
}
int main() {
    httplib::Server svr; //instantiates the server

    // Serve static files from current directory
    svr.set_mount_point("/", ".");
    

    cout << "Server running on port 8080..." << endl;
    svr.listen("0.0.0.0", 8080);
}

















