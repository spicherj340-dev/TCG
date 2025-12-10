#pragma once
#include "GameObjects.h"
#include <iostream>

class OProto : public OffensiveCard
{
public:
    OProto(User* owner)
    : OffensiveCard(owner, 1, "Attack Spell")
    {
        _desc = "Deal 1 damage to target.";
        _needsTarg = true;
    }

    void setTarget(User* target) override
    {
        _target = target;
    }

    void resolve() override
    {
        if (_target)
        {
            _target->takeDamage(1);
        }
    }
};

class DProto : public DefensiveCard
{
public:
    DProto(User* owner)
    : DefensiveCard(owner, 1, "Defense Spell")
    {
        _desc = "Block 1 damage.";
        _needsTarg = false;
    }

    void setTarget(User* target) override
    {
        _target = _owner;
    }

    void resolve() override
    {
        if (_target)
        {
            _target->gainLife(1);
        }
    }
};

class UProto : public UtilityCard
{
public:
    UProto(User* owner)
    : UtilityCard(owner, 1, "Utility Spell")
    {
        _desc = "Draw 1 card.";
        _needsTarg = false;
    }

    void setTarget(User* target) override
    {
        _target = _owner;
    }

    void resolve() override
    {
        if (_target)
        {
            _target->draw();
        }
    }
};