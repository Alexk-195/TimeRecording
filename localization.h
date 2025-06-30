#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <string>
#include <map>
#include <vector>
#include <codecvt>
#include <locale>

#pragma execution_character_set("utf-8")

class Localization {
private:
    std::string language;
    std::map<std::string, std::map<std::string, std::wstring>> translations;

    void InitializeTranslations() {
        // Window and UI elements
        translations["WINDOW_TITLE"]["de"] = L"Zeiterfassung";
        translations["WINDOW_TITLE"]["en"] = L"Time Tracking";

        translations["ARRIVAL_LABEL"]["de"] = L"Ankunft";
        translations["ARRIVAL_LABEL"]["en"] = L"Arrival";

        translations["BTN_ARRIVE"]["de"] = L"Kommen";
        translations["BTN_ARRIVE"]["en"] = L"Arrive";

        translations["BTN_LEAVE"]["de"] = L"Gehen";
        translations["BTN_LEAVE"]["en"] = L"Leave";

        translations["BTN_DAILY_SUMMARY"]["de"] = L"T\u00E4gliche Zusammenfassung";
        translations["BTN_DAILY_SUMMARY"]["en"] = L"Daily Summary";

        translations["BTN_WEEKLY_SUMMARY"]["de"] = L"W\u00F6chentliche Zusammenfassung";
        translations["BTN_WEEKLY_SUMMARY"]["en"] = L"Weekly Summary";

        translations["BTN_OPEN_LOG"]["de"] = L"Log \u00F6ffnen";
        translations["BTN_OPEN_LOG"]["en"] = L"Open Log";

        translations["BTN_INFO"]["de"] = L"Info";
        translations["BTN_INFO"]["en"] = L"About";

        translations["BTN_CLOSE"]["de"] = L"Schlie\u00DFen";
        translations["BTN_CLOSE"]["en"] = L"Close";

        // Dialog titles
        translations["DAILY_SUMMARY_TITLE"]["de"] = L"T\u00E4gliche Zusammenfassung";
        translations["DAILY_SUMMARY_TITLE"]["en"] = L"Daily Summary";

        translations["WEEKLY_SUMMARY_TITLE"]["de"] = L"W\u00F6chentliche Zusammenfassung";
        translations["WEEKLY_SUMMARY_TITLE"]["en"] = L"Weekly Summary";

        translations["ABOUT_TITLE"]["de"] = L"Info";
        translations["ABOUT_TITLE"]["en"] = L"About";

        // About dialog content
        translations["ABOUT_TEXT"]["de"] = L"Time Recording\n\n"
                                          L"Erfasst die aktive Zeit am PC.\n"
                                          L"https://github.com/Alexk-195/TimeRecording";

        translations["ABOUT_TEXT"]["en"] = L"Time Recording\n\n"
                                          L"Tracks active PC time.\n"
                                          L"https://github.com/Alexk-195/TimeRecording";

        // Summary headers
        translations["DAILY_SUMMARY_HEADER"]["de"] = L"=== TAEGLICHE ZUSAMMENFASSUNG ===";
        translations["DAILY_SUMMARY_HEADER"]["en"] = L"=== DAILY SUMMARY ===";

        translations["WEEKLY_SUMMARY_HEADER"]["de"] = L"=== WOECHENTLICHE ZUSAMMENFASSUNG ===";
        translations["WEEKLY_SUMMARY_HEADER"]["en"] = L"=== WEEKLY SUMMARY ===";

        // Time units
        translations["HOURS"]["de"] = L"Stunden";
        translations["HOURS"]["en"] = L"hours";

        translations["WEEK"]["de"] = L"Woche";
        translations["WEEK"]["en"] = L"Week";

        // Log event messages (these should remain consistent for log parsing)
        translations["LOG_ARRIVE"]["de"] = L"ARRIVE";
        translations["LOG_ARRIVE"]["en"] = L"ARRIVE";

        translations["LOG_LEAVE"]["de"] = L"LEAVE";
        translations["LOG_LEAVE"]["en"] = L"LEAVE";

        translations["LOG_LEAVE_HIBERNATION"]["de"] = L"LEAVE (app hibernation)";
        translations["LOG_LEAVE_HIBERNATION"]["en"] = L"LEAVE (app hibernation)";

        translations["LOG_ARRIVE_HIBERNATION"]["de"] = L"ARRIVE (from hibernation)";
        translations["LOG_ARRIVE_HIBERNATION"]["en"] = L"ARRIVE (from hibernation)";

        translations["LOG_LEAVE_CLOSED"]["de"] = L"LEAVE (app closed)";
        translations["LOG_LEAVE_CLOSED"]["en"] = L"LEAVE (app closed)";

        translations["LOG_LEAVE_TERMINATED"]["de"] = L"LEAVE (app forcefully terminated)";
        translations["LOG_LEAVE_TERMINATED"]["en"] = L"LEAVE (app forcefully terminated)";

        // Default time display
        translations["DEFAULT_TIME"]["de"] = L"0:00";
        translations["DEFAULT_TIME"]["en"] = L"0:00";

        translations["DEFAULT_ARRIVAL"]["de"] = L"00:00:00";
        translations["DEFAULT_ARRIVAL"]["en"] = L"00:00:00";

        // Error messages (if needed in future)
        translations["ERROR_FILE_NOT_FOUND"]["de"] = L"Datei nicht gefunden";
        translations["ERROR_FILE_NOT_FOUND"]["en"] = L"File not found";

        translations["ERROR_PARSE_LOG"]["de"] = L"Fehler beim Lesen der Log-Datei";
        translations["ERROR_PARSE_LOG"]["en"] = L"Error reading log file";

        // Status messages
        translations["STATUS_TRACKING"]["de"] = L"Zeiterfassung l\u00E4uft...";
        translations["STATUS_TRACKING"]["en"] = L"Time tracking active...";

        translations["STATUS_STOPPED"]["de"] = L"Zeiterfassung gestoppt";
        translations["STATUS_STOPPED"]["en"] = L"Time tracking stopped";
    }

