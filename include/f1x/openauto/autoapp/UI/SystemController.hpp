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

#pragma once

#include <QObject>
#include <QFile>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Configuration/SystemPaths.hpp>
#include <f1x/openauto/autoapp/System/SafeSystemExecutor.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

/**
 * @brief System Controller for hardware and UI state management
 * 
 * Handles brightness control, volume control, and day/night mode switching.
 * Extracted from MainWindow to provide better separation of concerns.
 */
class SystemController : public QObject
{
    Q_OBJECT

public:
    explicit SystemController(configuration::IConfiguration::Pointer configuration,
                            configuration::SystemPaths::Pointer systemPaths,
                            system::SafeSystemExecutor::Pointer systemExecutor,
                            QObject* parent = nullptr);
    ~SystemController() override;

    // Brightness control
    void setBrightness(int value);
    int getBrightness() const;
    bool isBrightnessControlAvailable() const;
    
    // Volume control
    void setVolume(int value);
    int getVolume() const;
    
    // Theme switching
    void switchToNightMode();
    void switchToDayMode();
    bool isNightMode() const;
    
    // System state
    void setMute(bool muted);
    bool isMuted() const;

signals:
    void brightnessChanged(int value);
    void volumeChanged(int value);
    void themeChanged(bool isNightMode);
    void muteStateChanged(bool isMuted);

private:
    // Helper methods
    void initializeBrightnessControl();
    void updateBrightnessFiles();
    bool checkFileExists(const QString& pathKey) const;
    QString getSystemPath(const QString& pathKey) const;
    bool executeSystemCommand(const QString& command) const;

private:
    configuration::IConfiguration::Pointer configuration_;
    configuration::SystemPaths::Pointer systemPaths_;
    system::SafeSystemExecutor::Pointer systemExecutor_;
    
    // Brightness control files
    QFile* brightnessFile_;
    QFile* brightnessFileAlt_;
    bool customBrightnessControl_;
    
    // State tracking
    int currentBrightness_;
    int currentVolume_;
    bool isNightMode_;
    bool isMuted_;
};

}
}
}
}
