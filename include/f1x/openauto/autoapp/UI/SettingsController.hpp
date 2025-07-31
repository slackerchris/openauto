/*
*  This file is part of openauto project.
*  Copyright (C) 2018 f1x.studio (Michal Szwaj)
*
*  openauto is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.

*  openauto is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * MODERNIZATION NOTE:
 * Created: August 1, 2025
 * Author: GitHub Copilot
 * Purpose: Part of MainWindow.cpp refactoring/modernization effort
 * 
 * This file was created as part of a major architectural refactoring to decompose
 * the monolithic MainWindow class (2,375+ lines) into specialized controllers.
 * This controller manages configuration and settings window lifecycle.
 * 
 * Refactoring Goals:
 * - Centralized configuration management
 * - Settings window lifecycle control
 * - Thread-safe configuration access
 * - MP3 and media settings coordination
 */

#pragma once

#include <memory>
#include <QObject>
#include <QMutex>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

class SettingsWindow;

/**
 * @brief Manages interactions with the SettingsWindow and configuration changes.
 * 
 * This class provides a clean interface for settings-related operations
 * that were previously embedded in MainWindow.
 */
class SettingsController : public QObject
{
    Q_OBJECT

public:
    explicit SettingsController(configuration::IConfiguration::Pointer configuration,
                               QObject* parent = nullptr);
    ~SettingsController() override;

    // Settings window management
    void setSettingsWindow(SettingsWindow* settingsWindow);
    void openSettings();
    void closeSettings();
    bool isSettingsOpen() const;

    // Configuration access
    void saveConfiguration();
    void loadConfiguration();
    void resetToDefaults();

    // Specific setting operations
    void updateMp3Settings(const QString& folder, const QString& subFolder, int track);
    void updateDisplaySettings(bool showCursor, bool showNetworkInfo);
    void updatePlayerSettings(bool autoPlay, bool instantPlay, bool buttonControl);
    void updateSystemSettings(bool hideWarning, bool enableBluetooth);

signals:
    void settingsChanged();
    void settingsWindowVisibilityChanged(bool visible);
    void configurationSaved();

private slots:
    void onConfigurationChanged();
    void onSettingsWindowClosed();

private:
    void connectToConfiguration();
    void disconnectFromConfiguration();

    configuration::IConfiguration::Pointer configuration_;
    SettingsWindow* settingsWindow_;
    bool settingsOpen_;
    
    mutable QMutex configurationMutex_;
};

}
}
}
}
