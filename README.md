# Zeiterfassung / Time Tracking

A lightweight Windows desktop application for tracking work hours with automatic hibernation detection and crash recovery.

## Features

- **Simple Time Tracking**: One-click arrival and departure logging
- **Hibernation Detection**: Automatically detects and excludes system sleep/hibernation periods
- **Crash Recovery**: Recovers tracking state after unexpected application termination
- **Multi-language Support**: Available in German and English
- **Daily & Weekly Summaries**: View detailed time reports
- **Persistent Logging**: All data saved to local text file

## Screenshots

The application provides a clean, intuitive interface with large buttons for easy interaction.

## Installation

### Prerequisites
- Windows 10/11
- Visual Studio Build Tools with C++ support OR MinGW

### Building from Source

1. Clone the repository:
```bash
git clone https://github.com/alexk195/Zeiterfassung.git
cd Zeiterfassung
```

2. Compile using the provided batch file:
```bash
compile.bat
```

3. Run the executable:
```bash
TimeRecording.exe
```

## Usage

### Basic Operation
- Click **"Kommen"/"Arrive"** when starting work
- Click **"Gehen"/"Leave"** when finishing work
- The application automatically tracks active time and excludes hibernation periods

### Language Support
Launch with language parameter:
```bash
TimeRecording.exe en          # English
TimeRecording.exe de          # German (default)
TimeRecording.exe --language=en
```

### Reports
- **Daily Summary**: View hours worked per day
- **Weekly Summary**: View total hours per week
- **Open Log**: Access raw time log file

### Auto-Start (Optional)
1. Press `Win+R`, type `shell:startup`, press Enter
2. Copy `TimeRecording.exe` to the opened folder

## File Structure

- `main.cpp` - Application entry point and window management
- `TimeTracker.h/cpp` - Core time tracking logic
- `localization.h` - Multi-language support
- `compile.bat` - Build script for Visual Studio
- `Timelog.txt` - Generated time log file

## Technical Details

- **Language**: C++ with Win32 API
- **Architecture**: Single-threaded with timer-based updates
- **Data Storage**: Plain text CSV format
- **Hibernation Detection**: 2-minute inactivity threshold
- **Update Interval**: 60 seconds

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is open source. See repository for license details.

## Support

For issues or feature requests, please open an issue on GitHub.