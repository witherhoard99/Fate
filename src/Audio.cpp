#include "Audio.h"

Audio::Audio(const std::string &filename, bool loopSound, float volume):
    m_soundBuffer(filename),
    m_sound(m_soundBuffer)
{
    m_sound.setLooping(loopSound);
    m_sound.setSpatializationEnabled(false);
    m_sound.setVolume(volume);
}

void Audio::ContinuePlaying()
{
    sf::Sound::Status status = m_sound.getStatus();
    if (status == sf::Sound::Status::Playing)
        return;
    if (status == sf::Sound::Status::Paused || status == sf::Sound::Status::Stopped)
        m_sound.play();
}
