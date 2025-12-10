#ifndef GAME_OBJECTS_H
#define GAME_OBJECTS_H
#include <string>
#include <vector>
using namespace std;

// Forward declarations, since there is circular dependency between all types
    // Card has a User owner, but User has Card functions and so on
class Zone;
class Card;
class Deck;
class Hand;
class Field;
class Graveyard;
class CardFactory;
class User;

// Cards
class Card
{
protected:
    // Protected constructor so that we can't try and instantiate Card
        // However, descendants can inherit this
        // ALL cards will have an owner, name, cost, and target at the very least
    User* _owner;
    User* _target;
    bool _needsTarg;
    int _cost;
    string _name;

    // All cards will also have a description, but we'll inherit that later
    string _desc;

    // Since target is unknown at creation, instantiate _target to null ptr
    Card(User* o, int c, const string& n)
    : _owner(o), _cost(c), _name(n), _target(nullptr), _desc("") {};

    // I also want cards to load their stats and stuff from some other file that we could have
        // Would allow us to just put values in instead of having to code every card manually

public:
    // Basic setters
    virtual void setTarget(User* target) = 0;

    // Basic getters
    virtual string getName() const {return _name;}
    virtual int getCost() const {return _cost;}
    virtual User* getOwner() const {return _owner;}
    virtual User* getTarget() const {return _target;}
    virtual string getDesc() const {return _desc;}
    virtual bool needsTarget() const {return _needsTarg;}

    // Return card type
    virtual char getType() const = 0;

    // Resolve card
    virtual void resolve() = 0;

    // Virtual destructor
    virtual ~Card() {}
};

// The three horsemen of the card types

class OffensiveCard : public Card
{
protected:
    // Initialize as a card with o, c, n
    // Build as a card
    OffensiveCard(User* o, int c, const string& n)
        : Card(o, c, n){}
public:
    char getType() const override {return 'O';}
};

class DefensiveCard : public Card
{
protected:
    // Initialize as a card with o, c, n
    // Build as a card
    DefensiveCard(User* o, int c, const string& n)
        : Card(o, c, n){}
public:
    char getType() const override {return 'D';}
};

class UtilityCard : public Card
{
protected:
    // Initialize as a card with o, c, n
    // Build as a card
    UtilityCard(User* o, int c, const string& n)
        : Card(o, c, n){}
public:
    char getType() const override {return 'U';}
    
};

// Zones
class Zone
{
protected:
    User* _owner;
    vector<Card*> _cards;
    // No constructor - these things will be way too varied

public:
    // Add/remove card from zone
    // This should be pretty universal
        // In implementation file
    virtual void addCard(Card* card);
    virtual Card* removeCard(Card* card);

    // Move card from zone to zone
        // In implementation file
    virtual bool moveCard(Card* c, Zone* dest);

    // Get size of zone
    virtual size_t size() const {return _cards.size();}

    // Get cards user can see
    virtual vector<Card*> getVisibleCards(User* user) const = 0;

    // Get ALL CARDS (should ONLY be used for special purposes)
    virtual vector<Card*> getAllCards() const {return _cards;}

    // Clean up all cards in the zone
    void cleanup();

    // Virtual destructor
    virtual ~Zone() {}

};

class Deck : public Zone
{
public:
    Deck(User* owner) {_owner = owner;}

    // Draw top card of deck
    Card* drawCard();

    // Get visible cards for user, empty vector because no one can see deck normally
    vector<Card*> getVisibleCards(User* user) const override {return {};};

    void shuffle();

};

class Hand : public Zone
{
public:
    Hand(User* owner) {_owner = owner;}

    vector<Card*> getVisibleCards(User* user) const override;

    Card* playCard(Card* card);
};

class Field : public Zone
{
public:
    Field(User* owner) {_owner = owner;}

    // Publicly visible
    vector<Card*> getVisibleCards(User* user) const override {return _cards;};
};

class Graveyard : public Zone
{
public:
    Graveyard(User* owner) {_owner = owner;}

    // Publicly visible
    vector<Card*> getVisibleCards(User* user) const override {return _cards;};
};

// User at bottom since it wants everything else to exist first
class User
{
private:
    // User attributes
    string _name;
    int _life;
    int _charge;
    // User's zones
    Deck* _deck;
    Hand* _hand;
    Field* _field;
    Graveyard* _graveyard;

    // Card factory which I will figure out at some other time
    CardFactory* _CF;

public:

    // Not sure how to handle loading in a deck
        // Will make cardfactory class later
    User(const string& name);

    // Getters
    // Zone getters probably shouldn't be used unless necessary, we'll have more refined ones
    // Having all be Zone* allows for easier use of general zone methods
        // ie I was having trouble in the implementation file and this was an easy fix
    Zone* getDeck() const {return _deck;}
    Zone* getHand() const {return _hand;}
    Zone* getField() const {return _field;}
    Zone* getGraveyard() const {return _graveyard;}

    // Other getters
    string getName() const {return _name;}
    int getLife() const {return _life;}
    int getCharge() const {return _charge;}

    // Affect values
    void takeDamage(int amt) {_life -= amt;}
    void gainLife(int amt) {_life += amt;}
    void useCharge(int amt) {_charge -= amt;}
    void zeroCharge() {_charge = 0;}
    void gainCharge(int amt) {_charge += amt;}

    // Methods
    // Draw a card
    Card* draw();
    // Play a card
    bool play(Card* c);

    // Shuffle deck
    void shuffle();

    // Destructor
        // Will need to define it since we have a LOT of other objects here
    ~User();
};


#endif