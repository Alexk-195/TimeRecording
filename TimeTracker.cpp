#include "TimeTracker.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <shellapi.h>
#include <iostream>


// Debug helper function for Windows GUI apps
void DebugOutput(const std::string& message) {
    #ifdef _DEBUG
    // Write to debug output (visible in Visual Studio Output window)
    OutputDebugStringA((message + "\n").c_str());

    // Also write to a debug log file
    std::ofstream debugFile("debug_log.txt", std::ios::app);
    debugFile << message << std::endl;
    debugFile.close();
    #endif
}

// Window procedure for summary dialog
LRESULT CALLBACK SummaryDialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            return 0;
        case WM_DESTROY:
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
                DestroyWindow(hWnd);
                return 0;
            }
            break;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

TimeTracker::TimeTracker(Localization* loc)
    : minutesHibernation(0), isArrived(false), localization(loc) {
}

void TimeTracker::Initialize(HWND hwnd) {
    hWnd = hwnd;
    CreateControls();
    CheckCrashRecovery();
    Arrive();
    lastActiveTime = arriveTime;
    SetTimer(hWnd, ID_TIMER, TIMER_INTERVAL, NULL);
    OnTimer(); // Initial update
}

HWND TimeTracker::CreateTextControl(const std::wstring& text, int x, int y, int width, int height, DWORD style) {
    return CreateWindowW(L"STATIC", text.c_str(),
        style, x, y, width, height,
        hWnd, NULL, GetModuleHandle(NULL), NULL);
}

HWND TimeTracker::CreateButtonControl(const std::wstring& text, int x, int y, int width, int height,
                                     int controlId, DWORD style) {
    return CreateWindowW(L"BUTTON", text.c_str(),
        style, x, y, width, height,
        hWnd, (HMENU)controlId, GetModuleHandle(NULL), NULL);
}

