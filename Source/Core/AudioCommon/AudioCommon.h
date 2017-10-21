// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <string>
#include <vector>

class Mixer;

namespace AudioCommon
{
Mixer* GetMixer();
void Init();
void Shutdown();
std::string GetDefaultSoundBackend();
std::vector<std::string> GetSoundBackends();
bool SupportsDPL2Decoder(const std::string& backend);
bool SupportsLatencyControl(const std::string& backend);
bool SupportsVolumeChanges(const std::string& backend);
void UpdateSoundStream();
void SetSoundStreamRunning(bool running);
void SendAIBuffer(const short* samples, unsigned int num_samples);
void StartAudioDump();
void StopAudioDump();
void IncreaseVolume(unsigned short offset);
void DecreaseVolume(unsigned short offset);
void ToggleMuteVolume();
}