    // Helper function to convert wstring to string safely
    std::string WStringToString(const std::wstring& wstr) const {
        if (wstr.empty()) {
            return std::string();
        }

        // Use Windows API for conversion on Windows platform
        #ifdef _WIN32
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
        return strTo;
        #else
        // Fallback for non-Windows platforms
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        return converter.to_bytes(wstr);
        #endif
    }

public:
    // Constructor accepts language code
    Localization(const std::string& lang = "de") : language(lang) {
        // Validate language, default to German if invalid
        if (lang != "de" && lang != "en") {
            language = "de";
        }
        InitializeTranslations();
    }

    // Get translated string by key
    std::wstring Get(const std::string& key) const {
        auto keyIt = translations.find(key);
        if (keyIt != translations.end()) {
            auto langIt = keyIt->second.find(language);
            if (langIt != keyIt->second.end()) {
                return langIt->second;
            }
            // Fallback to German if current language not found
            auto fallbackIt = keyIt->second.find("de");
            if (fallbackIt != keyIt->second.end()) {
                return fallbackIt->second;
            }
        }
        // Return key as fallback if translation not found
        return std::wstring(key.begin(), key.end());
    }

    // Get translated string for log events (returns std::string for file writing)
    std::string GetLogEvent(const std::string& key) const {
        std::wstring wstr = Get(key);
        return WStringToString(wstr);
    }

    // Get current language
    std::string GetLanguage() const {
        return language;
    }

    // Change language at runtime
    void SetLanguage(const std::string& lang) {
        if (lang == "de" || lang == "en") {
            language = lang;
        }
    }

    // Check if language is supported
    static bool IsLanguageSupported(const std::string& lang) {
        return (lang == "de" || lang == "en");
    }

    // Get list of supported languages
    static std::vector<std::string> GetSupportedLanguages() {
        return {"de", "en"};
    }
};

#endif // LOCALIZATION_H