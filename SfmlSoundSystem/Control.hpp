//////////////////////////////////////////////////////////////////////////////
//
// SFML Sound System (https://github.com/Hapaxia/SfmlSoundSystem)
// --
//
// Control
//
// Copyright(c) 2016-2018 M.J.Silk
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions :
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software.If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
// M.J.Silk
// MJSilk2@gmail.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SFMLSOUNDSYSTEM_CONTROL_HPP
#define SFMLSOUNDSYSTEM_CONTROL_HPP

#include "Common.hpp"

#include <unordered_map>
#include <vector>

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/SoundSource.hpp>

namespace sfmlSoundSystem
{

// SSS (SFML Sound System) v1.3 - Control
class Control
{
public:
	Control();
	void update();
	bool loadBuffer(const std::string& soundId, const std::vector<char>& memoryBlock);
	bool linkMusic(const std::string& musicId, std::vector<char>& memoryBlock);
	bool loadBuffer(const std::string& soundId, const char* memoryBlock, std::size_t memorySize);
	bool loadBuffer(const std::string& soundId, const std::string& filename);
	bool linkMusic(const std::string& musicId, const std::string& filename);
	bool playSound(const std::string& soundId, const sf::Vector3f& positionOffset = { 0.f, 0.f, 0.f }, float volumeMultiplier = 1.f);
	bool playSound(const std::string& soundId, const sf::Vector2f& positionOffset, float volumeMultiplier = 1.f);
	bool playMusic(const std::string& musicId, sf::Time transitionDuration = sf::Time::Zero, float volumeMultiplier = 1.f);
	bool assignVolumeSound(const std::string& soundId, float volume = 1.f);
	bool assignVolumeMusic(const std::string& musicId, float volume = 1.f);
	sf::Time getCurrentMusicPosition() const;
	void pauseMusic();
	void resumeMusic();
	void stopFx();
	void stopMusic();
	void stopAll();
	void setMaximumNumberOfVoices(unsigned int maximumNumberOfVoices);
	unsigned int getMaximumNumberOfVoices() const;
	sf::SoundBuffer& getBuffer(const std::string& soundId);
	std::string getCurrentMusic() const;
	sf::SoundSource::Status getCurrentMusicStatus() const;
	unsigned int getNumberOfSoundsPlaying() const;







private:
	std::unordered_map<std::string, sf::SoundBuffer> m_buffers;
	std::unordered_map<std::string, float> m_bufferVolumes;
	std::vector<sf::Sound> m_voices;

	std::unordered_map<std::string, std::vector<char>*> m_pMusicMemories;
	std::unordered_map<std::string, std::string> m_musicFilenames;
	std::unordered_map<std::string, float> m_musicVolumes;
	std::vector<sf::Music> m_musics;
	unsigned int m_currentMusicVoice;
	std::string m_currentMusic;

	sf::Clock m_fadeTimer;
	float m_previousVolume;
	sf::Time m_transitionDuration;

	bool priv_triggerSound(const sf::SoundBuffer& buffer, const sf::Vector3f& positionOffset = { 0.f, 0.f, 0.f }, float volume = 1.f);
	void priv_startSound(sf::Sound& voice, const sf::SoundBuffer& buffer, const sf::Vector3f& positionOffset = { 0.f, 0.f, 0.f }, float volume = 1.f);
};

inline unsigned int Control::getMaximumNumberOfVoices() const
{
	return static_cast<unsigned int>(m_voices.size());
}

inline sf::SoundBuffer& Control::getBuffer(const std::string& soundId)
{
	return m_buffers[soundId];
}

inline std::string Control::getCurrentMusic() const
{
	return m_currentMusic;
}

inline sf::SoundSource::Status Control::getCurrentMusicStatus() const
{
	return m_musics[m_currentMusicVoice].getStatus();
}

inline unsigned int Control::getNumberOfSoundsPlaying() const
{
	unsigned int total{ 0u };
	for (auto& voice : m_voices)
	{
		if (voice.getStatus() == sf::SoundSource::Status::Playing)
			++total;
	}
	return total;
}

} // namespace sfmlSoundSystem
#endif // SFMLSOUNDSYSTEM_CONTROL_HPP
