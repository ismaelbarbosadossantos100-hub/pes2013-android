/**
 * @file        ui/windowed_app_main_sdl.cpp
 * @brief       Entry point for windowed applications (SDL3 windowing)
 *
 * @copyright   Copyright (c) 2026 Tom Clay <tomc@tctechstuff.com>
 *              All rights reserved.
 *
 * @license     BSD 3-Clause License
 *              See LICENSE file in the project root for full license text.
 */

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <rex/cvar.h>
#include <rex/logging.h>
#include <rex/platform.h>
#include <rex/ui/windowed_app.h>
#include <rex/ui/windowed_app_context_sdl.h>

#if defined(__ANDROID__)
#include <android/log.h>
#define PES2013_NATIVE_LOG(...) \
  __android_log_print(ANDROID_LOG_INFO, "PES2013_NATIVE", __VA_ARGS__)
#else
#define PES2013_NATIVE_LOG(...) ((void)0)
#endif

#if REX_PLATFORM_WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <objbase.h>
#include <shellapi.h>
#endif

namespace {

std::string g_android_diagnostic_file;

void WriteAndroidDiagnostic(const std::string& message) {
#if defined(__ANDROID__)
  if (g_android_diagnostic_file.empty()) {
    return;
  }
  std::ofstream output(g_android_diagnostic_file, std::ios::app);
  if (output) {
    output << "NATIVE_STAGE " << message << '\n';
    output.flush();
  }
#else
  (void)message;
#endif
}

void DiscoverAndroidDiagnosticFile(int argc, char** argv) {
#if defined(__ANDROID__)
  for (int i = 0; i < argc; ++i) {
    const std::string argument = argv[i] ? argv[i] : "";
    constexpr std::string_view kLogFilePrefix = "--log_file=";
    constexpr std::string_view kLogFileDashPrefix = "--log-file=";
    if (argument.rfind(kLogFilePrefix, 0) == 0) {
      g_android_diagnostic_file = argument.substr(kLogFilePrefix.size());
    } else if (argument.rfind(kLogFileDashPrefix, 0) == 0) {
      g_android_diagnostic_file = argument.substr(kLogFileDashPrefix.size());
    }
  }
#else
  (void)argc;
  (void)argv;
#endif
}

int RunWindowedApp(int argc, char** argv) {
  DiscoverAndroidDiagnosticFile(argc, argv);
  WriteAndroidDiagnostic("RUN_ENTER");
  PES2013_NATIVE_LOG("RUN_ENTER argc=%d", argc);
  auto remaining = rex::cvar::Init(argc, argv);
  WriteAndroidDiagnostic("CVAR_INIT");
  PES2013_NATIVE_LOG("CVAR_INIT remaining=%zu", remaining.size());
  rex::cvar::ApplyEnvironment();
  rex::InitLoggingEarly();
  WriteAndroidDiagnostic("LOGGING_EARLY_READY");
  PES2013_NATIVE_LOG("LOGGING_EARLY_READY");

  int result;
  {
    rex::ui::SDLWindowedAppContext app_context;
    if (!app_context.Initialize()) {
      WriteAndroidDiagnostic("SDL_CONTEXT_INIT_FAILED");
      PES2013_NATIVE_LOG("SDL_CONTEXT_INIT_FAILED");
      return EXIT_FAILURE;
    }
    WriteAndroidDiagnostic("SDL_CONTEXT_READY");
    PES2013_NATIVE_LOG("SDL_CONTEXT_READY");

#if REX_PLATFORM_WIN32
    // Apartment-threaded COM for shell dialogs.
    if (FAILED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED))) {
      return EXIT_FAILURE;
    }
#endif

#if XE_UI_WINDOWED_APPS_IN_LIBRARY
    auto creator = rex::ui::WindowedApp::GetCreator("pes2013");
    if (!creator) {
      REXLOG_ERROR("No registered windowed application creator found");
      return EXIT_FAILURE;
    }
    std::unique_ptr<rex::ui::WindowedApp> app = creator(app_context);
#else
    std::unique_ptr<rex::ui::WindowedApp> app = rex::ui::GetWindowedAppCreator()(app_context);
#endif
    WriteAndroidDiagnostic("APP_CREATOR_READY");
    PES2013_NATIVE_LOG("APP_CREATOR_READY app=%p", static_cast<void*>(app.get()));

    // Match remaining positional args to the app's expected options.
    const auto& option_names = app->GetPositionalOptions();
    std::map<std::string, std::string> parsed;
    size_t count = std::min(remaining.size(), option_names.size());
    for (size_t i = 0; i < count; ++i) {
      parsed[option_names[i]] = remaining[i];
    }
    app->SetParsedArguments(std::move(parsed));

    const bool initialized = app->OnInitialize();
    WriteAndroidDiagnostic(initialized ? "APP_INITIALIZE_RESULT=1" : "APP_INITIALIZE_RESULT=0");
    PES2013_NATIVE_LOG("APP_INITIALIZE_RESULT value=%d", initialized ? 1 : 0);
    result = initialized ? app_context.RunMainMessageLoop() : EXIT_FAILURE;
    WriteAndroidDiagnostic("MAIN_LOOP_RETURN");
    PES2013_NATIVE_LOG("MAIN_LOOP_RETURN result=%d", result);

    app->InvokeOnDestroy();
  }

#if REX_PLATFORM_WIN32
  CoUninitialize();
#endif

  return result;
}

#if REX_PLATFORM_WIN32
// Convert wide argv from CommandLineToArgvW to UTF-8 for cvar::Init.
std::vector<std::string> WideArgsToUtf8(int argc, wchar_t** wargv) {
  std::vector<std::string> args;
  args.reserve(static_cast<size_t>(argc));
  for (int i = 0; i < argc; ++i) {
    std::wstring wide(wargv[i]);
    if (wide.empty()) {
      args.emplace_back();
      continue;
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr,
                                   0, nullptr, nullptr);
    std::string utf8(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), utf8.data(), size,
                        nullptr, nullptr);
    args.push_back(std::move(utf8));
  }
  return args;
}
#endif

}  // namespace

#if REX_PLATFORM_WIN32

int WINAPI wWinMain(HINSTANCE hinstance, HINSTANCE hinstance_prev, LPWSTR command_line,
                    int show_cmd) {
  (void)hinstance;
  (void)hinstance_prev;
  (void)command_line;
  (void)show_cmd;

  int wargc = 0;
  wchar_t** wargv = CommandLineToArgvW(GetCommandLineW(), &wargc);
  auto utf8_args = WideArgsToUtf8(wargc, wargv);
  LocalFree(wargv);

  std::vector<char*> argv_ptrs;
  argv_ptrs.reserve(utf8_args.size());
  for (auto& s : utf8_args) {
    argv_ptrs.push_back(s.data());
  }
  return RunWindowedApp(static_cast<int>(argv_ptrs.size()), argv_ptrs.data());
}

#else

int main(int argc, char* argv[]) {
  return RunWindowedApp(argc, argv);
}

#endif
