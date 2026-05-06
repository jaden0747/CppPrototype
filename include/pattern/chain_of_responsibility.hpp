#pragma once
#include <functional>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// Chain of Responsibility Pattern
//
// Intent: Avoid coupling the sender of a request to its receiver by giving
// more than one object a chance to handle the request. Chain the receiving
// objects and pass the request along the chain until an object handles it.
//
// Real-world analogy: A technical support escalation system.
// A Tier-1 agent handles basic questions; if stumped, escalates to Tier-2;
// if still unsolved, escalates to Tier-3 engineering.
//
// Key C++ mechanics used:
//  - Abstract Handler with a `setNext()` and `handle()` interface.
//  - Concrete handlers decide whether to process or pass along.
//  - Chaining via unique_ptr; handler owns its successor.
//  - Request passed as a value type (struct with severity level).
//
// When to use:
//  - More than one object may handle a request, and the handler isn't known
//    a priori.
//  - You want to issue a request without specifying the receiver explicitly.
//  - The set of objects that can handle the request should be specified
//    dynamically.
// ---------------------------------------------------------------------------

namespace pattern
{

struct SupportRequest
{
    int         level; // 1 = basic, 2 = intermediate, 3 = advanced
    std::string description;
};

// ---------- Abstract Handler ---------------------------------------------

class SupportHandler
{
public:
    virtual ~SupportHandler() = default;

    SupportHandler* setNext(std::unique_ptr<SupportHandler> next)
    {
        next_ = std::move(next);
        return next_.get();
    }

    virtual std::string handle(const SupportRequest& req) = 0;

protected:
    std::string passToNext(const SupportRequest& req)
    {
        if (next_)
            return next_->handle(req);
        return "Unhandled: " + req.description;
    }

    std::unique_ptr<SupportHandler> next_;
};

// ---------- Concrete Handlers --------------------------------------------

class Tier1Handler : public SupportHandler
{
public:
    std::string handle(const SupportRequest& req) override
    {
        if (req.level == 1)
            return "Tier1 handled: " + req.description;
        return passToNext(req);
    }
};

class Tier2Handler : public SupportHandler
{
public:
    std::string handle(const SupportRequest& req) override
    {
        if (req.level == 2)
            return "Tier2 handled: " + req.description;
        return passToNext(req);
    }
};

class Tier3Handler : public SupportHandler
{
public:
    std::string handle(const SupportRequest& req) override
    {
        if (req.level == 3)
            return "Tier3 handled: " + req.description;
        return passToNext(req);
    }
};

// ---------- Helper: build a default chain --------------------------------
// Returns the head (Tier1 → Tier2 → Tier3)
inline std::unique_ptr<SupportHandler> buildDefaultChain()
{
    auto t1 = std::unique_ptr<SupportHandler>(new Tier1Handler());
    auto t2 = std::unique_ptr<SupportHandler>(new Tier2Handler());
    auto t3 = std::unique_ptr<SupportHandler>(new Tier3Handler());
    t2->setNext(std::move(t3));
    t1->setNext(std::move(t2));
    return t1;
}

struct InputEvent
{
    enum class Type
    {
        KeyPress,
        MouseClick,
        GamepadButton
    };
    Type type;
    int  keyCode;
    bool handled = false;
};

class Player
{
public:
    bool isGrounded() const
    {
        return true;
    }

    void jump()
    {
    }
};

class InputHandler
{
public:
    virtual ~InputHandler() = default;

    // Set the next handler in the chain
    InputHandler* setNext(std::unique_ptr<InputHandler> next)
    {
        m_next = std::move(next);
        return m_next.get();
    }

    // Each handler decides to handle or pass along
    virtual void handle(InputEvent& event)
    {
        if (m_next && !event.handled)
            m_next->handle(event);
    }

protected:
    std::unique_ptr<InputHandler> m_next;
};

// UIHandler — highest priority, eats input when menus are open
class UIHandler : public InputHandler
{
public:
    void handle(InputEvent& event) override
    {
        if (m_menuOpen && event.type == InputEvent::Type::KeyPress)
        {
            // Consume the event — don't pass down the chain
            event.handled = true;
            processMenuInput(event.keyCode);
            return;
        }
        InputHandler::handle(event); // pass along
    }

    void setMenuOpen(bool open)
    {
        m_menuOpen = open;
    }

private:
    bool m_menuOpen = false;

    void processMenuInput(int key)
    {
        // Navigate menu items, confirm selection, etc.
    }
};

// AbilityHandler — checks if a key is bound to a skill
class AbilityHandler : public InputHandler
{
public:
    void handle(InputEvent& event) override
    {
        if (event.type == InputEvent::Type::KeyPress)
        {
            auto it = m_abilityBindings.find(event.keyCode);
            if (it != m_abilityBindings.end())
            {
                activateAbility(it->second);
                event.handled = true;
                return;
            }
        }
        InputHandler::handle(event);
    }

    void bindAbility(int keyCode, int abilityId)
    {
        m_abilityBindings[keyCode] = abilityId;
    }

private:
    std::unordered_map<int, int> m_abilityBindings;

    void activateAbility(int id)
    { /* trigger ability system */
    }
};

// MovementHandler — jump, crouch, dodge
class MovementHandler : public InputHandler
{
public:
    void handle(InputEvent& event) override
    {
#define KEY_SPACE 32
        if (event.type == InputEvent::Type::KeyPress && event.keyCode == KEY_SPACE)
        {
            if (m_player->isGrounded())
            {
                m_player->jump();
                event.handled = true;
                return;
            }
        }
        InputHandler::handle(event);
    }

    void setPlayer(Player* player)
    {
        m_player = player;
    }

private:
    Player* m_player = nullptr;
};

// DefaultHandler — catches anything unhandled for logging/debug
class DefaultHandler : public InputHandler
{
public:
    void handle(InputEvent& event) override
    {
        if (!event.handled)
            logUnhandledInput(event);
        // End of chain — don't call InputHandler::handle()
    }

private:
    void logUnhandledInput(const InputEvent& e)
    {
        // Debug output
    }
};

// GameInputSystem.cpp
class GameInputSystem
{
public:
    void buildChain(Player* player)
    {
        // Build from the bottom up — ownership flows through unique_ptr
        auto defaultH  = std::make_unique<DefaultHandler>();
        auto movementH = std::make_unique<MovementHandler>();
        auto abilityH  = std::make_unique<AbilityHandler>();
        auto uiH       = std::make_unique<UIHandler>();

        constexpr int KEY_Q            = 81;
        constexpr int KEY_E            = 69;
        constexpr int ABILITY_FIREBALL = 1;
        constexpr int ABILITY_SHIELD   = 2;
        movementH->setPlayer(player);
        abilityH->bindAbility(KEY_Q, ABILITY_FIREBALL);
        abilityH->bindAbility(KEY_E, ABILITY_SHIELD);

        // Wire the chain: UI → Ability → Movement → Default
        movementH->setNext(std::move(defaultH));
        abilityH->setNext(std::move(movementH));
        uiH->setNext(std::move(abilityH));

        m_chainHead = std::move(uiH);
    }

    void onInput(InputEvent event)
    {
        if (m_chainHead)
            m_chainHead->handle(event);
    }

private:
    std::unique_ptr<InputHandler> m_chainHead;
};

} // namespace pattern
