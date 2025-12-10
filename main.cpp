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
    
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) { // sets site to index html
        res.set_content(load_file("index.html"), "text/html");
    });
    // test, unused.
    svr.Get("/hello", [](const httplib::Request&, httplib::Response& res) {
        cout << "hello" << endl;
        res.set_content("Hello, World!", "text/plain");
    });
    svr.Get("/index", [](const httplib::Request&, httplib::Response& res) { // sets site to index html
        res.set_content(load_file("choose.html"), "text/html");
    });
    
    
    



    //enter a player name.
    svr.Post("/echo", [](const httplib::Request&req, httplib::Response& res) {
      
        try {
            // Parse JSON body
            json data = json::parse(req.body);

            // Extract the message
            string message = data.value("message", "");

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

    svr.Post("/startgame", [](const httplib::Request& req, httplib::Response& res) {
    try {
        json data = json::parse(req.body);

        if (!data.contains("players") || !data["players"].is_array()) {
            res.status = 400;
            res.set_content("Invalid player list", "text/plain");
            return;
        }

        // Clear previous game 
        gamePlayers.clear();

        for (auto& p : data["players"]) {
            string name = p.get<string>();

            //use name as name of new user
            string storedName = name;
            if(storedName == ''){
                throw logic_error();
            }
            // Create the user and store pointer
            gamePlayers.push_back(new User(storedName));
            cout << "Created user: " << storedName << endl;
        }
        string contname = "Game started. 4 players created: ";
        string players;
        for (auto & p : gamePlayers){
            players += p->getName();
            players += ", ";
        }
        players.pop_back();
        players.pop_back();
        
        //tell the user names
        res.set_content(contname + players,  "text/plain");
    }
    catch (json::parse_error&) {
        res.status = 400;
        res.set_content("Invalid JSON", "text/plain");
    }
    catch(logic_error&){
        res.set_content("All players must have a name." "text/plain");
    }
});



    cout << "Server running on port 8080..." << endl;
    svr.listen("0.0.0.0", 8080);
}





















