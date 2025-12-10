#pragma once
#include "GameObjects.h"
#include <vector>
#include <stack>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>

using namespace std;

class User;
class Card;

struct GameEvent {
    string _reqtype;
    string _player;
    string _payload;
};

class GameManager
{
// I'm doing some shenanigans but basically these are all the attributes
private:
    GameManager() : _currPlayer(0) {}

    // Singleton pattern, delete duplicators
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;

    // Game events
    queue<GameEvent> _eventsQueue;
    mutex _eventLock;
    condition_variable _eventCV;

    // Players
    vector<User*> _players;
    // Index of current player
    int _currPlayer = 0;
    // Current cycle count (starts at 1)
    int _cycle = 1;
    
    stack<Card*> _stack;
    // Singleton instance
    static GameManager* _instance;

// These are public methods (get instance because singleton and also game start)
public:
    static GameManager* getInstance() 
    {
        if (!_instance) _instance = new GameManager();
        return _instance;
    }

    void addPlayer(User* player);

    void gameStart();

    void stackPush(Card* c);

    void eventAdd(const GameEvent& ev);

    const vector<User*>& getPlayers() const { return _players; }

// These are PRIVATE methods, generally you should not be able to skip phases and such
    // The game manager handles such events
private:
    // Game phases (They are per player)
    void drawPhase(User* player);
    void actionPhase(User* player);
    void responsePhase(User* player);
    void cleanupPhase(User* player);

    // Resolve stack
    void resolveSpells();

    // Check if game is over
    bool gameOver();

    // Move to next player
    void nextPlayer();

    // End game - overseer in charge of cleaning up cards and other objects
    void finalCleanup();
};