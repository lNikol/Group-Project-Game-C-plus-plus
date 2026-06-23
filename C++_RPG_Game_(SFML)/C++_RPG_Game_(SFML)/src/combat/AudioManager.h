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

        bool loadFromJSON(const std::string& filepath);

        void playSound(const std::string& id, float volume = 100.0f, bool loop = false);

        void playSoundFile(const std::string& path);

        void stopMusic();

        void loadFootsteps(const std::string& id);
        void startFootsteps();
        void stopFootsteps();
        void setFootstepsLoop(bool loop);

    private:
        std::unordered_map<std::string, sf::SoundBuffer> m_buffers;

        std::unordered_map<std::string, sf::SoundBuffer> m_loadedSounds;

        std::vector<std::unique_ptr<sf::Sound>> m_sounds;

        std::unique_ptr<sf::Music> m_music;
        sf::SoundBuffer m_footstepBuffer;
        std::unique_ptr<sf::Sound> m_footstepSound;
    };
}
