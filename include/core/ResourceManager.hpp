#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <iostream>

namespace core {

class ResourceManager {
public:
    static ResourceManager& getInstance() {
        static ResourceManager instance;
        return instance;
    }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    sf::Font& getFont(const std::string& filepath) {
        auto it = m_fonts.find(filepath);
        if (it != m_fonts.end()) {
            return *it->second;
        }

        auto font = std::make_unique<sf::Font>();
        if (!font->loadFromFile(filepath)) {
            std::cerr << "Failed to load font: " << filepath << std::endl;
        }
        m_fonts[filepath] = std::move(font);
        return *m_fonts[filepath];
    }

    sf::Texture& getTexture(const std::string& filepath) {
        auto it = m_textures.find(filepath);
        if (it != m_textures.end()) {
            return *it->second;
        }

        auto texture = std::make_unique<sf::Texture>();
        if (!texture->loadFromFile(filepath)) {
            std::cerr << "Failed to load texture: " << filepath << std::endl;
        }
        m_textures[filepath] = std::move(texture);
        return *m_textures[filepath];
    }

    sf::SoundBuffer& getSoundBuffer(const std::string& filepath) {
        auto it = m_soundBuffers.find(filepath);
        if (it != m_soundBuffers.end()) {
            return *it->second;
        }

        auto soundBuffer = std::make_unique<sf::SoundBuffer>();
        if (!soundBuffer->loadFromFile(filepath)) {
            std::cerr << "Failed to load sound buffer: " << filepath << std::endl;
        }
        m_soundBuffers[filepath] = std::move(soundBuffer);
        return *m_soundBuffers[filepath];
    }

    void clear() {
        m_fonts.clear();
        m_textures.clear();
        m_soundBuffers.clear();
    }

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    std::unordered_map<std::string, std::unique_ptr<sf::Font>> m_fonts;
    std::unordered_map<std::string, std::unique_ptr<sf::Texture>> m_textures;
    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> m_soundBuffers;
};

} // namespace core

#endif // RESOURCE_MANAGER_HPP
