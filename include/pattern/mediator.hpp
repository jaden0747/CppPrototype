#pragma once
#include <string>
#include <vector>
#include <any>
#include <functional>
#include <unordered_map>


// ---------------------------------------------------------------------------
// Mediator Pattern
//
// Intent: Define an object that encapsulates how a set of objects interact.
// Mediator promotes loose coupling by keeping objects from referring to each
// other explicitly, and allows you to vary their interaction independently.
//
// Real-world analogy: An air traffic control (ATC) tower. Planes don't
// communicate directly with each other — they all talk to the tower, which
// coordinates landings, take-offs, and taxiing.
//
// Key C++ mechanics used:
//  - Abstract Mediator interface so components don't depend on a concrete class.
//  - Components hold a pointer to the Mediator (not to each other).
//  - Mediator stores references/pointers to all components.
//  - Messages routed through notify() — components publish; mediator routes.
//
// When to use:
//  - A set of objects communicate in complex, tightly-coupled ways.
//  - You want to reuse a component in a different context.
//  - Interaction logic is distributed across many classes and needs
//    centralisation (think: form validation, GUI widget coordination).
// ---------------------------------------------------------------------------

namespace pattern
{

class Component;

// ---------- Mediator interface --------------------------------------------

class Mediator
{
public:
    virtual ~Mediator()                                              = default;
    virtual void notify(Component* sender, const std::string& event) = 0;
};

// ---------- Component base -----------------------------------------------

class Component
{
public:
    explicit Component(const std::string& name)
        : name_(name)
        , mediator_(nullptr)
    {
    }
    virtual ~Component() = default;

    void setMediator(Mediator* m)
    {
        mediator_ = m;
    }
    const std::string& name() const
    {
        return name_;
    }

protected:
    void trigger(const std::string& event)
    {
        if (mediator_)
            mediator_->notify(this, event);
    }

    std::string name_;
    Mediator*   mediator_;
};

// ---------- Concrete Components (chat room participants) -----------------

class ChatUser : public Component
{
public:
    explicit ChatUser(const std::string& name)
        : Component(name)
    {
    }

    void send(const std::string& message)
    {
        lastSent_ = message;
        trigger("message:" + message);
    }

    void receive(const std::string& from, const std::string& message)
    {
        inbox_.push_back("[" + from + "]: " + message);
    }

    const std::string& lastSent() const
    {
        return lastSent_;
    }
    const std::vector<std::string>& inbox() const
    {
        return inbox_;
    }
    void clearInbox()
    {
        inbox_.clear();
    }

private:
    std::string              lastSent_;
    std::vector<std::string> inbox_;
};

// ---------- Concrete Mediator (chat room) --------------------------------

class ChatRoom : public Mediator
{
public:
    void addUser(ChatUser* user)
    {
        users_.push_back(user);
        user->setMediator(this);
    }

    void notify(Component* sender, const std::string& event) override
    {
        // parse "message:<text>"
        const std::string prefix = "message:";
        if (event.rfind(prefix, 0) != 0)
            return;

        std::string text     = event.substr(prefix.size());
        ChatUser*   fromUser = static_cast<ChatUser*>(sender);

        for (auto* u : users_)
        {
            if (u != fromUser)
                u->receive(fromUser->name(), text);
        }
    }

    std::size_t userCount() const
    {
        return users_.size();
    }

private:
    std::vector<ChatUser*> users_;
};

// Event payload — flexible key/value bag
struct GameEvent
{
    std::string                               type;
    std::unordered_map<std::string, std::any> data;
};

using EventHandler = std::function<void(const GameEvent&)>;

class GameEventBus
{
public:
    static GameEventBus& Get()
    {
        static GameEventBus instance;
        return instance;
    }

    void Subscribe(const std::string& eventType, EventHandler handler)
    {
        listeners_[eventType].push_back(std::move(handler));
    }

    void Publish(const GameEvent& event)
    {
        auto it = listeners_.find(event.type);
        if (it != listeners_.end())
        {
            for (auto& handler : it->second)
                handler(event);
        }
    }

private:
    std::unordered_map<std::string, std::vector<EventHandler>> listeners_;
};

// ----- Physics: publishes a collision event -----
class PhysicsSystem
{
public:
    void OnCollision(const std::string& objA, const std::string& objB)
    {
        GameEventBus::Get().Publish({"collision", {{"objectA", objA}, {"objectB", objB}}});
    }
};

// ----- GameState: listens and updates score -----
class GameState
{
public:
    GameState()
    {
        GameEventBus::Get().Subscribe(
            "collision",
            [this](const GameEvent& e)
            {
                auto objA = std::any_cast<std::string>(e.data.at("objectA"));
                if (objA == "bullet")
                {
                    score_ += 10;
                    GameEventBus::Get().Publish({"score_changed", {{"score", score_}}});
                }
            });
    }

private:
    int score_ = 0;
};

// ----- UI: listens for score changes -----
class UISystem
{
public:
    UISystem()
    {
        GameEventBus::Get().Subscribe(
            "score_changed",
            [](const GameEvent& e)
            {
                int score = std::any_cast<int>(e.data.at("score"));
                // Re-draw the HUD score display
            });
    }
};

// ----- Audio: listens for collision to play SFX -----
class AudioSystem
{
public:
    AudioSystem()
    {
        GameEventBus::Get().Subscribe(
            "collision",
            [](const GameEvent& e)
            {
                // Play impact sound
            });
    }
};

} // namespace pattern
