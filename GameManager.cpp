#include "GameManager.h"
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
using namespace std;

// Apparently this was a problem - necessary declaration of instance pointer, init to nullptr
GameManager* GameManager::_instance = nullptr;

// Add players to pool
void GameManager::addPlayer(User* player)
{
    _players.push_back(player);
}

// Core game loop
void GameManager::gameStart()
{
    // Not enough players
    if (_players.size() < 2) 
    {
        cout << "Not enough players to start game" << endl;
        return;
    }

    // Print Game start with x players
    cout << "Game Started with " << _players.size() << " players." << endl;

    // Shuffle each player's deck
    for (User* p : _players) p->shuffle();

    // While game is not over
    while (!gameOver())
    {
        // Set current player to current player index
        User* curr = _players[_currPlayer];
        
        // Print cycle number and turn
        cout << "Cycle " << _cycle << endl;
        cout << curr->getName() << "'s turn." << endl;

        // Zero charge, then set charge to cycle
        curr->zeroCharge();
        curr->gainCharge(_cycle);

        drawPhase(curr);
        actionPhase(curr);
        responsePhase(curr);
        cleanupPhase(curr);

        nextPlayer();
    }

    cout << "Game over." << endl;
}

// Push a card onto the stack
void GameManager::stackPush(Card* c)
{
    _stack.push(c);
}

// Push a gameEvent
void GameManager::eventAdd(const GameEvent& ev)
{
    {
        // lock_guard automatically locks AND unlocks after leaving {}(beautiful)
        lock_guard<mutex> lock(_eventLock);
        _eventsQueue.push(ev);
    }
    // Notify that event is now waiting
    _eventCV.notify_one();
}

// Game phases (They are per player)
void GameManager::drawPhase(User* player)
{
    cout << player->getName() << " is drawing." << endl;
    Card* drawn = player->draw();
    if (drawn == nullptr)
    {
        cout << "Failed to draw." << endl;
        return;
    }

    cout << drawn->getName() << " added to " << player->getName() << "'s hand." << endl;
}

// This one is a beefcake
void GameManager::actionPhase(User* player)
{
    cout << player->getName() << " is in action phase. Waiting for event..." << endl;
    // Wait for an event
    while(true)
    {
        GameEvent event;
        bool evWaiting = false;

        // lock_guard needs {} to know when to lock and unlock, but it's safer than manually doing it
        {
            // Lock Events, we don't want weird stuff
            lock_guard<mutex> lock(_eventLock);

            // When events are waiting
            if (!_eventsQueue.empty())
            {
                // Event = LIFO event, we are now waiting
                event = _eventsQueue.front();
                _eventsQueue.pop();
                evWaiting = true;
            }
        }
        // We now unlocked mutex
        // If we have an event:
        if (evWaiting)
        {
            // This stage only accepts PLAY events, otherwise we discard
            if (event._reqtype != "play")
            {
                if (event._reqtype == "pass")
                {
                    cout << "Passing phase." << endl;
                    return;
                }
                cout << "Incorrect event type ignored. Type: " << event._reqtype << endl;
                
                // Repeat the loop
                continue;
            }

            // Otherwise, we have a play event!
            // MAKE SURE THAT EVENT PLAYER AND PLAYER NAMES ARE THE SAME!!!!
            // Only current players can play
            if (event._player != player->getName())
            {
                cout << "Attempt to play out of turn from " << event._player << endl;
                continue;
            }

            // Check if the card exists in hand
            string cname = event._payload;
            Card* chosen = nullptr;

            // Search hand for card
            for (Card* c : player->getHand()->getAllCards())
            {
                if (c->getName() == cname)
                {
                    chosen = c;
                    break;
                }
            }
            
            // Failed to find case
            if (!chosen)
            {
                cout << "ERROR: " << cname << " NOT FOUND IN HAND." << endl;
                continue;
            }

            // Can't be defensive card:
            if (chosen->getType() == 'D')
            {
                cout << "Defensive card cannot be played in this phase." << endl;
                continue;
            }

            // Do we need a target?
            if (chosen->needsTarget())
            {
                bool waitingfortarg = true;
                while (waitingfortarg)
                {
                    // Wait - lock while evaluating
                    unique_lock<mutex> lock(_eventLock);
                    cout << "Waiting for target." << endl;
                    // Wait for queue to not be empty (my basic understanding, had to look this up ngl)
                    _eventCV.wait(lock, [this]{return !_eventsQueue.empty();});

                    // When events are waiting
                    // Event = LIFO event
                    event = _eventsQueue.front();
                    _eventsQueue.pop();
                    
                    // Need target, we only accept target events
                    if (event._reqtype != "target")
                    {
                        cout << "Incorrect event type ignored. Type: " << event._reqtype << endl;
                        // Repeat the loop
                        continue;
                    }
                    waitingfortarg = false;

                    string targname = event._payload;
                    User* targ = nullptr;

                    // Search for target in players
                    for (User* p : _players)
                    {
                        if (p->getName() == targname)
                        {
                            targ = p;
                            break;
                        }
                    }

                    // Failed to find target
                    if (!targ)
                    {
                        cout << "ERROR: COULD NOT RESOLVE TARGET " << targname <<"." << endl;
                        continue; 
                    }
                    chosen->setTarget(targ);
                }
            }

            // Player now plays card, user pushes it onto stack
            bool success = player->play(chosen);
            
            // Failure to play card for some odd reason (probably no charge)
            if (!success)
            {
                cout << "Card " << cname << " could not be played." << endl;
            }
        }

        // Wait - lock while evaluating
        unique_lock<mutex> lock(_eventLock);
        // Wait for queue to not be empty (my basic understanding, had to look this up ngl)
        _eventCV.wait(lock, [this]{return !_eventsQueue.empty();});
    }
}

