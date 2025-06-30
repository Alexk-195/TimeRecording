#ifndef TIMETRACKER_H
#define TIMETRACKER_H

#include <windows.h>
#include <string>
#include <chrono>
#include <map>
#include "localization.h"

// Control IDs
#define ID_TIMER 1
#define ID_BTN_ARRIVE 1001
#define ID_BTN_LEAVE 1002
#define ID_BTN_DAILY 1003
#define ID_BTN_WEEKLY 1004
#define ID_BTN_OPENLOG 1005
#define ID_BTN_ABOUT 1006
#define ID_BTN_CLOSE 1007

// Timer interval (60 seconds)
#define TIMER_INTERVAL 60000
#define HIBERNATION_THRESHOLD 120 // 2 minutes in seconds

class TimeTracker {
private:
    HWND hWnd;
    HWND hLabelArrivalCaption;
    HWND hLabelArrival;
    HWND hLabelTime;
    HWND hBtnArrive;
    HWND hBtnLeave;
    HWND hBtnDaily;
    HWND hBtnWeekly;
    HWND hBtnOpenLog;
    HWND hBtnAbout;
    HWND hBtnClose;

    std::chrono::system_clock::time_point arriveTime;
    std::chrono::system_clock::time_point lastActiveTime;
    uint32_t minutesHibernation;
    bool isArrived;

    const std::string filename = "Timelog.txt";
    const std::string filenameTmp = "Timelog_tmp.txt";

    int summaryFontSize = 14;  // Default font size

    Localization* localization;

    // Helper functions for UI creation
    HWND CreateTextControl(const std::wstring& text, int x, int y, int width, int height,
                          DWORD style = WS_VISIBLE | WS_CHILD | SS_CENTER);
    HWND CreateButtonControl(const std::wstring& text, int x, int y, int width, int height,
                            int controlId, DWORD style = WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON);
    void SetControlFont(HWND hControl, int fontSize = 16, bool bold = false,
                       const wchar_t* fontName = L"Arial");

    // Internal helper functions
    void WriteEvent(const std::chrono::system_clock::time_point& t,
                   const std::string& fname, const std::string& s, bool append = true);
    std::string TimeToString(const std::chrono::system_clock::time_point& t);
    std::wstring TimeToWString(const std::chrono::system_clock::time_point& t);
    std::string WStringToString(const std::wstring& wstr) const;

    // Log parsing functions
    std::chrono::system_clock::time_point ParseTimeFromLine(const std::string& line);
    std::string GetDateKey(const std::chrono::system_clock::time_point& tp);
    std::string GetWeekKey(const std::chrono::system_clock::time_point& tp);
    void ParseLogFile(std::map<std::string, int>& summary, bool daily);
    void SetButtonFont(HWND hButton);
public:
    TimeTracker(Localization* loc);
    ~TimeTracker() = default;

    // Main interface functions
    void Initialize(HWND hwnd);
    void CreateControls();
    void CheckCrashRecovery();
    void OnTimer();
    void HandleCommand(WPARAM wParam);
    void OnDestroy();

    // Action functions
    void Arrive();
    void Leave();
    void ShowDailySummary();
    void ShowWeeklySummary();
    void OpenLog();
    void ShowAbout();
    void Close();

    // Summary generation functions
    std::string GenerateDailySummary();
    std::string GenerateWeeklySummary();
    void ShowSummaryDialog(bool daily);
};

#endif // TIMETRACKER_H