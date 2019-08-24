// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cubeb/cubeb.h>

#include "AudioCommon/CubebStream.h"
#include "AudioCommon/CubebUtils.h"
#include "Common/CommonTypes.h"
#include "Common/Logging/Log.h"
#include "Common/Thread.h"
#include "Core/ConfigManager.h"

constexpr u32 MIN_SURROUND_BUFFER_SIZE = 240;

long CubebStream::DataCallback(cubeb_stream* stream, void* user_data, const void* /*input_buffer*/,
                               void* output_buffer, long num_frames)
{
  auto* self = static_cast<CubebStream*>(user_data);

  if (self->m_stereo)
    self->m_mixer->Mix(static_cast<short*>(output_buffer), num_frames);
  else
    self->m_mixer->MixSurround(static_cast<float*>(output_buffer), num_frames);

  return num_frames;
}

void CubebStream::StateCallback(cubeb_stream* stream, void* user_data, cubeb_state state)
{
}

bool CubebStream::Init()
{
  m_ctx = CubebUtils::GetContext();
  if (!m_ctx)
    return false;

  m_stereo = !SConfig::GetInstance().bDPL2Decoder;

  cubeb_stream_params params;
  params.rate = m_mixer->GetSampleRate();
  if (m_stereo)
  {
    params.channels = 2;
    params.format = CUBEB_SAMPLE_S16NE;
    params.layout = CUBEB_LAYOUT_STEREO;
  }
  else
  {
    params.channels = 6;
    params.format = CUBEB_SAMPLE_FLOAT32NE;
    params.layout = CUBEB_LAYOUT_3F2_LFE;
  }

  u32 latency = 240;  // get_min_latency shouldn't fail, but if it does.. about 10ms
  if (cubeb_get_min_latency(m_ctx.get(), &params, &latency) == CUBEB_OK)
  {
    INFO_LOG(AUDIO, "Minimum latency: %i frames", latency);
  }
  else
  {
    ERROR_LOG(AUDIO, "Error getting minimum latency");
  }

  if (!m_stereo)
  {
    latency = std::max(latency, MIN_SURROUND_BUFFER_SIZE);
  }

  return cubeb_stream_init(m_ctx.get(), &m_stream, "Dolphin Audio Output", nullptr, nullptr,
                           nullptr, &params, latency, DataCallback, StateCallback,
                           this) == CUBEB_OK;
}

bool CubebStream::SetRunning(bool running)
{
  if (running)
    return cubeb_stream_start(m_stream) == CUBEB_OK;
  else
    return cubeb_stream_stop(m_stream) == CUBEB_OK;
}

CubebStream::~CubebStream()
{
  SetRunning(false);
  cubeb_stream_destroy(m_stream);
  m_ctx.reset();
}

void CubebStream::SetVolume(int volume)
{
  cubeb_stream_set_volume(m_stream, volume / 100.0f);
}
