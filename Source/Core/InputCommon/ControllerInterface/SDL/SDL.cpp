// Copyright 2010 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerInterface/SDL/SDL.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <thread>

#include <SDL_events.h>

#include "Common/Event.h"
#include "Common/Logging/Log.h"
#include "Common/ScopeGuard.h"
#include "Common/StringUtil.h"
#include "InputCommon/ControllerInterface/ControllerInterface.h"

#ifdef _WIN32
#pragma comment(lib, "SDL2.lib")
#endif

namespace ciface
{
namespace SDL
{
// 10ms = 100Hz which homebrew docs very roughly imply is within WiiMote normal
// range, used for periodic haptic effects though often ignored by devices
static const u16 RUMBLE_PERIOD = 10;
static const u16 RUMBLE_LENGTH_MAX =
    500;  // ms: enough to span multiple frames at low FPS, but still finite

static std::string GetJoystickName(int index)
{
#if SDL_VERSION_ATLEAST(2, 0, 0)
  return SDL_JoystickNameForIndex(index);
#else
  return SDL_JoystickName(index);
#endif
}

static void OpenAndAddDevice(int index)
{
  SDL_Joystick* dev = SDL_JoystickOpen(index);
  if (dev)
  {
    auto js = std::make_shared<Joystick>(dev, index);
    // only add if it has some inputs/outputs
    if (!js->Inputs().empty() || !js->Outputs().empty())
      g_controller_interface.AddDevice(std::move(js));
  }
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
static Common::Event s_init_event;
static Uint32 s_stop_event_type;
static Uint32 s_populate_event_type;
static Common::Event s_populated_event;
static std::thread s_hotplug_thread;

static bool HandleEventAndContinue(const SDL_Event& e)
{
  if (e.type == SDL_JOYDEVICEADDED)
  {
    OpenAndAddDevice(e.jdevice.which);
  }
  else if (e.type == SDL_JOYDEVICEREMOVED)
  {
    g_controller_interface.RemoveDevice([&e](const auto* device) {
      const Joystick* joystick = dynamic_cast<const Joystick*>(device);
      return joystick && SDL_JoystickInstanceID(joystick->GetSDLJoystick()) == e.jdevice.which;
    });
  }
  else if (e.type == s_populate_event_type)
  {
    for (int i = 0; i < SDL_NumJoysticks(); ++i)
      OpenAndAddDevice(i);
    s_populated_event.Set();
  }
  else if (e.type == s_stop_event_type)
  {
    return false;
  }

  return true;
}
#endif

void Init()
{
#if !SDL_VERSION_ATLEAST(2, 0, 0)
  if (SDL_Init(SDL_INIT_JOYSTICK) != 0)
    ERROR_LOG(SERIALINTERFACE, "SDL failed to initialize");
  return;
#else
  s_hotplug_thread = std::thread([] {
    Common::ScopeGuard quit_guard([] {
      // TODO: there seems to be some sort of memory leak with SDL, quit isn't freeing everything up
      SDL_Quit();
    });

    {
      Common::ScopeGuard init_guard([] { s_init_event.Set(); });

      if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC) != 0)
      {
        ERROR_LOG(SERIALINTERFACE, "SDL failed to initialize");
        return;
      }

      Uint32 custom_events_start = SDL_RegisterEvents(2);
      if (custom_events_start == static_cast<Uint32>(-1))
      {
        ERROR_LOG(SERIALINTERFACE, "SDL failed to register custom events");
        return;
      }
      s_stop_event_type = custom_events_start;
      s_populate_event_type = custom_events_start + 1;

      // Drain all of the events and add the initial joysticks before returning. Otherwise, the
      // individual joystick events as well as the custom populate event will be handled _after_
      // ControllerInterface::Init/RefreshDevices has cleared its list of devices, resulting in
      // duplicate devices.
      SDL_Event e;
      while (SDL_PollEvent(&e) != 0)
      {
        if (!HandleEventAndContinue(e))
          return;
      }
    }

    SDL_Event e;
    while (SDL_WaitEvent(&e) != 0)
    {
      if (!HandleEventAndContinue(e))
        return;
    }
  });

