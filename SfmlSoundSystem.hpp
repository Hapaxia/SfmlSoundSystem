//////////////////////////////////////////////////////////////////////////////
//
// SFML Sound System (https://github.com/Hapaxia/SfmlSoundSystem)
//
// Copyright(c) 2016-2025 M.J.Silk
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

#pragma once

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

// SSS (SFML Sound System) v1.6 - Control
class Control
{
public:
	Control();
	void update();
	bool copyBuffer(const std::string& soundId, const sf::SoundBuffer& soundBuffer);
	bool loadBuffer(const std::string& soundId, const std::vector<char>& memoryBlock);
	bool openMusic(const std::string& musicId, std::vector<char>& memoryBlock);
	bool loadBuffer(const std::string& soundId, const char* memoryBlock, std::size_t memorySize);
	bool loadBuffer(const std::string& soundId, const std::string& filename);
	bool openMusic(const std::string& musicId, const std::string& filename);
	bool playSound(const std::string& soundId, const sf::Vector3f& positionOffset = { 0.f, 0.f, 0.f }, float volumeMultiplier = 1.f);
	bool playSound(const std::string& soundId, const sf::Vector2f& positionOffset, float volumeMultiplier = 1.f);
	bool playMusic(const std::string& musicId, sf::Time transitionDuration = sf::Time::Zero, float volumeMultiplier = 1.f);
	bool assignVolumeSound(const std::string& soundId, float volume = 1.f);
	bool assignVolumeMusic(const std::string& musicId, float volume = 1.f);
	sf::Time getCurrentMusicPosition() const;
	bool restartMusic();
	void pauseMusic();
	void resumeMusic();
	void stopFx();
	void stopMusic();
	void stopAll();
	void setMaxNumberOfVoices(std::size_t maxNumberOfVoices);
	std::size_t getMaxNumberOfVoices() const;
	sf::SoundBuffer& getBuffer(const std::string& soundId);
	std::string getCurrentMusic() const;
	sf::SoundSource::Status getCurrentMusicStatus() const;
	std::size_t getNumberOfSoundsPlaying() const;
	bool getIsCurrentlyPaused() const;







private:
	const std::size_t m_defaultMaxNumberOfVoices;
	const sf::SoundBuffer m_emptySoundBuffer;

	std::unordered_map<std::string, sf::SoundBuffer> m_buffers;
	std::unordered_map<std::string, float> m_bufferVolumes;
	std::vector<sf::Sound> m_voices;

	std::unordered_map<std::string, std::vector<char>*> m_pMusicMemories;
	std::unordered_map<std::string, std::string> m_musicFilenames;
	std::unordered_map<std::string, float> m_musicVolumes;
	std::vector<sf::Music> m_musics;
	std::size_t m_currentMusicVoice;
	std::string m_currentMusic;

	sf::Clock m_fadeTimer;
	float m_previousVolume;
	sf::Time m_transitionDuration;

	bool priv_triggerSound(const sf::SoundBuffer& buffer, const sf::Vector3f& positionOffset = { 0.f, 0.f, 0.f }, float volume = 1.f);
	void priv_startSound(sf::Sound& voice, const sf::SoundBuffer& buffer, const sf::Vector3f& positionOffset = { 0.f, 0.f, 0.f }, float volume = 1.f);
};

} // namespace sfmlSoundSystem













#include <SFML/Audio/Listener.hpp>

