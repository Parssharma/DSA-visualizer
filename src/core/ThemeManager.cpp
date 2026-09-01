#include "core/ThemeManager.hpp"

namespace core {

ThemeManager::ThemeManager() : m_currentType(ThemeType::Dark) {
    loadDarkTheme();
}

void ThemeManager::setTheme(ThemeType type) {
    m_currentType = type;
    if (type == ThemeType::Dark) {
        loadDarkTheme();
    } else if (type == ThemeType::Light) {
        loadLightTheme();
    } else {
        loadHighContrastTheme();
    }
}

void ThemeManager::loadDarkTheme() {
    // Sleek Dark Theme
    m_colors.background = sf::Color(18, 18, 24);         // Deep dark blue-gray
    m_colors.panelBg = sf::Color(26, 26, 36);            // Slightly lighter panel bg
    m_colors.cardBg = sf::Color(34, 34, 48);             // Lighter card bg
    m_colors.textPrimary = sf::Color(245, 245, 250);     // Clean white
    m_colors.textSecondary = sf::Color(160, 160, 180);   // Cool gray
    m_colors.accent = sf::Color(99, 102, 241);           // Indigo-500
    m_colors.accentHover = sf::Color(79, 70, 229);       // Indigo-600
    
    // Status colors
    m_colors.success = sf::Color(16, 185, 129);          // Emerald-500
    m_colors.danger = sf::Color(239, 68, 68);            // Red-500
    m_colors.warning = sf::Color(245, 158, 11);          // Amber-500
    m_colors.info = sf::Color(59, 130, 246);             // Blue-500
    
    // Visualizer specific
    m_colors.barDefault = sf::Color(99, 102, 241, 180);   // Translucent Indigo
    m_colors.barCompare = sf::Color(245, 158, 11);       // Amber for comparison
    m_colors.barSwap = sf::Color(239, 68, 68);           // Red for swaps/writes
    m_colors.barSorted = sf::Color(16, 185, 129);        // Emerald for sorted
    m_colors.barActive = sf::Color(139, 92, 246);        // Purple for current active element
    
    m_colors.border = sf::Color(50, 50, 70);
}

void ThemeManager::loadLightTheme() {
    // Elegant Light Theme
    m_colors.background = sf::Color(248, 250, 252);      // Slate-50
    m_colors.panelBg = sf::Color(255, 255, 255);         // Pure white
    m_colors.cardBg = sf::Color(241, 245, 249);          // Slate-100
    m_colors.textPrimary = sf::Color(15, 23, 42);        // Slate-900
    m_colors.textSecondary = sf::Color(100, 116, 139);   // Slate-500
    m_colors.accent = sf::Color(79, 70, 229);            // Indigo-600
    m_colors.accentHover = sf::Color(67, 56, 202);        // Indigo-700
    
    // Status colors
    m_colors.success = sf::Color(5, 150, 105);           // Emerald-600
    m_colors.danger = sf::Color(220, 38, 38);            // Red-600
    m_colors.warning = sf::Color(217, 119, 6);           // Amber-600
    m_colors.info = sf::Color(37, 99, 235);              // Blue-600
    
    // Visualizer specific
    m_colors.barDefault = sf::Color(129, 140, 248);      // Indigo-400
    m_colors.barCompare = sf::Color(245, 158, 11);       // Amber
    m_colors.barSwap = sf::Color(239, 68, 68);           // Red
    m_colors.barSorted = sf::Color(16, 185, 129);        // Emerald
    m_colors.barActive = sf::Color(139, 92, 246);        // Purple
    
    m_colors.border = sf::Color(226, 232, 240);          // Slate-200
}

void ThemeManager::loadHighContrastTheme() {
    // Pure Black background and stark White layout contrast borders
    m_colors.background = sf::Color(0, 0, 0);            // Pure Black
    m_colors.panelBg = sf::Color(10, 10, 10);
    m_colors.cardBg = sf::Color(25, 25, 25);
    m_colors.textPrimary = sf::Color(255, 255, 255);     // Stark White
    m_colors.textSecondary = sf::Color(210, 210, 210);   // High Contrast Light Gray
    m_colors.accent = sf::Color(255, 255, 0);            // Neon Yellow
    m_colors.accentHover = sf::Color(220, 220, 0);
    
    // Status colors
    m_colors.success = sf::Color(0, 255, 0);             // Pure Green
    m_colors.danger = sf::Color(255, 0, 0);              // Pure Red
    m_colors.warning = sf::Color(255, 128, 0);           // High contrast orange
    m_colors.info = sf::Color(0, 255, 255);              // Stark Cyan
    
    // Visualizer specific colors
    m_colors.barDefault = sf::Color(255, 255, 255);      // Pure White bars
    m_colors.barCompare = sf::Color(255, 255, 0);        // Yellow
    m_colors.barSwap = sf::Color(255, 0, 0);             // Red
    m_colors.barSorted = sf::Color(0, 255, 0);           // Green
    m_colors.barActive = sf::Color(0, 255, 255);         // Cyan
    
    m_colors.border = sf::Color(255, 255, 255);          // Stark white border
}

} // namespace core