  s_init_event.Wait();
#endif
}

void DeInit()
{
#if !SDL_VERSION_ATLEAST(2, 0, 0)
  SDL_Quit();
#else
  if (!s_hotplug_thread.joinable())
    return;

  SDL_Event stop_event{s_stop_event_type};
  SDL_PushEvent(&stop_event);

  s_hotplug_thread.join();
#endif
}

void PopulateDevices()
{
#if !SDL_VERSION_ATLEAST(2, 0, 0)
  if (!SDL_WasInit(SDL_INIT_JOYSTICK))
    return;

  for (int i = 0; i < SDL_NumJoysticks(); ++i)
    OpenAndAddDevice(i);
#else
  if (!s_hotplug_thread.joinable())
    return;

  SDL_Event populate_event{s_populate_event_type};
  SDL_PushEvent(&populate_event);

  s_populated_event.Wait();
#endif
}

Joystick::Joystick(SDL_Joystick* const joystick, const int sdl_index)
    : m_joystick(joystick), m_name(StripSpaces(GetJoystickName(sdl_index)))
{
// really bad HACKS:
// to not use SDL for an XInput device
// too many people on the forums pick the SDL device and ask:
// "why don't my 360 gamepad triggers/rumble work correctly"
#ifdef _WIN32
  // checking the name is probably good (and hacky) enough
  // but I'll double check with the num of buttons/axes
  std::string lcasename = GetName();
  std::transform(lcasename.begin(), lcasename.end(), lcasename.begin(), tolower);

  if ((std::string::npos != lcasename.find("xbox 360")) &&
      (10 == SDL_JoystickNumButtons(joystick)) && (5 == SDL_JoystickNumAxes(joystick)) &&
      (1 == SDL_JoystickNumHats(joystick)) && (0 == SDL_JoystickNumBalls(joystick)))
  {
    // this device won't be used
    return;
  }
#endif

  if (SDL_JoystickNumButtons(joystick) > 255 || SDL_JoystickNumAxes(joystick) > 255 ||
      SDL_JoystickNumHats(joystick) > 255 || SDL_JoystickNumBalls(joystick) > 255)
  {
    // This device is invalid, don't use it
    // Some crazy devices(HP webcam 2100) end up as HID devices
    // SDL tries parsing these as joysticks
    return;
  }

  // get buttons
  for (u8 i = 0; i != SDL_JoystickNumButtons(m_joystick); ++i)
    AddInput(new Button(i, m_joystick));

  // get hats
  for (u8 i = 0; i != SDL_JoystickNumHats(m_joystick); ++i)
  {
    // each hat gets 4 input instances associated with it, (up down left right)
    for (u8 d = 0; d != 4; ++d)
      AddInput(new Hat(i, m_joystick, d));
  }

  // get axes
  for (u8 i = 0; i != SDL_JoystickNumAxes(m_joystick); ++i)
  {
    // each axis gets a negative and a positive input instance associated with it
    AddAnalogInputs(new Axis(i, m_joystick, -32768), new Axis(i, m_joystick, 32767));
  }

#ifdef USE_SDL_HAPTIC
  // try to get supported ff effects
  m_haptic = SDL_HapticOpenFromJoystick(m_joystick);
  if (m_haptic)
  {
    // SDL_HapticSetGain( m_haptic, 1000 );
    // SDL_HapticSetAutocenter( m_haptic, 0 );

    const unsigned int supported_effects = SDL_HapticQuery(m_haptic);

    // constant effect
    if (supported_effects & SDL_HAPTIC_CONSTANT)
      AddOutput(new ConstantEffect(m_haptic));

    // ramp effect
    if (supported_effects & SDL_HAPTIC_RAMP)
      AddOutput(new RampEffect(m_haptic));

    // sine effect
    if (supported_effects & SDL_HAPTIC_SINE)
      AddOutput(new SineEffect(m_haptic));

    // triangle effect
    if (supported_effects & SDL_HAPTIC_TRIANGLE)
      AddOutput(new TriangleEffect(m_haptic));

    // left-right effect
    if (supported_effects & SDL_HAPTIC_LEFTRIGHT)
      AddOutput(new LeftRightEffect(m_haptic));
  }
#endif
}

Joystick::~Joystick()
{
#ifdef USE_SDL_HAPTIC
  if (m_haptic)
  {
    // stop/destroy all effects
    SDL_HapticStopAll(m_haptic);
    // close haptic first
    SDL_HapticClose(m_haptic);
  }
#endif

  // close joystick
  SDL_JoystickClose(m_joystick);
}

#ifdef USE_SDL_HAPTIC
void Joystick::HapticEffect::Update()
{
  if (m_id == -1 && m_effect.type > 0)
  {
    m_id = SDL_HapticNewEffect(m_haptic, &m_effect);
    if (m_id > -1)
      SDL_HapticRunEffect(m_haptic, m_id, 1);
  }
  else if (m_id > -1 && m_effect.type == 0)
  {
    SDL_HapticStopEffect(m_haptic, m_id);
    SDL_HapticDestroyEffect(m_haptic, m_id);
    m_id = -1;
  }
  else if (m_id > -1)
  {
    SDL_HapticUpdateEffect(m_haptic, m_id, &m_effect);
  }
}

std::string Joystick::ConstantEffect::GetName() const
{
  return "Constant";
}

std::string Joystick::RampEffect::GetName() const
{
  return "Ramp";
}

std::string Joystick::SineEffect::GetName() const
{
  return "Sine";
}

std::string Joystick::TriangleEffect::GetName() const
{
  return "Triangle";
}

std::string Joystick::LeftRightEffect::GetName() const
{
  return "LeftRight";
}

void Joystick::HapticEffect::SetState(ControlState state)
{
  memset(&m_effect, 0, sizeof(m_effect));
  if (state)
  {
    SetSDLHapticEffect(state);
  }
  else
  {
    // this module uses type==0 to indicate 'off'
    m_effect.type = 0;
  }
  Update();
}

void Joystick::ConstantEffect::SetSDLHapticEffect(ControlState state)
{
  m_effect.type = SDL_HAPTIC_CONSTANT;
  m_effect.constant.length = RUMBLE_LENGTH_MAX;
  m_effect.constant.level = (Sint16)(state * 0x7FFF);
}

void Joystick::RampEffect::SetSDLHapticEffect(ControlState state)
{
  m_effect.type = SDL_HAPTIC_RAMP;
  m_effect.ramp.length = RUMBLE_LENGTH_MAX;
  m_effect.ramp.start = (Sint16)(state * 0x7FFF);
}

void Joystick::SineEffect::SetSDLHapticEffect(ControlState state)
{
  m_effect.type = SDL_HAPTIC_SINE;
  m_effect.periodic.period = RUMBLE_PERIOD;
  m_effect.periodic.magnitude = (Sint16)(state * 0x7FFF);
  m_effect.periodic.offset = 0;
  m_effect.periodic.phase = 18000;
  m_effect.periodic.length = RUMBLE_LENGTH_MAX;
  m_effect.periodic.delay = 0;
  m_effect.periodic.attack_length = 0;
}

void Joystick::TriangleEffect::SetSDLHapticEffect(ControlState state)
{
  m_effect.type = SDL_HAPTIC_TRIANGLE;
  m_effect.periodic.period = RUMBLE_PERIOD;
  m_effect.periodic.magnitude = (Sint16)(state * 0x7FFF);
  m_effect.periodic.offset = 0;
  m_effect.periodic.phase = 18000;
  m_effect.periodic.length = RUMBLE_LENGTH_MAX;
  m_effect.periodic.delay = 0;
  m_effect.periodic.attack_length = 0;
}

void Joystick::LeftRightEffect::SetSDLHapticEffect(ControlState state)
{
  m_effect.type = SDL_HAPTIC_LEFTRIGHT;
  m_effect.leftright.length = RUMBLE_LENGTH_MAX;
  // max ranges tuned to 'feel' similar in magnitude to triangle/sine on xbox360 controller
  m_effect.leftright.large_magnitude = (Uint16)(state * 0x4000);
  m_effect.leftright.small_magnitude = (Uint16)(state * 0xFFFF);
}
#endif

void Joystick::UpdateInput()
{
  // each joystick is doin this, o well
  SDL_JoystickUpdate();
}

std::string Joystick::GetName() const
{
  return m_name;
}

std::string Joystick::GetSource() const
{
  return "SDL";
}

SDL_Joystick* Joystick::GetSDLJoystick() const
{
  return m_joystick;
}

std::string Joystick::Button::GetName() const
{
  std::ostringstream ss;
  ss << "Button " << (int)m_index;
  return ss.str();
}

std::string Joystick::Axis::GetName() const
{
  std::ostringstream ss;
  ss << "Axis " << (int)m_index << (m_range < 0 ? '-' : '+');
  return ss.str();
}

std::string Joystick::Hat::GetName() const
{
  static char tmpstr[] = "Hat . .";
  // I don't think more than 10 hats are supported
  tmpstr[4] = (char)('0' + m_index);
  tmpstr[6] = "NESW"[m_direction];
  return tmpstr;
}

ControlState Joystick::Button::GetState() const
{
  return SDL_JoystickGetButton(m_js, m_index);
}

ControlState Joystick::Axis::GetState() const
{
  return std::max(0.0, ControlState(SDL_JoystickGetAxis(m_js, m_index)) / m_range);
}

ControlState Joystick::Hat::GetState() const
{
  return (SDL_JoystickGetHat(m_js, m_index) & (1 << m_direction)) > 0;
}
}
}