void TimeTracker::SetControlFont(HWND hControl, int fontSize, bool bold, const wchar_t* fontName) {
    HFONT hFont = CreateFontW(fontSize, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, fontName);
    SendMessage(hControl, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void TimeTracker::SetButtonFont(HWND hButton) {
    SetControlFont(hButton, 16, true, L"Arial");
  }

void TimeTracker::CreateControls() {
    // Arrival time label
    hLabelArrivalCaption = CreateTextControl(localization->Get("ARRIVAL_LABEL"), 20, 20, 240, 25);

    hLabelArrival = CreateTextControl(localization->Get("DEFAULT_ARRIVAL"), 20, 45, 240, 30);

    // Current time worked label
    hLabelTime = CreateTextControl(localization->Get("DEFAULT_TIME"), 20, 85, 240, 40);

    // Buttons
    hBtnArrive = CreateButtonControl(localization->Get("BTN_ARRIVE"), 20, 140, 240, 35, ID_BTN_ARRIVE);
    hBtnLeave = CreateButtonControl(localization->Get("BTN_LEAVE"), 20, 180, 240, 35, ID_BTN_LEAVE);
    hBtnDaily = CreateButtonControl(localization->Get("BTN_DAILY_SUMMARY"), 20, 230, 240, 35, ID_BTN_DAILY);
    hBtnWeekly = CreateButtonControl(localization->Get("BTN_WEEKLY_SUMMARY"), 20, 270, 240, 35, ID_BTN_WEEKLY);
    hBtnOpenLog = CreateButtonControl(localization->Get("BTN_OPEN_LOG"), 20, 320, 240, 35, ID_BTN_OPENLOG);
    hBtnAbout = CreateButtonControl(localization->Get("BTN_INFO"), 20, 360, 240, 35, ID_BTN_ABOUT);
    hBtnClose = CreateButtonControl(localization->Get("BTN_CLOSE"), 20, 400, 240, 35, ID_BTN_CLOSE);

    // Set fonts
    SetControlFont(hLabelArrivalCaption, 20, true, L"Arial");
    SetControlFont(hLabelTime, 40, true, L"Arial");
    SetControlFont(hLabelArrival, 30, false, L"Arial");
    SetButtonFont(hBtnArrive);
    SetButtonFont(hBtnLeave);
    SetButtonFont(hBtnDaily);
    SetButtonFont(hBtnWeekly);
    SetButtonFont(hBtnOpenLog);
    SetButtonFont(hBtnAbout);
    SetButtonFont(hBtnClose);
}

void TimeTracker::CheckCrashRecovery() {
    std::ifstream tmp(filenameTmp);
    if (tmp.good()) {
        tmp.close();
        std::ifstream tmpRead(filenameTmp);
        std::ofstream log(filename, std::ios::app);
        log << tmpRead.rdbuf();
        tmpRead.close();
        log.close();
        DeleteFileA(filenameTmp.c_str());
    }
}

void TimeTracker::OnTimer() {
    auto currentTime = std::chrono::system_clock::now();
    auto secondsSinceLastTimer = std::chrono::duration_cast<std::chrono::seconds>(
        currentTime - lastActiveTime).count();

    if (secondsSinceLastTimer > HIBERNATION_THRESHOLD) {
        // Hibernation detected
        WriteEvent(lastActiveTime, filename, localization->GetLogEvent("LOG_LEAVE_HIBERNATION"));
        WriteEvent(currentTime, filename, localization->GetLogEvent("LOG_ARRIVE_HIBERNATION"));

        minutesHibernation += std::chrono::duration_cast<std::chrono::minutes>(
            currentTime - lastActiveTime).count();
    }

    if (isArrived) {
        uint64_t minsActive = std::chrono::duration_cast<std::chrono::minutes>(
            currentTime - arriveTime).count();
        minsActive -= minutesHibernation;

        // Update time display
        std::wstring timeStr = std::to_wstring(minsActive / 60) + L":" +
            (minsActive % 60 < 10 ? L"0" : L"") + std::to_wstring(minsActive % 60);
        SetWindowTextW(hLabelTime, timeStr.c_str());
    }

    // Write to temp file for crash recovery
    WriteEvent(currentTime, filenameTmp, localization->GetLogEvent("LOG_LEAVE_TERMINATED"), false);

    lastActiveTime = currentTime;
}

void TimeTracker::Arrive() {
    arriveTime = std::chrono::system_clock::now();
    WriteEvent(arriveTime, filename, localization->GetLogEvent("LOG_ARRIVE"));

    std::wstring arrivalStr = TimeToWString(arriveTime);
    SetWindowTextW(hLabelArrival, arrivalStr.c_str());

    EnableWindow(hBtnArrive, FALSE);
    EnableWindow(hBtnLeave, TRUE);
    isArrived = true;
    minutesHibernation = 0;
}

void TimeTracker::Leave() {
    WriteEvent(std::chrono::system_clock::now(), filename, localization->GetLogEvent("LOG_LEAVE"));
    EnableWindow(hBtnArrive, TRUE);
    EnableWindow(hBtnLeave, FALSE);
    isArrived = false;
    SetWindowTextW(hLabelTime, localization->Get("DEFAULT_TIME").c_str());
}

void TimeTracker::ShowDailySummary() {
    ShowSummaryDialog(true);
}

void TimeTracker::ShowWeeklySummary() {
    ShowSummaryDialog(false);
}

void TimeTracker::ShowSummaryDialog(bool daily) {
    static bool dialogClassRegistered = false;

    // Register dialog window class if not already registered
    if (!dialogClassRegistered) {
        WNDCLASS wc = {};
        wc.lpfnWndProc = SummaryDialogProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.lpszClassName = "SummaryDialogClass";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

        if (RegisterClass(&wc)) {
            dialogClassRegistered = true;
        }
    }

    auto summary = daily ? GenerateDailySummary() : GenerateWeeklySummary();
    std::wstring title = daily ? localization->Get("DAILY_SUMMARY_TITLE") : localization->Get("WEEKLY_SUMMARY_TITLE");

    // Create proper dialog window
    HWND hDlg = CreateWindowW(L"SummaryDialogClass", title.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 300,
        hWnd, NULL, GetModuleHandle(NULL), NULL);

    if (hDlg) {
        // Create text area with proper sizing
        RECT clientRect;
        GetClientRect(hDlg, &clientRect);

        HWND hText = CreateWindowW(L"EDIT", std::wstring(summary.begin(), summary.end()).c_str(),
            WS_VISIBLE | WS_CHILD | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY,
            10, 10, clientRect.right - 20, clientRect.bottom - 60,
            hDlg, NULL, GetModuleHandle(NULL), NULL);

        // Create OK button
        HWND hOkButton = CreateWindowW(L"BUTTON", L"OK",
            WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            (clientRect.right - 80) / 2, clientRect.bottom - 40, 80, 30,
            hDlg, (HMENU)IDOK, GetModuleHandle(NULL), NULL);

        SetControlFont(hText, summaryFontSize, false, L"Courier New");
        SetButtonFont(hOkButton);

        // Set focus to the OK button
        SetFocus(hOkButton);
    }
}

std::string TimeTracker::WStringToString(const std::wstring& wstr) const {
    if (wstr.empty()) {
        return std::string();
    }

    // Use Windows API for conversion
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::string TimeTracker::GenerateDailySummary() {
    std::map<std::string, int> dailyMinutes;
    ParseLogFile(dailyMinutes, true);

    std::stringstream ss;
    std::wstring header = localization->Get("DAILY_SUMMARY_HEADER");
    ss << WStringToString(header) << " \r\n\r\n";

    std::wstring hoursText = localization->Get("HOURS");
    std::string hoursStr = WStringToString(hoursText);

    ss << "Number of entries: " << dailyMinutes.size() << "\r\n\r\n";

    for (const auto& entry : dailyMinutes) {
        int hours = entry.second / 60;
        int minutes = entry.second % 60;
        ss << entry.first << ": " << hours << ":"
           << std::setfill('0') << std::setw(2) << minutes << " " << hoursStr << "\r\n";
    }

    return ss.str();
}

std::string TimeTracker::GenerateWeeklySummary() {
    std::map<std::string, int> weeklyMinutes;
    ParseLogFile(weeklyMinutes, false);

    std::stringstream ss;
    std::wstring header = localization->Get("WEEKLY_SUMMARY_HEADER");
    ss << WStringToString(header) << " \r\n\r\n";

    std::wstring weekText = localization->Get("WEEK");
    std::string weekStr = WStringToString(weekText);
    std::wstring hoursText = localization->Get("HOURS");
    std::string hoursStr = WStringToString(hoursText);
    ss << "Number of entries: " << weeklyMinutes.size() << "\r\n\r\n";
    for (const auto& entry : weeklyMinutes) {
        int hours = entry.second / 60;
        int minutes = entry.second % 60;
        ss << weekStr << " " << entry.first << ": " << hours << ":"
           << std::setfill('0') << std::setw(2) << minutes << " " << hoursStr << "\r\n";
    }

    return ss.str();
}

void TimeTracker::ParseLogFile(std::map<std::string, int>& summary, bool daily) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        DebugOutput("Could not open log file: " + filename);
        return;
    }

    std::string line;
    std::chrono::system_clock::time_point arriveTime;
    bool arrived = false;
    int totalLines = 0;
    int processedArrivals = 0;
    int processedDepartures = 0;

    DebugOutput("Starting to parse log file: " + filename);

    while (std::getline(file, line)) {
        totalLines++;

        DebugOutput("Processing line " + std::to_string(totalLines) + ": " + line);

        // More flexible string matching - check if line contains ARRIVE or LEAVE
        bool isArriveEvent = (line.find("ARRIVE") != std::string::npos);
        bool isLeaveEvent = (line.find("LEAVE") != std::string::npos);

        if (isArriveEvent && !arrived) {
            arriveTime = ParseTimeFromLine(line);

            // Check if time parsing was successful
            auto epoch = std::chrono::system_clock::time_point{};
            if (arriveTime != epoch) {
                arrived = true;
                processedArrivals++;

                DebugOutput("Recorded arrival at: " + TimeToString(arriveTime));
            }
        }
        else if (isLeaveEvent && arrived) {
            auto leaveTime = ParseTimeFromLine(line);

            // Check if time parsing was successful
            auto epoch = std::chrono::system_clock::time_point{};
            if (leaveTime != epoch) {
                auto duration = std::chrono::duration_cast<std::chrono::minutes>(
                    leaveTime - arriveTime).count();

                // Only count positive durations
                if (duration > 0) {
                    std::string key = daily ? GetDateKey(arriveTime) : GetWeekKey(arriveTime);
                    summary[key] += duration;
                    processedDepartures++;

                    DebugOutput("Recorded departure at: " + TimeToString(leaveTime) +
                               ", Duration: " + std::to_string(duration) + " minutes, Key: " + key);
                }
                arrived = false;
            }
        }
    }

    file.close();

    DebugOutput("Summary: Total lines: " + std::to_string(totalLines) +
               ", Arrivals: " + std::to_string(processedArrivals) +
               ", Departures: " + std::to_string(processedDepartures) +
               ", Summary entries: " + std::to_string(summary.size()));
}

std::chrono::system_clock::time_point TimeTracker::ParseTimeFromLine(const std::string& line) {
    // Parse format: DD.MM.YYYY,HH:MM:SS,EVENT
    // Find the second comma to get the datetime part
    size_t firstComma = line.find(',');
    size_t secondComma = line.find(',', firstComma + 1);

    if (firstComma == std::string::npos || secondComma == std::string::npos) {
        DebugOutput("Invalid line format - missing commas: " + line);
        return std::chrono::system_clock::time_point{}; // Return epoch time on error
    }

    // Extract the datetime portion: "DD.MM.YYYY,HH:MM:SS"
    std::string dateTimeStr = line.substr(0, secondComma);

    DebugOutput("Parsing datetime: " + dateTimeStr);

    // Parse the date part (DD.MM.YYYY)
    std::string datePart = line.substr(0, firstComma);
    std::string timePart = line.substr(firstComma + 1, secondComma - firstComma - 1);

    DebugOutput("Date part: " + datePart + ", Time part: " + timePart);

    std::tm tm = {};

    // Parse date: DD.MM.YYYY
    std::istringstream dateStream(datePart);
    char dot1, dot2;
    int day, month, year;
    dateStream >> day >> dot1 >> month >> dot2 >> year;

    if (dateStream.fail() || dot1 != '.' || dot2 != '.') {
        DebugOutput("Failed to parse date components: " + datePart);
        return std::chrono::system_clock::time_point{};
    }

    // Parse time: HH:MM:SS
    std::istringstream timeStream(timePart);
    char colon1, colon2;
    int hour, minute, second;
    timeStream >> hour >> colon1 >> minute >> colon2 >> second;

    if (timeStream.fail() || colon1 != ':' || colon2 != ':') {
        DebugOutput("Failed to parse time components: " + timePart);
        return std::chrono::system_clock::time_point{};
    }

    // Set tm structure
    tm.tm_mday = day;
    tm.tm_mon = month - 1;  // months are 0-based
    tm.tm_year = year - 1900;  // years since 1900
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    tm.tm_isdst = -1; // Let system determine DST

    auto time_t_val = std::mktime(&tm);
    if (time_t_val == -1) {
        DebugOutput("mktime failed for: " + std::to_string(day) + "." + std::to_string(month) + "." +
                   std::to_string(year) + " " + std::to_string(hour) + ":" +
                   std::to_string(minute) + ":" + std::to_string(second));
        return std::chrono::system_clock::time_point{}; // Return epoch time on error
    }

    DebugOutput("Successfully parsed time: " + std::to_string(day) + "." + std::to_string(month) + "." +
               std::to_string(year) + " " + std::to_string(hour) + ":" +
               std::to_string(minute) + ":" + std::to_string(second));

    return std::chrono::system_clock::from_time_t(time_t_val);
}

std::string TimeTracker::GetDateKey(const std::chrono::system_clock::time_point& tp) {
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&tt);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << tm.tm_mday << "."
       << std::setfill('0') << std::setw(2) << (tm.tm_mon + 1) << "."
       << (tm.tm_year + 1900);
    return ss.str();
}

