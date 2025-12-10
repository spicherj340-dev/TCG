// To avoid circular inclusion
// Card factory, will be used to make decks
    // We'll have a dataset with cards
#include <string>
class Card;
class User;

class CardFactory
{
public:
    static Card* createCard(const std::string& name, User* owner);
};