void GameManager::responsePhase(User* player)
{
    cout << player->getName() << " is in response phase. Waiting for event..." << endl;
    bool responses = true;

    while(responses)
    {
        responses = false;

        // Everyone gets a chance
        for(User* p : _players)
        {
            GameEvent event;
            // Wait for an event
            {
                unique_lock<mutex> lock(_eventLock);
                _eventCV.wait(lock, [this]{ return !_eventsQueue.empty(); });
                event = _eventsQueue.front();
                _eventsQueue.pop();
            }

            // Handle only player with priority
            if(event._player != p->getName())
            {
                cout << "Player attempted to play without priority." << endl;
                continue;
            }

            if(event._reqtype == "pass")
            {
                cout << p->getName() << " passes." << endl;
                continue; // next player
            }

            if(event._reqtype == "play")
            {
                string cname = event._payload;
                Card* chosen = nullptr;

                // Search hand
                for(Card* c : p->getHand()->getAllCards())
                    if(c->getName() == cname)
                    {
                        chosen = c;
                        break;
                    }

                if(!chosen)
                {
                    cout << "ERROR: " << cname << " NOT FOUND IN HAND." << endl;
                    continue;
                }

                // Only defensive or utility
                if(chosen->getType() == 'O')
                {
                    cout << "Offensive cards cannot be played in response." << endl;
                    continue;
                }

                // Do we need a target?
                if (chosen->needsTarget())
                {
                    bool waitingfortarg = true;
                    while (waitingfortarg)
                    {
                        // Wait - lock while evaluating
                        unique_lock<mutex> lock(_eventLock);
                        cout << "Waiting for target." << endl;
                        // Wait for queue to not be empty (my basic understanding, had to look this up ngl)
                        _eventCV.wait(lock, [this]{return !_eventsQueue.empty();});

                        // When events are waiting
                        // Event = LIFO event
                        event = _eventsQueue.front();
                        _eventsQueue.pop();
                        
                        // Need target, we only accept target events
                        if (event._reqtype != "target")
                        {
                            cout << "Incorrect event type ignored. Type: " << event._reqtype << endl;
                            // Repeat the loop
                            continue;
                        }
                        waitingfortarg = false;

                        string targname = event._payload;
                        User* targ = nullptr;

                        // Search for target in players
                        for (User* pt : _players)
                        {
                            if (pt->getName() == targname)
                            {
                                targ = pt;
                                break;
                            }
                        }

                        // Failed to find target
                        if (!targ)
                        {
                            cout << "ERROR: COULD NOT RESOLVE TARGET " << targname <<"." << endl;
                            continue; 
                        }
                        chosen->setTarget(targ);
                    }
                }
                // Player now plays card, user pushes it onto stack
                bool success = p->play(chosen);
                
                // Failure to play card for some odd reason (probably no charge)
                if (!success)
                {
                    cout << "Card " << cname << " could not be played." << endl;
                }

                // Someone played a card, we can respond again
                responses = true;
            }
        }
    }
    cout << "No more responses, moving to cleanup." << endl;
}

void GameManager::cleanupPhase(User* player)
{
    cout << "Cleanup phase, resolving stack." << endl;
    resolveSpells();
    return;
}

// Resolve stack
void GameManager::resolveSpells()
{
    // While cards on stack
    while (!_stack.empty())
    {
        // Find top card on stack
        Card *c = _stack.top();

        // Remove top card from stack
        _stack.pop();

        // Resolve (cout for debugging purposes)
        cout << "Resolving: " << c->getName() << endl;
        c->resolve();

        // Get card's owner
        User* owner = c->getOwner();

        // Send the resolved card to the graveyard
        owner->getGraveyard()->addCard(c);
    }
}

// Check if game is over
bool GameManager::gameOver()
{
    // Set amount of alive players to 0
    int alive = 0;

    for (User* p : _players)
    {
        // Player is alive if >0 life
        if (p->getLife() > 0) alive++;
    }

    // Game is over once 1 or less players are alive
    if (alive <= 1) return true;
    return false;
}

// Move to next player
void GameManager::nextPlayer()
{
    _currPlayer++;
    // Gotta cast to int because size is type size_t
    if (_currPlayer >= static_cast<int>(_players.size()))
    {
        // Wrap around to player 0 and increment cycle
        _currPlayer = 0;
        _cycle++;
    }
}

// Final cleanup for after game end
void GameManager::finalCleanup()
{
    // For each player
    for (User* player : _players)
    {
        // Take all their zones
        vector<Zone*> zones = {
            player->getDeck(),
            player->getHand(),
            player->getField(),
            player->getGraveyard()
        };

        // And then for each zone
        for (Zone* z : zones)
        {
            // Delete each card in the zone
            z->cleanup();
        }

        // Then delete the player, which will handle the zones
        delete player;
    }
}
