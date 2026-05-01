#pragma once
#include <memory>
#include <string>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Bridge Pattern
//
// Intent: Decouple an abstraction from its implementation so the two can vary
// independently.
//
// Real-world analogy: A remote control (abstraction) can work with any TV
// brand (implementation). The remote has basic and advanced variants; TVs have
// Sony and Samsung variants. All 4 combos work without a new class each.
//
// Key C++ mechanics used:
//  - The Abstraction holds a pointer to the Implementor (bridge/composition).
//  - The Abstraction's interface delegates to the Implementor.
//  - Both hierarchies extend independently.
//  - std::unique_ptr for ownership of the implementor.
//
// When to use:
//  - You want to avoid a permanent binding between abstraction & implementation.
//  - Both should be extensible via subclassing.
//  - Implementation changes should not affect client code.
//  - "Cartesian product" explosion: M abstractions × N implementations would
//    require M*N classes; Bridge needs only M + N.
// ---------------------------------------------------------------------------

namespace pattern {

// ---------- Implementor interface ----------------------------------------

class Device
{
public:
    virtual ~Device() = default;
    virtual bool        isEnabled()            const = 0;
    virtual void        enable()                     = 0;
    virtual void        disable()                    = 0;
    virtual int         volume()               const = 0;
    virtual void        setVolume(int percent)       = 0;
    virtual int         channel()              const = 0;
    virtual void        setChannel(int ch)           = 0;
    virtual std::string name()                 const = 0;
};

// ---------- Concrete Implementors -----------------------------------------

class TV : public Device
{
public:
    TV() : enabled_(false), volume_(30), channel_(1) {}

    bool        isEnabled()              const override { return enabled_; }
    void        enable()                       override { enabled_ = true;  }
    void        disable()                      override { enabled_ = false; }
    int         volume()                 const override { return volume_; }
    void        setVolume(int v)               override { volume_  = (v < 0 ? 0 : v > 100 ? 100 : v); }
    int         channel()                const override { return channel_; }
    void        setChannel(int ch)             override { channel_ = ch; }
    std::string name()                   const override { return "TV"; }

private:
    bool enabled_;
    int  volume_;
    int  channel_;
};

class Radio : public Device
{
public:
    Radio() : enabled_(false), volume_(50), channel_(1) {}

    bool        isEnabled()              const override { return enabled_; }
    void        enable()                       override { enabled_ = true;  }
    void        disable()                      override { enabled_ = false; }
    int         volume()                 const override { return volume_; }
    void        setVolume(int v)               override { volume_  = (v < 0 ? 0 : v > 100 ? 100 : v); }
    int         channel()                const override { return channel_; }
    void        setChannel(int ch)             override { channel_ = ch; }
    std::string name()                   const override { return "Radio"; }

private:
    bool enabled_;
    int  volume_;
    int  channel_;
};

// ---------- Abstraction ---------------------------------------------------

class RemoteControl
{
public:
    explicit RemoteControl(std::unique_ptr<Device> device)
        : device_(std::move(device)) {}

    virtual ~RemoteControl() = default;

    void togglePower()
    {
        if (device_->isEnabled()) device_->disable();
        else                      device_->enable();
    }

    void volumeDown() { device_->setVolume(device_->volume() - 10); }
    void volumeUp()   { device_->setVolume(device_->volume() + 10); }
    void channelDown(){ device_->setChannel(device_->channel() - 1); }
    void channelUp()  { device_->setChannel(device_->channel() + 1); }

    const Device& device() const { return *device_; }

protected:
    std::unique_ptr<Device> device_;
};

// ---------- Refined Abstraction -------------------------------------------

class AdvancedRemote : public RemoteControl
{
public:
    explicit AdvancedRemote(std::unique_ptr<Device> device)
        : RemoteControl(std::move(device)) {}

    void mute()    { device_->setVolume(0); }
    void setChannel(int ch) { device_->setChannel(ch); }
};

} // namespace pattern
