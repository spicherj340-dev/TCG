#include "httplib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <json.hpp>
using json = nlohmann::json;
using namespace std;


int main() {
    httplib::Server svr; //instantiates the server

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) { // sets site to index html
        std::ifstream file("index.html");
        std::stringstream buffer;
        buffer << file.rdbuf();  // read the whole file
        res.set_content(buffer.str(), "text/html");
    });
    // basic example:



    //press the create user button or soemthing
    svr.Post("/echo", [](const httplib::Request&req, httplib::Response& res) {
      
        try {
            // Parse JSON body
            json data = json::parse(req.body);

            // Extract the message
            std::string message = data.value("message", "");

            if (message.empty()) {
                res.status = 400;
                res.set_content("Missing 'message' field", "text/plain");
                return;
            }

            // Respond with formatted text
            res.set_content("You typed: " + message, "text/plain");
        }
        catch (json::parse_error& e) {
            res.status = 400;
            res.set_content("Invalid JSON", "text/plain");
        }
    });



    std::cout << "Server running on port 8080..." << std::endl;
    svr.listen("0.0.0.0", 8080);
}






