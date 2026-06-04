#include "AudioManager.h"
#include <iostream>
#include <algorithm>

namespace RPG {
    AudioManager& AudioManager::getInstance() {
        static AudioManager instance;
        return instance;
    }
    void AudioManager::playSoundFile(const std::string& path)
    {
        sf::SoundBuffer buffer;

        if (!buffer.loadFromFile(path))
            return;

        m_buffers[path] = std::move(buffer);

        auto sound = std::make_unique<sf::Sound>(m_buffers[path]);
        sound->play();

        m_sounds.push_back(std::move(sound));
    }
}