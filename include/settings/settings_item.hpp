#pragma once

#include "settings/settings_registry.hpp"

#include <string>

/// ---------------------------------------------------------------------------
/// SettingsItem<T>
///
/// Wraps a settings struct T, registers it in SettingsRegistry under the
/// given string key at construction time.
/// Replaces pc::util::coding::Item<T>.
///
/// Use as a file-scope or namespace-scope variable:
///   SettingsItem<MySettings> g_mySettings("MyFeature");
/// ---------------------------------------------------------------------------
template <typename T>
class SettingsItem
{
public:
    explicit SettingsItem(const std::string& name)
        : m_data{}
    {
        SettingsRegistry::instance().add(name, &m_data);
    }

    ~SettingsItem() = default;

    const T* operator->() const&
    {
        return &m_data;
    }
    T* operator->() &
    {
        return &m_data;
    }

    const T& data() const&
    {
        return m_data;
    }
    T& data() &
    {
        return m_data;
    }

    const T* operator->() const&& = delete;
    T*       operator->() &&      = delete;

    const T& data() const&& = delete;
    T&       data() &&      = delete;

private:
    SettingsItem(const SettingsItem&)            = delete;
    SettingsItem& operator=(const SettingsItem&) = delete;

    T m_data;
};
