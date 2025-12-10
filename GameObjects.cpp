#include "GameObjects.h"
#include "CardFactory.h"
#include "GameManager.h"
#include <iostream>
#include <algorithm>
#include <random>
using namespace std;

// Zone
// Move card from current zone to another
bool Zone::moveCard(Card* c, Zone* dest)
{
// Remove the card from the zone

    Card* card = removeCard(c);
    // If card doesnt exist
    if (!card) return false;

    // Else add the card to dest zone
    dest->addCard(card);
    return true;
}

// Remove specific card from zone
Card* Zone::removeCard(Card* card)
{
    // Create an iterator to search        
    for (auto k = _cards.begin(); k != _cards.end(); k++)
    {
        if (*k == card)
        {
            // Remove card at k then return it    
            _cards.erase(k);
            return card;    
        }
    }
    // Card not found, return null ptr
    return nullptr;
}

// Add card to zone
void Zone::addCard(Card* card)
{
     _cards.push_back(card);   
}

// Used at end of game to clean up card objects
void Zone::cleanup()
{
    // Delete all the cards
    for (Card* c : _cards)
        delete c;
    
    // Clear the card vector
    _cards.clear();
}

// Deck
// Draw top card of deck
Card* Deck::drawCard()
{
    // If empty, nullptr (can be used for shuffle back logic)    
    if (_cards.empty()) return nullptr;   
    
    // Return top card
    Card* card = _cards.back();
    _cards.pop_back();
    return card;
}

void Deck::shuffle()
{
   // If no cards, don't even try     
   if (_cards.empty()) return;

   // Shuffle cards
   static std::random_device rd;
   static std::mt19937 g(rd());
   std::shuffle(_cards.begin(), _cards.end(), g);
}

// Hand
// Play card from hand
Card* Hand::playCard(Card* card)
{
    // Playing logic handled by User
    // If moving fails for some reason return nullptr
    if (moveCard(card, _owner->getField())) return card;
    return nullptr;

}
// Get visible cards from hand
vector<Card*> Hand::getVisibleCards(User* user) const
{
    // Only the owner may see their hand    
    if (user == _owner) return _cards;
    // Else empty vector
    return {};
}

// Field and Graveyard aren't really special apart from visibility, game manager should clean

// User
User::User(const string& name)
:_name(name), _life(20)
{
    string s;
    _deck = new Deck(this);
    _hand = new Hand(this);
    _field = new Field(this);
    _graveyard = new Graveyard(this);


    // TEMPORARY, we will need a new deck building system later
    for(int i = 0; i < 20; i++)
    {    cin >> s;
        _deck->addCard(CardFactory::createCard(s, this));
    }
}

Card* User::draw()
{
    Card* card = _deck->drawCard();
    
    // If deck is empty, shuffle gy into deck
    if (card == nullptr)
    {
        // Prevent weird behavior if graveyard is somehow empty
        if (_graveyard->getAllCards().size() == 0) return nullptr;
        
        // Move graveyard to deck
        // We can't iterate over graveyard itself so we must make a copy
        auto graveCards = _graveyard->getAllCards();
        // Now just move all cards from graveyard to deck
        for (Card* card : graveCards)
        {
            _graveyard->moveCard(card, _deck);    
        }
    
    // Shuffle the deck
    _deck->shuffle();

    // Draw again
    card = _deck->drawCard();
    }
    
    // Add to hand
    _hand->addCard(card);

    return card;
}

bool User::play(Card* c)
{
    // Weird nullptr edge case    
    if (!c) return false;

    int cost = c->getCost();
    // If we cant afford the card we cant play the card
    if (_charge < cost) return false;

    // Play the card
    _charge -= cost;
    // Not sure how to set a target yet
    // Play if able
        // Play now handled by gameManager
    GameManager::getInstance()->stackPush(c);
    
    return true;
}

void User::shuffle()
{
    _deck->shuffle();    
}

// Destructor must delete owned objects
User::~User()
{
    delete _deck;
    delete _graveyard;
    delete _hand;
    delete _field;
}

