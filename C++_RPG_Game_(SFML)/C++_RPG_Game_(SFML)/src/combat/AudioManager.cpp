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
    bool AudioManager::playMusic(const std::string& path, bool loop)
    {
        m_music = std::make_unique<sf::Music>();

        if (!m_music->openFromFile(path))
        {
            std::cerr << "Failed to load music: " << path << '\n';
            return false;
        }

        m_music->setLooping(loop);
        m_music->play();
        m_music->setVolume(50);

        return true;
    }
    void AudioManager::stopMusic()
    {
        if (m_music)
            m_music->stop();
    }
    void AudioManager::loadFootsteps(const std::string& path)
    {
        if (!m_footstepBuffer.loadFromFile(path))
            return;

        m_footstepSound = std::make_unique<sf::Sound>(m_footstepBuffer);
        m_footstepSound->setLooping(true);
    }
    void AudioManager::startFootsteps()
    {
        if (!m_footstepSound)
            return;

        if (m_footstepSound->getStatus() != sf::Sound::Status::Playing)
            m_footstepSound->play();
    }
    void AudioManager::stopFootsteps()
    {
        if (m_footstepSound)
            m_footstepSound->stop();
    }
    void AudioManager::setFootstepsLoop(bool loop)
    {
        if (m_footstepSound)
            m_footstepSound->setLooping(loop);
    }
}