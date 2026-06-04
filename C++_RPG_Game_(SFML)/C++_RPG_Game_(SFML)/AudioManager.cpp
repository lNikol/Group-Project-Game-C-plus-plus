#include "combat/AudioManager.h"
#include <iostream>
#include <algorithm>

namespace RPG {
    AudioManager& AudioManager::getInstance() {
        static AudioManager instance;
        return instance;
    }

    void AudioManager::loadSound(const std::string& id, const std::string& path) {
        sf::SoundBuffer buffer;

        if (!buffer.loadFromFile(path)) {
            std::cerr << "Failed to load sound: " << path << std::endl;
            return;
        }

        m_buffers[id] = std::move(buffer);
    }

    void AudioManager::playSound(const std::string& id) {
        auto it = m_buffers.find(id);

        if (it == m_buffers.end()) {
            std::cerr << "Sound not found: " << id << std::endl;
            return;
        }

        //// Remove finished sounds
        //m_sounds.erase(
        //    std::remove_if(
        //        m_sounds.begin(),
        //        m_sounds.end(),
        //        [](const std::unique_ptr<sf::Sound>& s) {
        //            return s->getStatus() == sf::Sound::Stopped;
        //        }),
        //    m_sounds.end()
        //);

        // Create sound
        auto sound = std::make_unique<sf::Sound>();

        sound->setBuffer(it->second);
        sound->play();

        m_sounds.push_back(std::move(sound));
    }
}