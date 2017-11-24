// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "InputCommon/ControllerInterface/Win32/Win32.h"

#include <windows.h>

#include <thread>

#include "Common/Flag.h"
#include "Common/Logging/Log.h"
#include "Common/ScopeGuard.h"
#include "InputCommon/ControllerInterface/DInput/DInput.h"
#include "InputCommon/ControllerInterface/XInput/XInput.h"

constexpr UINT WM_DOLPHIN_STOP = WM_USER;

static HWND s_hwnd;
static HWND s_message_window;
static std::thread s_thread;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
  if (message == WM_INPUT_DEVICE_CHANGE)
  {
    ciface::DInput::PopulateDevices(s_hwnd);
    ciface::XInput::PopulateDevices();
  }

  return DefWindowProc(hwnd, message, wparam, lparam);
}

void ciface::Win32::Init(void* hwnd)
{
  s_hwnd = static_cast<HWND>(hwnd);
  XInput::Init();

  s_thread = std::thread([] {
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED)))
    {
      ERROR_LOG(SERIALINTERFACE, "CoInitializeEx failed: %i", GetLastError());
      return;
    }
    Common::ScopeGuard uninit([] { CoUninitialize(); });

    WNDCLASSEX window_class_info{};
    window_class_info.cbSize = sizeof(window_class_info);
    window_class_info.lpfnWndProc = WindowProc;
    window_class_info.hInstance = GetModuleHandle(nullptr);
    window_class_info.lpszClassName = L"Message";

    ATOM window_class = RegisterClassEx(&window_class_info);
    if (!window_class)
    {
      NOTICE_LOG(SERIALINTERFACE, "RegisterClassEx failed: %i", GetLastError());
      return;
    }
    Common::ScopeGuard unregister([&window_class] {
      if (!UnregisterClass(MAKEINTATOM(window_class), GetModuleHandle(nullptr)))
        ERROR_LOG(SERIALINTERFACE, "UnregisterClass failed: %i", GetLastError());
    });

    s_message_window = CreateWindowEx(0, L"Message", nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr,
                                      nullptr, nullptr);
    if (!s_message_window)
    {
      ERROR_LOG(SERIALINTERFACE, "CreateWindowEx failed: %i", GetLastError());
      return;
    }
    Common::ScopeGuard destroy([] {
      if (!DestroyWindow(s_message_window))
        ERROR_LOG(SERIALINTERFACE, "DestroyWindow failed: %i", GetLastError());
      s_message_window = nullptr;
    });

    std::array<RAWINPUTDEVICE, 2> devices;
    // game pad devices
    devices[0].usUsagePage = 0x01;
    devices[0].usUsage = 0x05;
    devices[0].dwFlags = RIDEV_DEVNOTIFY;
    devices[0].hwndTarget = s_message_window;
    // joystick devices
    devices[1].usUsagePage = 0x01;
    devices[1].usUsage = 0x04;
    devices[1].dwFlags = RIDEV_DEVNOTIFY;
    devices[1].hwndTarget = s_message_window;

    if (!RegisterRawInputDevices(devices.data(), static_cast<UINT>(devices.size()),
                                 static_cast<UINT>(sizeof(decltype(devices)::value_type))))
    {
      ERROR_LOG(SERIALINTERFACE, "RegisterRawInputDevices failed: %i", GetLastError());
      return;
    }

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0) > 0)
    {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
      if (msg.message == WM_DOLPHIN_STOP)
        break;
    }
  });
}

void ciface::Win32::PopulateDevices()
{
  if (s_thread.joinable())
    PostMessage(s_message_window, WM_INPUT_DEVICE_CHANGE, 0, 0);
}

void ciface::Win32::DeInit()
{
  NOTICE_LOG(SERIALINTERFACE, "win32 DeInit");
  if (s_thread.joinable())
  {
    PostMessage(s_message_window, WM_DOLPHIN_STOP, 0, 0);
    s_thread.join();
  }

  XInput::DeInit();
}
