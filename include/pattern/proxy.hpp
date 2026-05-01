#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Proxy Pattern
//
// Intent: Provide a surrogate or placeholder for another object to control
// access to it.
//
// Three common variants shown:
//   1. Virtual Proxy — lazy initialisation (expensive resource loaded on demand)
//   2. Protection Proxy — access control (role-based)
//   3. Caching Proxy — caches results of expensive calls
//
// Real-world analogy:
//   - Virtual: a thumbnail image proxy loads the full image only when viewed.
//   - Protection: an employee proxy checks permissions before accessing data.
//   - Caching: a web proxy caches web pages so repeated requests are fast.
//
// Key C++ mechanics used:
//  - All proxies implement the same interface as the real subject.
//  - Proxy holds a pointer to (or creates on demand) the real subject.
//  - std::unique_ptr for ownership; std::unordered_map for caching.
//
// When to use:
//  - Lazy initialisation (virtual proxy).
//  - Logging / instrumentation (logging proxy).
//  - Access control (protection proxy).
//  - Caching (caching proxy).
//  - Remote access (remote proxy — not shown here).
// ---------------------------------------------------------------------------

namespace pattern {

// ---------- Subject interface --------------------------------------------

class Image
{
public:
    virtual ~Image() = default;
    virtual void    display()                    = 0;
    virtual std::string name() const             = 0;
    virtual bool    isLoaded() const             = 0;
};

// ---------- Real Subject -------------------------------------------------

class RealImage : public Image
{
public:
    explicit RealImage(const std::string& filename)
        : filename_(filename), loaded_(false)
    {
        load();  // simulates expensive disk I/O
    }

    void display() override { /* render pixel data */ }
    std::string name() const override { return filename_; }
    bool isLoaded()    const override { return loaded_; }

private:
    void load() { loaded_ = true; }
    std::string filename_;
    bool        loaded_;
};

// ---------- 1. Virtual Proxy (lazy init) ---------------------------------

class LazyImageProxy : public Image
{
public:
    explicit LazyImageProxy(const std::string& filename)
        : filename_(filename), real_(nullptr) {}

    void display() override
    {
        if (!real_)
            real_ = std::unique_ptr<RealImage>(new RealImage(filename_));
        real_->display();
    }

    std::string name()     const override { return filename_; }
    bool        isLoaded() const override { return real_ != nullptr && real_->isLoaded(); }

private:
    std::string                filename_;
    std::unique_ptr<RealImage> real_;
};

// ---------- 2. Protection Proxy -----------------------------------------

enum class Role { Guest, User, Admin };

class Service
{
public:
    virtual ~Service() = default;
    virtual std::string getData() const  = 0;
    virtual bool        deleteData()     = 0;
};

class RealService : public Service
{
public:
    std::string getData()    const override { return "Sensitive data"; }
    bool        deleteData()       override { return true; }
};

class ProtectionProxy : public Service
{
public:
    ProtectionProxy(std::unique_ptr<Service> real, Role role)
        : real_(std::move(real)), role_(role) {}

    std::string getData() const override
    {
        if (role_ == Role::Guest)
            throw std::runtime_error("Access denied: guests cannot read data");
        return real_->getData();
    }

    bool deleteData() override
    {
        if (role_ != Role::Admin)
            throw std::runtime_error("Access denied: only admins can delete");
        return real_->deleteData();
    }

private:
    std::unique_ptr<Service> real_;
    Role role_;
};

// ---------- 3. Caching Proxy --------------------------------------------

class DataSource
{
public:
    virtual ~DataSource() = default;
    virtual std::string fetch(const std::string& key) const = 0;
};

class SlowDataSource : public DataSource
{
public:
    mutable int fetchCount = 0;
    std::string fetch(const std::string& key) const override
    {
        ++fetchCount;
        return "value_of_" + key;
    }
};

class CachingProxy : public DataSource
{
public:
    explicit CachingProxy(std::unique_ptr<DataSource> real)
        : real_(std::move(real)) {}

    std::string fetch(const std::string& key) const override
    {
        auto it = cache_.find(key);
        if (it != cache_.end())
            return it->second;

        std::string result = real_->fetch(key);
        cache_[key] = result;
        return result;
    }

    std::size_t cacheSize() const { return cache_.size(); }

private:
    std::unique_ptr<DataSource>                  real_;
    mutable std::unordered_map<std::string, std::string> cache_;
};

} // namespace pattern
