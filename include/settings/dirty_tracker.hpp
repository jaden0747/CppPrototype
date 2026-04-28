#pragma once

#include <cstdint>

/// ---------------------------------------------------------------------------
/// DirtyTracker
///
/// Mixin base class. Settings structs that need change detection should
/// inherit from this. SettingsRegistry calls dirty() after each load.
/// ---------------------------------------------------------------------------
class DirtyTracker
{
public:
    void dirty()
    {
        ++m_modifiedCount;
        onChanged();
    }

    uint32_t getModifiedCount() const
    {
        return m_modifiedCount;
    }

protected:
    virtual void onChanged() {}
    ~DirtyTracker() = default;

private:
    uint32_t m_modifiedCount{0U};
};
