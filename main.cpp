#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <iostream>
#include <string>
#include "localization.h"
#include "TimeTracker.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "shell32.lib")

// Window dimensions
#define WINDOW_WIDTH 300
#define WINDOW_HEIGHT 500

TimeTracker* g_pTracker = nullptr;
Localization* g_pLocalization = nullptr;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            g_pTracker = new TimeTracker(g_pLocalization);
            g_pTracker->Initialize(hWnd);
            break;

        case WM_COMMAND:
            if (g_pTracker) {
                g_pTracker->HandleCommand(wParam);
            }
            break;

        case WM_TIMER:
            if (g_pTracker) {
                g_pTracker->OnTimer();
            }
            break;

        case WM_DESTROY:
            if (g_pTracker) {
                g_pTracker->OnDestroy();
                delete g_pTracker;
                g_pTracker = nullptr;
            }
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

std::string ParseLanguageFromCommandLine(int argc, wchar_t* argv[]) {
   std::string language = "de"; // Default language

   for (int i = 1; i < argc; i++) {
       std::wstring arg(argv[i]);

       // Check for language parameter formats: -lang=XX, --language=XX, /lang:XX
       if (arg.find(L"-lang=") == 0 || arg.find(L"--language=") == 0) {
           size_t pos = arg.find(L'=');
           if (pos != std::wstring::npos && pos + 1 < arg.length()) {
               std::wstring langCode = arg.substr(pos + 1);
               if (langCode.length() == 2) {
                   // Safe conversion using Windows API
                   int size_needed = WideCharToMultiByte(CP_UTF8, 0, &langCode[0], (int)langCode.size(), NULL, 0, NULL, NULL);
                   std::string langStr(size_needed, 0);
                   WideCharToMultiByte(CP_UTF8, 0, &langCode[0], (int)langCode.size(), &langStr[0], size_needed, NULL, NULL);

                   if (Localization::IsLanguageSupported(langStr)) {
                       language = langStr;
                   }
               }
           }
       }
       else if (arg.find(L"/lang:") == 0) {
           if (arg.length() >= 7) {
               std::wstring langCode = arg.substr(6);
               if (langCode.length() == 2) {
                   // Safe conversion using Windows API
                   int size_needed = WideCharToMultiByte(CP_UTF8, 0, &langCode[0], (int)langCode.size(), NULL, 0, NULL, NULL);
                   std::string langStr(size_needed, 0);
                   WideCharToMultiByte(CP_UTF8, 0, &langCode[0], (int)langCode.size(), &langStr[0], size_needed, NULL, NULL);

                   if (Localization::IsLanguageSupported(langStr)) {
                       language = langStr;
                   }
               }
           }
       }
       // Simple format: just "en" or "de"
       else if (arg.length() == 2) {
           // Safe conversion using Windows API
           int size_needed = WideCharToMultiByte(CP_UTF8, 0, &arg[0], (int)arg.size(), NULL, 0, NULL, NULL);
           std::string langStr(size_needed, 0);
           WideCharToMultiByte(CP_UTF8, 0, &arg[0], (int)arg.size(), &langStr[0], size_needed, NULL, NULL);

           if (Localization::IsLanguageSupported(langStr)) {
               language = langStr;
           }
       }
   }

   return language;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Parse command line for language
    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::string language = ParseLanguageFromCommandLine(argc, argv);
    LocalFree(argv);

    // Initialize localization
    g_pLocalization = new Localization(language);

    // Initialize common controls
    InitCommonControls();

    // Register window class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TimeRecordingClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClass(&wc);

    // Create window with localized title
    HWND hWnd = CreateWindowW(
        L"TimeRecordingClass",
        g_pLocalization->Get("WINDOW_TITLE").c_str(),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (!hWnd) {
        delete g_pLocalization;
        return 1;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    delete g_pLocalization;
    g_pLocalization = nullptr;

    return 0;
}