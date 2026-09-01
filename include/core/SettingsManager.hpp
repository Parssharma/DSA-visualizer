#ifndef SETTINGS_MANAGER_HPP
#define SETTINGS_MANAGER_HPP

#include "core/ThemeManager.hpp"

namespace core {

class SettingsManager {
public:
    static SettingsManager& getInstance() {
        static SettingsManager instance;
        return instance;
    }

    SettingsManager(const SettingsManager&) = delete;
    SettingsManager& operator=(const SettingsManager&) = delete;

    ThemeType getTheme() const { return m_theme; }
    void setTheme(ThemeType theme) {
        m_theme = theme;
        ThemeManager::getInstance().setTheme(theme);
    }

    float getSpeedMultiplier() const { return m_speedMultiplier; }
    void setSpeedMultiplier(float mult) { m_speedMultiplier = mult; }

    int getDefaultArraySize() const { return m_defaultArraySize; }
    void setDefaultArraySize(int size) { m_defaultArraySize = size; }

    bool isSoundEnabled() const { return m_soundEnabled; }
    void setSoundEnabled(bool enabled) { m_soundEnabled = enabled; }

    float getFontScale() const { return m_fontScale; }
    void setFontScale(float scale) { m_fontScale = scale; }

private:
    SettingsManager() 
        : m_theme(ThemeType::Dark),
          m_speedMultiplier(1.f),
          m_defaultArraySize(50),
          m_soundEnabled(true),
          m_fontScale(1.f) {}
    ~SettingsManager() = default;

    ThemeType m_theme;
    float m_speedMultiplier;
    int m_defaultArraySize;
    bool m_soundEnabled;
    float m_fontScale;
};

} // namespace core

#endif // SETTINGS_MANAGER_HPP
