#pragma once
#include <memory>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// State Pattern
//
// Intent: Allow an object to alter its behaviour when its internal state
// changes. The object will appear to change its class.
//
// Real-world analogy: A vending machine. Inserting a coin, selecting a
// product, and dispensing it are all valid actions — but only in the right
// order. Each state captures which transitions are legal.
//
// Key C++ mechanics used:
//  - State interface with one method per action.
//  - Context holds a std::unique_ptr<State> and delegates all action calls.
//  - Concrete states call context->setState(...) to trigger transitions,
//    keeping transition logic close to the state that initiates it.
//
// When to use:
//  - An object's behaviour depends on its state and must change at run time.
//  - Operations have large, multipart conditionals that depend on the
//    object's state (replace with polymorphism).
// ---------------------------------------------------------------------------

namespace pattern
{

class VendingMachine; // forward declaration

// ---------- State interface ----------------------------------------------

class VendingState
{
public:
    virtual ~VendingState()                               = default;
    virtual void        insertCoin(VendingMachine& vm)    = 0;
    virtual void        selectProduct(VendingMachine& vm) = 0;
    virtual void        dispense(VendingMachine& vm)      = 0;
    virtual std::string name() const                      = 0;
};

// ---------- Concrete States ----------------------------------------------

class IdleState : public VendingState
{
public:
    void        insertCoin(VendingMachine& vm) override;
    void        selectProduct(VendingMachine& vm) override;
    void        dispense(VendingMachine& vm) override;
    std::string name() const override
    {
        return "Idle";
    }
};

class HasCoinState : public VendingState
{
public:
    void        insertCoin(VendingMachine& vm) override;
    void        selectProduct(VendingMachine& vm) override;
    void        dispense(VendingMachine& vm) override;
    std::string name() const override
    {
        return "HasCoin";
    }
};

class DispensingState : public VendingState
{
public:
    void        insertCoin(VendingMachine& vm) override;
    void        selectProduct(VendingMachine& vm) override;
    void        dispense(VendingMachine& vm) override;
    std::string name() const override
    {
        return "Dispensing";
    }
};

class OutOfStockState : public VendingState
{
public:
    void        insertCoin(VendingMachine& vm) override;
    void        selectProduct(VendingMachine& vm) override;
    void        dispense(VendingMachine& vm) override;
    std::string name() const override
    {
        return "OutOfStock";
    }
};

// ---------- Context -------------------------------------------------------

class VendingMachine
{
public:
    explicit VendingMachine(int stock)
        : stock_(stock)
    {
        if (stock_ > 0)
            setState(std::unique_ptr<VendingState>(new IdleState()));
        else
            setState(std::unique_ptr<VendingState>(new OutOfStockState()));
    }

    void setState(std::unique_ptr<VendingState> s)
    {
        state_ = std::move(s);
    }

    void insertCoin()
    {
        state_->insertCoin(*this);
    }
    void selectProduct()
    {
        state_->selectProduct(*this);
    }
    void dispense()
    {
        state_->dispense(*this);
    }

    std::string stateName() const
    {
        return state_->name();
    }

    int stock() const
    {
        return stock_;
    }
    void decreaseStock()
    {
        if (stock_ > 0)
            --stock_;
    }

private:
    std::unique_ptr<VendingState> state_;
    int                           stock_;
};

// ---------- State implementations ----------------------------------------

inline void IdleState::insertCoin(VendingMachine& vm)
{
    vm.setState(std::unique_ptr<VendingState>(new HasCoinState()));
}
inline void IdleState::selectProduct(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Insert coin first");
}
inline void IdleState::dispense(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Insert coin first");
}

inline void HasCoinState::insertCoin(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Coin already inserted");
}
inline void HasCoinState::selectProduct(VendingMachine& vm)
{
    vm.setState(std::unique_ptr<VendingState>(new DispensingState()));
}
inline void HasCoinState::dispense(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Select a product first");
}

inline void DispensingState::insertCoin(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Please wait, dispensing");
}
inline void DispensingState::selectProduct(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Already dispensing");
}
inline void DispensingState::dispense(VendingMachine& vm)
{
    vm.decreaseStock();
    if (vm.stock() > 0)
        vm.setState(std::unique_ptr<VendingState>(new IdleState()));
    else
        vm.setState(std::unique_ptr<VendingState>(new OutOfStockState()));
}

inline void OutOfStockState::insertCoin(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Out of stock");
}
inline void OutOfStockState::selectProduct(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Out of stock");
}
inline void OutOfStockState::dispense(VendingMachine& vm)
{
    (void)vm;
    throw std::logic_error("Out of stock");
}

} // namespace pattern
