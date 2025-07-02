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
    explicit Audio(const std::string& filename, bool loopSound, float volume);

    /**
     * If the sound is paused, it plays from there
     * If the sound is already playing, it plays from the beginning
     * If the sound is stopped, it plays from beginning
     */
    void StartPlaying() { m_sound.play(); }

    /**
     * If the sound is paused, it plays from there
     * If the sound is playing, this method does nothing (the sound continues to play as normal)
     * If the sound is stopped, play from beginning
     */
    void ContinuePlaying();

    /**
     * Pause the sound, if we are playing. Otherwise, does nothing
     */
    void Pause() { m_sound.pause(); }

    /**
     * Resets the sound to the beginning
     */
    void Stop() { m_sound.stop(); }
};



#endif //AUDIO_H
