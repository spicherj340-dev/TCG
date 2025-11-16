#include <iostream>
#include <algorithm>
#include <random>
#include <thread>
#include <chrono>
#include "httplib.h"

using namespace std;
//placeholder for card
class Card
{
    private:
        string _id;
        string _name;
        
    public:
        int mana;
};

//placeholder
class Field
{
    
};


// initialize user 
class User
{
    private:
    //initialize objects  
    //placeholder card object
    string _name; // initialize name
    int _life;
    bool _deckbuilt;
    int _decknum;
    int _mana;

    //replace these with zone -> hand, deck etc
    unordered_set<Card*> _hand;
    deque<Card*> _deck; // stack<card objects> _deck;
    deque<Card*> _graveyard;  // stack<card objects> _graveyard;

    //something something field

    

    public:
    User(string username) : _name(username), _life(20), _deckbuilt(false){
        //shuffle deck
        //initial draw
        /*do not allow game to start until deck is built. 
        Perhaps handle this when trying to start game, check deckbuilt existence for each player. 
        if not, direct to deckbuilder on interface for change deck get request
        */
    };

    User(string username, int decknum): _name(username), _life(20),  _deckbuilt(false){
        _buildDeck(decknum);
    }; // initialize prebuilt deck

    
    //user functions
    void changeDeck(int decknum){
        _deckbuilt = false;
        _buildDeck(decknum);
    }
    void _buildDeck(int _decknumber = -1){ // if decknumber emmitted, manually make your deck
        if(_deckbuilt==true){/*exit*/} // handle oopsies
        if(_decknumber == -1){
           // built deck manually
           //(user built screen)
           // (user clicks cards, signals back to put on top of stack)(handle limits)
        }
        else{
            /* return correct deck stack from somewhere ingame objects 
            (arr[decknum] = correct stack of cards ~ stack<Card*>prebuiltstack) or something*/
            
            // _deck = returned stack

        }
        
    };

    //initial draw cards
    void InitDraw(int mulligan=0){ 
        if(mulligan>=7){
            //make the fool stop somehow
        }
        for(int i = 0; i<(8-mulligan); i++){
            Draw();
            //wait a second or so ; perhaps for draw animation
            this_thread::sleep_for(chrono::seconds(1)); 
        }

    }
    //draw a card
    Card* Draw(){
        if(_deck.empty()){
            ShuffleGraveyardToHand();
        }
        if(_deck.empty()){
            return nullptr; // perhaps all exiled? eliminate player? lost()?
        }
        Card* toDraw = _deck.front();
        _deck.pop_front();
        _hand.insert(toDraw);
        return toDraw;
    };  

    void DeckShuffle(){
        random_device ran; //get your pseudo random num from device
        mt19937 g(ran()); // generate random seed

        std::shuffle(_deck.begin(), _deck.end(), g); // shuffle your deck
    }
    //shuffle graveyard into hand
    void ShuffleGraveyardToHand(){
        //swap full graveyard with empty hand
        _deck.swap(_graveyard);
        DeckShuffle();    
    }
    
    //play a card
    void Play(Card* card){
        if(card->mana <= _mana){
            //do card functionality defined somewhere in card or game manager idk
            _mana-=card->mana;
        }
        else{
            cout << "Not enough mana!" << endl; //replace with UI functionality stdin or something
        }
    }
    //declare blockers
    void declareBlocker(/*reference to opposing player field or something*/){
        //
    }

    //return top card of deck
    Card* Peek(){
        return _deck.front();
    }

    //place card at bottom of deck
    void placeBottom(Card* card){
        _deck.push_back(card);
    }
    //place a card at the top of the deck
    void placeTop(Card* card){
        _deck.push_front(card);
    }

    int TakeDamage(int amount){
        _life-=amount;
        if(_life <= 0 ){
            lost();
        }
    }
    int Heal(int amount){
        _life+=amount;
    }

    void lost(){
        //do some losing functionality using game manager or something idk
        //might delete this function
    }

};
