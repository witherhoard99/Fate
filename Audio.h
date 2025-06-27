#ifndef AUDIO_H
#define AUDIO_H

#include "Util.h"

#include "Audio/Sound.hpp"
#include "Audio/SoundBuffer.hpp"

#include "Jolt/Math/Vec3.h"

class Audio
{
private:
    sf::SoundBuffer m_soundBuffer;
    sf::Sound m_sound;

public:
    explicit Audio(const std::string& filename, bool loopSound, float volume) :
        m_soundBuffer(filename),
        m_sound(m_soundBuffer)
    {
        m_sound.setLooping(loopSound);
        m_sound.setSpatializationEnabled(false);
        m_sound.setVolume(volume);
    }

    void StartPlaying()
    {
        m_sound.play();
    }

    void ContinuePlaying()
    {
        sf::Sound::Status status = m_sound.getStatus();
        if (status == sf::Sound::Status::Playing)
            return;
        if (status == sf::Sound::Status::Paused || status == sf::Sound::Status::Stopped)
            m_sound.play();
    }

    void Pause()
    {
        m_sound.pause();
    }

    void Stop()
    {
        m_sound.stop();
    }
};



#endif //AUDIO_H
