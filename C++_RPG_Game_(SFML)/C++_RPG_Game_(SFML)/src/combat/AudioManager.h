#pragma once

#include <SFML/Audio.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
namespace RPG {
    class AudioManager {
    public:
        static AudioManager& getInstance();
        void playSoundFile(const std::string& id);
        

    private:
        std::unordered_map<std::string, sf::SoundBuffer> m_buffers;
        std::vector<std::unique_ptr<sf::Sound>> m_sounds;
    };
}