//////////////////////////////////////////////////////////////////////////////
//
// SFML Sound System (https://github.com/Hapaxia/SfmlSoundSystem)
// --
//
// Control
//
// Copyright(c) 2016-2021 M.J.Silk
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

#include "Control.hpp"

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>
#include <SFML/Audio/Listener.hpp>
#include <SFML/System/Clock.hpp>

namespace
{

constexpr unsigned int defaultNumberOfVoices{ 64u };

} // namespace

namespace sfmlSoundSystem
{

Control::Control()
	: m_musics(2)
	, m_voices(defaultNumberOfVoices)
	, m_currentMusic("")
	, m_currentMusicVoice(0u)
	, m_previousVolume(0.f)
	, m_transitionDuration(sf::Time::Zero)
{
	for (auto& music : m_musics)
		music.setLoop(true);
}

void Control::update()
{
	const sf::Time fadeInLength{ m_transitionDuration };
	const sf::Time fadeCurrentTime{ m_fadeTimer.getElapsedTime() };
	const float fadeRatio{ fadeCurrentTime / fadeInLength };
	const float volumeMultiplier{ (fadeRatio < 1.f) ? fadeRatio : 1.f };
	m_musics[m_currentMusicVoice].setVolume(m_musicVolumes[m_currentMusic] * volumeMultiplier * 100.f);

	sf::Music& previous{ m_musics[1 - m_currentMusicVoice] };
	if (fadeRatio <= 1.f)
		previous.setVolume(m_previousVolume * (1.f - fadeRatio));
	else if (previous.getStatus() == sf::Music::Status::Playing)
		previous.stop();
}

bool Control::copyBuffer(const std::string& soundId, const sf::SoundBuffer& soundBuffer)
{
	return m_bufferVolumes.emplace(soundId, 1.f).second &&
		m_buffers.emplace(soundId, soundBuffer).second;
}

bool Control::loadBuffer(const std::string& soundId, const std::vector<char>& memoryBlock)
{
	sf::SoundBuffer soundBuffer;
	return soundBuffer.loadFromMemory(memoryBlock.data(), memoryBlock.size()) &&
		m_bufferVolumes.emplace(soundId, 1.f).second &&
		m_buffers.emplace(soundId, soundBuffer).second;
}

bool Control::loadBuffer(const std::string& soundId, const char* memoryBlock, const std::size_t memorySize)
{
	sf::SoundBuffer soundBuffer;
	return soundBuffer.loadFromMemory(memoryBlock, memorySize) &&
		m_bufferVolumes.emplace(soundId, 1.f).second &&
		m_buffers.emplace(soundId, soundBuffer).second;
}

bool Control::linkMusic(const std::string& musicId, std::vector<char>& memoryBlock)
{
	return m_musicVolumes.emplace(musicId, 1.f).second &&
		m_pMusicMemories.emplace(musicId, &memoryBlock).second;
}

bool Control::loadBuffer(const std::string& soundId, const std::string& filename)
{
	sf::SoundBuffer soundBuffer;
	return soundBuffer.loadFromFile(filename) &&
		m_bufferVolumes.emplace(soundId, 1.f).second &&
		m_buffers.emplace(soundId, soundBuffer).second;
}

bool Control::linkMusic(const std::string& musicId, const std::string& filename)
{
	return m_musicVolumes.emplace(musicId, 1.f).second &&
		m_musicFilenames.emplace(musicId, filename).second;
}

bool Control::playSound(const std::string& soundId, const sf::Vector3f& positionOffset, const float volumeMultiplier)
{
	if (m_buffers.find(soundId) == m_buffers.end())
		return false;
	priv_triggerSound(m_buffers[soundId], positionOffset, m_bufferVolumes[soundId] * volumeMultiplier);
	return true;
}

bool Control::playSound(const std::string& soundId, const sf::Vector2f& positionOffset, const float volumeMultiplier)
{
	return playSound(soundId, { positionOffset.x, positionOffset.y, 0.000001f }, volumeMultiplier); // apply 2D position to the XY plane immediately in front (+z)
}

bool Control::playMusic(const std::string& musicId, const sf::Time transitionDuration, const float volumeMultiplier)
{
	const bool useFileNotMemory{ m_pMusicMemories.find(musicId) == m_pMusicMemories.end() };;
	if (useFileNotMemory && m_musicFilenames.find(musicId) == m_musicFilenames.end())
		return false;

	m_currentMusic = musicId;
	sf::Music& previous{ m_musics[m_currentMusicVoice] };
	m_currentMusicVoice = 1 - m_currentMusicVoice;
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

bool Control::assignVolumeSound(const std::string& soundId, const float volume)
{
	if (m_bufferVolumes.find(soundId) == m_bufferVolumes.end())
		return false;
	m_bufferVolumes[soundId] = volume;
	return true;
}

bool Control::assignVolumeMusic(const std::string& musicId, const float volume)
{
	if (m_musicVolumes.find(musicId) == m_musicVolumes.end())
		return false;
	m_musicVolumes[musicId] = volume;
	return true;
}

sf::Time Control::getCurrentMusicPosition() const
{
	if (getCurrentMusicStatus() != sf::SoundSource::Status::Stopped)
		return m_musics[m_currentMusicVoice].getPlayingOffset();

	return sf::Time::Zero;
}

bool Control::restartMusic()
{
	return playMusic(m_currentMusic);
}

void Control::pauseMusic()
{
	if (getCurrentMusicStatus() == sf::SoundSource::Status::Playing)
		m_musics[m_currentMusicVoice].pause();
}

void Control::resumeMusic()
{
	if (getCurrentMusicStatus() == sf::SoundSource::Status::Paused)
		m_musics[m_currentMusicVoice].play();
}

void Control::stopFx()
{
	for (auto& voice : m_voices)
		voice.stop();
}

void Control::stopMusic()
{
	for (auto& music : m_musics)
		music.stop();
}

void Control::stopAll()
{
	stopFx();
	stopMusic();
}

void Control::setMaximumNumberOfVoices(const unsigned int maximumNumberOfVoices)
{
	m_voices.resize(maximumNumberOfVoices);
}







//////////
//
// PRIVATE
//
//////////



bool Control::priv_triggerSound(const sf::SoundBuffer& buffer, const sf::Vector3f& positionOffset, const float volume)
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

void Control::priv_startSound(sf::Sound& voice, const sf::SoundBuffer& buffer, const sf::Vector3f& positionOffset, const float volume)
{
	voice.setBuffer(buffer);
	voice.setVolume(volume * 100.f);
	voice.setPosition(positionOffset);
	voice.setRelativeToListener(true);
	voice.play();
}

} // namespace sfmlSoundSystem
