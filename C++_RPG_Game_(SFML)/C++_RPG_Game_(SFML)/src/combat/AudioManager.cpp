#include "AudioManager.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <external/json.hpp>

using json = nlohmann::json;

namespace RPG {

    AudioManager& AudioManager::getInstance() {
        static AudioManager instance;
        return instance;
    }

    bool AudioManager::loadFromJSON(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "CRITICAL: Failed to open audio file: " << filepath << std::endl;
            return false;
        }

        json j;
        try {
            file >> j;
        }
        catch (json::parse_error& e) {
            std::cerr << "JSON Parse Error in audio file: " << e.what() << std::endl;
            return false;
        }

        if (!j.contains("sounds") || !j["sounds"].is_array()) {
            std::cerr << "JSON Error: 'sounds' array missing in " << filepath << std::endl;
            return false;
        }

        for (const auto& item : j["sounds"]) {
            std::string id = item.value("id", "");
            std::string filePath = item.value("filePath", "");

            if (id.empty() || filePath.empty()) {
                std::cerr << "Skipping invalid sound entry (missing id or filePath)" << std::endl;
                continue;
            }

            sf::SoundBuffer buffer;
            if (!buffer.loadFromFile(filePath)) {
                std::cerr << "Failed to load sound buffer: " << filePath << " (id=" << id << ")" << std::endl;
                continue;
            }

            m_loadedSounds[id] = std::move(buffer);
            std::cout << "Loaded sound: " << id << " from " << filePath << std::endl;

            loadFootsteps("grass_sound");
        }
        return true;
    }

    void AudioManager::playSound(const std::string& id, float volume, bool loop) {
        auto it = m_loadedSounds.find(id);
        if (it == m_loadedSounds.end()) {
            std::cerr << "Sound id not found: " << id << std::endl;
            return;
        }

        auto sound = std::make_unique<sf::Sound>(it->second);
        sound->setVolume(volume);
        sound->setLooping(loop);
        sound->play();

        m_sounds.push_back(std::move(sound));
    }

    void AudioManager::playSoundFile(const std::string& path) {
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(path))
            return;
        m_buffers[path] = std::move(buffer);
        auto sound = std::make_unique<sf::Sound>(m_buffers[path]);
        sound->play();
        m_sounds.push_back(std::move(sound));
    }

    void AudioManager::stopMusic() {
        for (auto& sound : m_sounds) {
            sound->stop();
        }
        stopFootsteps();
    }

    void AudioManager::loadFootsteps(const std::string& id) {
        auto it = m_loadedSounds.find(id);
        if (it == m_loadedSounds.end()) {
            std::cerr << "Footstep sound ID '" << id << "' not found in preloaded sounds.\n";
            return;
        }
        m_footstepSound = std::make_unique<sf::Sound>(it->second);
    }

    void AudioManager::startFootsteps() {
        if (!m_footstepSound) return;
        if (m_footstepSound->getStatus() != sf::Sound::Status::Playing) {
            m_footstepSound->play();
            m_footstepSound->setLooping(true);
        }
    }

    void AudioManager::stopFootsteps() {
        if (m_footstepSound) m_footstepSound->stop();
    }

    void AudioManager::setFootstepsLoop(bool loop) {
        if (m_footstepSound) m_footstepSound->setLooping(loop);
    }
}