std::string TimeTracker::GetWeekKey(const std::chrono::system_clock::time_point& tp) {
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm = *std::localtime(&tt);

    // Calculate ISO week number more accurately
    int year = tm.tm_year + 1900;
    int dayOfYear = tm.tm_yday + 1; // tm_yday is 0-based
    int dayOfWeek = tm.tm_wday; // 0=Sunday, 1=Monday, etc.

    // Adjust for Monday as first day of week (ISO standard)
    if (dayOfWeek == 0) dayOfWeek = 7; // Sunday becomes 7

    // Calculate week number
    int weekNumber = (dayOfYear - dayOfWeek + 10) / 7;

    // Adjust for weeks that span year boundaries
    if (weekNumber == 0) {
        year--;
        weekNumber = 52; // or 53, but 52 is simpler
    } else if (weekNumber > 52) {
        // Check if we're actually in week 1 of next year
        year++;
        weekNumber = 1;
    }

    std::stringstream ss;
    ss << year << "-W" << std::setfill('0') << std::setw(2) << weekNumber;
    return ss.str();
}

void TimeTracker::OpenLog() {
    ShellExecuteW(NULL, L"open", std::wstring(filename.begin(), filename.end()).c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void TimeTracker::ShowAbout() {
    MessageBoxW(hWnd,
        localization->Get("ABOUT_TEXT").c_str(),
        localization->Get("ABOUT_TITLE").c_str(),
        MB_OK | MB_ICONINFORMATION);
}

void TimeTracker::Close() {
    // Send close message to the main window
    PostMessage(hWnd, WM_CLOSE, 0, 0);
}

void TimeTracker::OnDestroy() {
    WriteEvent(std::chrono::system_clock::now(), filename, localization->GetLogEvent("LOG_LEAVE_CLOSED"));
    KillTimer(hWnd, ID_TIMER);
    DeleteFileW(std::wstring(filenameTmp.begin(), filenameTmp.end()).c_str());
}

void TimeTracker::HandleCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case ID_BTN_ARRIVE:
            Arrive();
            break;
        case ID_BTN_LEAVE:
            Leave();
            break;
        case ID_BTN_DAILY:
            ShowDailySummary();
            break;
        case ID_BTN_WEEKLY:
            ShowWeeklySummary();
            break;
        case ID_BTN_OPENLOG:
            OpenLog();
            break;
        case ID_BTN_ABOUT:
            ShowAbout();
            break;
        case ID_BTN_CLOSE:
            Close();
            break;
    }
}

