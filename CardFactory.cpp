#include "CardFactory.h"
#include "GameObjects.h"
#include "Cards.h"

// Card Factory
Card* CardFactory::createCard(const string& name, User* owner)
{
    if (name == "OProto") return new OProto(owner);
    if (name == "DProto") return new DProto(owner);
    if (name == "UProto") return new UProto(owner);
    return nullptr;
}