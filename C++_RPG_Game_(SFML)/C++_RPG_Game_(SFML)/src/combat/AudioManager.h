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

        void playSoundFile(const std::string& path);
        

        bool playMusic(const std::string& path, bool loop = true);
        void stopMusic();

        void loadFootsteps(const std::string& path);
        void startFootsteps();
        void stopFootsteps();
        void setFootstepsLoop(bool loop);

    private:
        std::unordered_map<std::string, sf::SoundBuffer> m_buffers;
        std::vector<std::unique_ptr<sf::Sound>> m_sounds;
        std::unique_ptr<sf::Music> m_music;
        sf::SoundBuffer m_footstepBuffer;
        std::unique_ptr<sf::Sound> m_footstepSound;
    };


}