void TimeTracker::WriteEvent(const std::chrono::system_clock::time_point& t,
               const std::string& fname, const std::string& s, bool append) {
    std::ofstream ofs(fname, append ? std::ios::app : std::ios::out);
    ofs << TimeToString(t) << "," << s << std::endl;
    ofs.close();
}

std::string TimeTracker::TimeToString(const std::chrono::system_clock::time_point& t) {
    std::time_t tt = std::chrono::system_clock::to_time_t(t);
    std::tm tm = *std::localtime(&tt);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << tm.tm_mday << "."
       << std::setfill('0') << std::setw(2) << (tm.tm_mon + 1) << "."
       << (tm.tm_year + 1900) << ","
       << std::setfill('0') << std::setw(2) << tm.tm_hour << ":"
       << std::setfill('0') << std::setw(2) << tm.tm_min << ":"
       << std::setfill('0') << std::setw(2) << tm.tm_sec;
    return ss.str();
}

std::wstring TimeTracker::TimeToWString(const std::chrono::system_clock::time_point& t) {
    std::time_t tt = std::chrono::system_clock::to_time_t(t);
    std::tm tm = *std::localtime(&tt);
    std::wstringstream ss;
    ss << std::setfill(L'0') << std::setw(2) << tm.tm_hour << L":"
       << std::setfill(L'0') << std::setw(2) << tm.tm_min << L":"
       << std::setfill(L'0') << std::setw(2) << tm.tm_sec;
    return ss.str();
}