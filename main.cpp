#include "httplib.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "json.hpp"
#include <chrono>
#include <thread>
//test comment   iojo
#include "GameManager.h"



using json = nlohmann::json;
using namespace std;

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
    
    GameManager* gm = GameManager::getInstance();

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
    svr.Get("/final", [gm](const httplib::Request&, httplib::Response& res) { // sets site to index html
        res.set_content(load_file("final.html"), "text/html");
        
    });

    svr.Get("/refresh", [gm](const httplib::Request&, httplib::Response& res) {

        auto &players = gm->getPlayers();
        // User* p = players[0]; //current player always stored first index

        // auto curHand = p->getHand->getVisibleCards(p); // I am having trouble getting the hand. 
        // Maybe Ian knows how to get a users hand using gamemanager.

        //covert to json
        std::ostringstream out;

        out << "{ \"players\": [";
        
        for (int i = 0; i < players.size(); i++) { // will need to add current player deck and hand as well somehow
            out << "{"
                << "\"name\":\"" << players[i]->getName() << "\","
                << "\"life\":" << players[i]->getLife() << ","
                << "\"mana\":" << players[i]->getCharge()
                << "}";

            if (i + 1 < players.size()) out << ",";
        }

        out << "] }";

        

        //backend has no getImage yet, nor a good way to get hand (i think, i just couldnt get it to work)
        //so card objects can not yet be made properly

        // out << "\"hand\":[";

        // for (int i = -; i< visibleHand.size(); i++) {
        //     Card* c = visibleHand[i];

        //     out << "{"
        //         << "\"name\":\"" << c->getName() << "\","
        //         << "\"image\":\"" << c->getImage() << "\""
        //         << "}";
        //     if (i + 1 < visibleHand.size()) out << ",";
        //     }
        //     out << "]";


        res.set_content(out.str(), "application/json");
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
    //press start game
    svr.Post("/startgame", [gm](const httplib::Request& req, httplib::Response& res) {
    try {
        json data = json::parse(req.body);

        if (!data.contains("players") || !data["players"].is_array()) {
            gm->finalCleanup();
            res.status = 400;
            res.set_content("Invalid player list", "text/plain");
           
            return;
        }


        for (auto& p : data["players"]) {
            string name = p.get<string>();

            //use name as name of new user
            string storedName = name;
            if(storedName == ""){
                gm->finalCleanup();
                throw logic_error("Must name all players.");
            }
            else{
                // Create the user and store pointer
                User * u = new User(storedName);
                gm->addPlayer(u);
                cout << "Created user: " << storedName << endl;
            }
            
        }
        string contname = "Game started. Players created: ";
        string players;
        for (auto  p : gm->getPlayers()){
            players += p->getName();
            players += ", ";
        }
        players.pop_back();
        players.pop_back();
        
        //tell the user names
        res.set_content(contname + players,  "text/plain");

        //for now, gamestart breaks the game. maybe since we cant implement draw yet. or we cooked
        // this_thread::sleep_for(chrono::seconds(2));
        // gm->gameStart();
    }
    catch (json::parse_error&) {
        res.status = 400;
        res.set_content("Invalid JSON", "text/plain");
    }
    catch(std::logic_error& e){ 
        res.status = 400; 
        
 
        res.set_content(e.what(), "text/plain"); 


   
}
});

    //draw button
    svr.Post("/draw", [gm](const httplib::Request &req, httplib::Response &res)
             {
    User* curr = gm->getPlayers()[0]; // current player
    Card* drawn = curr->draw();

});

    // Pass button
    svr.Post("/pass", [gm](const httplib::Request &req, httplib::Response &res)
             {
    User* curr = gm->getPlayers()[0]; // current player
    GameEvent ev;
    ev._reqtype = "pass";
    ev._player = curr->getName();
    ev._payload = "";

    gm->eventAdd(ev);

});

    // Discard button
    svr.Post("/discard", [gm](const httplib::Request &req, httplib::Response &res)
             {
        User* curr = gm->getPlayers()[0]; // current player
        json data = json::parse(req.body);
        string cardName = data["card"];
    
        Card* toDiscard = nullptr;
        for (Card* c : curr->getHand()->getAllCards()) {
            if (c->getName() == cardName) {
                toDiscard = c;
                break;
            }
        }
    
        curr->getHand()->removeCard(toDiscard);
        curr->getGraveyard()->addCard(toDiscard);
    
    });
    
    cout << "Server running on port 8080..." << endl;
    svr.listen("0.0.0.0", 8080);
}



















