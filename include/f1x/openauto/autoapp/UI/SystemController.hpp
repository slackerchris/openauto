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
 * This controller handles system interactions, hardware control, and external script execution.
 * 
 * Refactoring Goals:
 * - System hardware interface management
 * - Brightness, volume, and camera controls
 * - Network and Bluetooth status monitoring
 * - Safe external command execution
 */

#pragma once

#include <memory>
#include <QObject>
#include <QMutex>
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
 * @brief Handles system interactions including brightness control,
 * volume control, debug logging, and external script execution.
 * 
 * This class encapsulates all system-level operations that were
 * previously mixed with UI logic in MainWindow.
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
    bool hasCustomBrightnessControl() const;

    // Volume control  
    void setSystemVolume(int value);
    int getSystemVolume() const;

    // System operations
    void createDebugLog();
    void updateNetworkInfo();
    void executeDayNightScript(bool isNight);

    // Camera operations
    void executeCameraCommand(const QString& command);
    void setCameraPosition(int yCorrection, int zoom);

    // Custom button operations
    void executeCustomButton(int buttonNumber);
    QString getCustomButtonCommand(int buttonNumber) const;
    void setCustomButtonCommand(int buttonNumber, const QString& command);

    // Bluetooth operations
    void setPairable();
    void updateBluetoothState();

    // System paths and file operations
    bool checkFileExists(const QString& pathKey) const;
    QString getSystemPath(const QString& pathKey) const;
    bool executeSystemCommand(const QString& command) const;
    bool executeCrankshaftCommand(const QString& action) const;

signals:
    void brightnessChanged(int value);
    void volumeChanged(int value);
    void networkInfoUpdated(const QString& info);
    void bluetoothStateChanged(bool enabled);
    void customButtonStateChanged(int buttonNumber, bool enabled);

private slots:
    void onBrightnessFileChanged();
    void onVolumeFileChanged();

private:
    void initializeBrightnessControl();
    void initializeVolumeControl();
    void setupFileWatchers();
    bool readBrightnessFromFile();
    bool writeBrightnessToFile(int value);
    void loadCustomButtonConfigurations();

    configuration::IConfiguration::Pointer configuration_;
    configuration::SystemPaths::Pointer systemPaths_;
    system::SafeSystemExecutor::Pointer systemExecutor_;

    // System state
    int currentBrightness_;
    int currentVolume_;
    bool customBrightnessControl_;
    bool bluetoothEnabled_;
    
    // Custom button configurations
    QString customButtonCommands_[6];
    QString customButtonFiles_[6]; 
    QString customButtonColors_[6];
    bool customButtonForced_[6];
    
    // Camera settings
    int cameraYCorrection_;
    int cameraZoom_;
    
    // Thread safety
    mutable QMutex systemStateMutex_;
};

}
}
}
}