namespace sfmlSoundSystem
{

inline Control::Control()
	: m_defaultMaxNumberOfVoices{ 64u }
	, m_emptySoundBuffer{}
	, m_musics(2u)
	, m_voices(m_defaultMaxNumberOfVoices, sf::Sound(m_emptySoundBuffer))
	, m_currentMusic{ "" }
	, m_currentMusicVoice{ 0u }
	, m_previousVolume{ 0.f }
	, m_transitionDuration{ sf::Time::Zero }
{
	for (auto& music : m_musics)
		music.setLooping(true);
}

inline void Control::update()
{
	const sf::Time fadeInLength{ m_transitionDuration };
	const sf::Time fadeCurrentTime{ m_fadeTimer.getElapsedTime() };
	const float fadeRatio{ (fadeInLength == sf::Time::Zero) ? 1.f : (fadeCurrentTime / fadeInLength) };
	const float volumeMultiplier{ (fadeRatio < 1.f) ? fadeRatio : 1.f };
	m_musics[m_currentMusicVoice].setVolume(m_musicVolumes[m_currentMusic] * volumeMultiplier * 100.f);

	sf::Music& previous{ m_musics[1u - m_currentMusicVoice] };
	if (fadeRatio <= 1.f)
		previous.setVolume(m_previousVolume * (1.f - fadeRatio));
	else if (previous.getStatus() == sf::Music::Status::Playing)
		previous.stop();
}

inline std::size_t Control::getMaxNumberOfVoices() const
{
	return m_voices.size();
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

inline std::size_t Control::getNumberOfSoundsPlaying() const
{
	std::size_t total{ 0u };
	for (auto& voice : m_voices)
	{
		if (voice.getStatus() == sf::SoundSource::Status::Playing)
			++total;
	}
	return total;
}

inline bool Control::getIsCurrentlyPaused() const
{
	return (getCurrentMusicStatus() == sf::SoundSource::Status::Paused);
}

inline bool Control::copyBuffer(const std::string& soundId, const sf::SoundBuffer& soundBuffer)
{
	return m_bufferVolumes.emplace(soundId, 1.f).second &&
		m_buffers.emplace(soundId, soundBuffer).second;
}

inline bool Control::loadBuffer(const std::string& soundId, const std::vector<char>& memoryBlock)
{
	sf::SoundBuffer soundBuffer;
	return soundBuffer.loadFromMemory(memoryBlock.data(), memoryBlock.size()) &&
		m_bufferVolumes.emplace(soundId, 1.f).second &&
		m_buffers.emplace(soundId, soundBuffer).second;
}

inline bool Control::loadBuffer(const std::string& soundId, const char* memoryBlock, const std::size_t memorySize)
{
	sf::SoundBuffer soundBuffer;
	return soundBuffer.loadFromMemory(memoryBlock, memorySize) &&
		m_bufferVolumes.emplace(soundId, 1.f).second &&
		m_buffers.emplace(soundId, soundBuffer).second;
}

inline bool Control::openMusic(const std::string& musicId, std::vector<char>& memoryBlock)
{
	return m_musicVolumes.emplace(musicId, 1.f).second &&
		m_pMusicMemories.emplace(musicId, &memoryBlock).second;
}

inline bool Control::loadBuffer(const std::string& soundId, const std::string& filename)
{
	sf::SoundBuffer soundBuffer;
	return soundBuffer.loadFromFile(filename) &&
		m_bufferVolumes.emplace(soundId, 1.f).second &&
		m_buffers.emplace(soundId, soundBuffer).second;
}

inline bool Control::openMusic(const std::string& musicId, const std::string& filename)
{
	return m_musicVolumes.emplace(musicId, 1.f).second &&
		m_musicFilenames.emplace(musicId, filename).second;
}

inline bool Control::playSound(const std::string& soundId, const sf::Vector3f& positionOffset, const float volumeMultiplier)
{

	if (m_buffers.find(soundId) == m_buffers.end())
		return false;
	priv_triggerSound(m_buffers[soundId], positionOffset, m_bufferVolumes[soundId] * volumeMultiplier);
	return true;
}

inline bool Control::playSound(const std::string& soundId, const sf::Vector2f& positionOffset, const float volumeMultiplier)
{
	return playSound(soundId, { positionOffset.x, positionOffset.y, 0.000001f }, volumeMultiplier); // apply 2D position to the XY plane immediately in front (+z)
}

inline bool Control::playMusic(const std::string& musicId, const sf::Time transitionDuration, const float volumeMultiplier)
{
	const bool useFileNotMemory{ m_pMusicMemories.find(musicId) == m_pMusicMemories.end() };;
	if (useFileNotMemory && m_musicFilenames.find(musicId) == m_musicFilenames.end())
		return false;

	m_currentMusic = musicId;
	sf::Music& previous{ m_musics[m_currentMusicVoice] };
	m_currentMusicVoice = 1u - m_currentMusicVoice;
	sf::Music& current{ m_musics[m_currentMusicVoice] };

	m_previousVolume = previous.getVolume();
	if (current.getStatus() == sf::Music::Status::Playing)
		current.stop();
	if (useFileNotMemory)
	{
		if (!current.openFromFile(m_musicFilenames[musicId]))
			return false;
	}
	else if (!current.openFromMemory(m_pMusicMemories[musicId]->data(), m_pMusicMemories[musicId]->size()))
		return false;
	current.setVolume(0.f);
	m_transitionDuration = transitionDuration;
	current.play();
	m_fadeTimer.restart();

	return true;
}

inline bool Control::assignVolumeSound(const std::string& soundId, const float volume)
{
	if (m_bufferVolumes.find(soundId) == m_bufferVolumes.end())
		return false;
	m_bufferVolumes[soundId] = volume;
	return true;
}

inline bool Control::assignVolumeMusic(const std::string& musicId, const float volume)
{
	if (m_musicVolumes.find(musicId) == m_musicVolumes.end())
		return false;
	m_musicVolumes[musicId] = volume;
	return true;
}

inline sf::Time Control::getCurrentMusicPosition() const
{
	if (getCurrentMusicStatus() != sf::SoundSource::Status::Stopped)
		return m_musics[m_currentMusicVoice].getPlayingOffset();

	return sf::Time::Zero;
}

inline bool Control::restartMusic()
{
	return playMusic(m_currentMusic);
}

inline void Control::pauseMusic()
{
	if (getCurrentMusicStatus() == sf::SoundSource::Status::Playing)
		m_musics[m_currentMusicVoice].pause();
}

inline void Control::resumeMusic()
{
	if (getCurrentMusicStatus() == sf::SoundSource::Status::Paused)
		m_musics[m_currentMusicVoice].play();
}

inline void Control::stopFx()
{
	for (auto& voice : m_voices)
		voice.stop();
}

inline void Control::stopMusic()
{
	for (auto& music : m_musics)
		music.stop();
}

inline void Control::stopAll()
{
	stopFx();
	stopMusic();
}

inline void Control::setMaxNumberOfVoices(const std::size_t maxNumberOfVoices)
{
	m_voices.resize(maxNumberOfVoices, sf::Sound(m_emptySoundBuffer));
}







//////////
//
// PRIVATE
//
//////////



inline bool Control::priv_triggerSound(const sf::SoundBuffer& buffer, const sf::Vector3f& positionOffset, const float volume)
{
	for (auto& voice : m_voices)
	{
		if (voice.getStatus() == sf::Sound::Status::Stopped)
		{
			priv_startSound(voice, buffer, positionOffset, volume);
			return true;
		}
	}
	return false;
}

inline void Control::priv_startSound(sf::Sound& voice, const sf::SoundBuffer& buffer, const sf::Vector3f& positionOffset, const float volume)
{
	voice.setBuffer(buffer);
	voice.setVolume(volume * 100.f);
	voice.setPosition(positionOffset);
	voice.setRelativeToListener(true);
	voice.play();
}

} // namespace sfmlSoundSystem

#ifndef SFMLSOUNDSYSTEM_NO_NAMESPACE_SHORTCUT
namespace sss = sfmlSoundSystem; // create shortcut namespace
#endif // SFMLSOUNDSYSTEM_NO_NAMESPACE_SHORTCUT
