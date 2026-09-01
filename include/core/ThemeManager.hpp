#ifndef THEME_MANAGER_HPP
#define THEME_MANAGER_HPP

#include <SFML/Graphics/Color.hpp>

namespace core {

enum class ThemeType {
    Dark,
    Light,
    HighContrast
};

struct ThemeColors {
    sf::Color background;
    sf::Color panelBg;
    sf::Color cardBg;
    sf::Color textPrimary;
    sf::Color textSecondary;
    sf::Color accent;
    sf::Color accentHover;
    
    // Status colors
    sf::Color success;
    sf::Color danger;
    sf::Color warning;
    sf::Color info;
    
    // Algorithm visualizer specific colors
    sf::Color barDefault;
    sf::Color barCompare;
    sf::Color barSwap;
    sf::Color barSorted;
    sf::Color barActive;
    
    sf::Color border;
};

class ThemeManager {
public:
    static ThemeManager& getInstance() {
        static ThemeManager instance;
        return instance;
    }

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void setTheme(ThemeType type);
    ThemeType getThemeType() const { return m_currentType; }
    const ThemeColors& getColors() const { return m_colors; }

private:
    ThemeManager();
    ~ThemeManager() = default;

    ThemeType m_currentType;
    ThemeColors m_colors;
    
    void loadDarkTheme();
    void loadLightTheme();
    void loadHighContrastTheme();
};

} // namespace core

#endif // THEME_MANAGER_HPP
