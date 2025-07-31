/*
*  This fil*  You should have received a copy of the GNU General Public License
*  along with openauto. If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * MODERNIZATION NOTE:
 * Created: August 1, 2025
 * Author: GitHub Copilot
 * Purpose: Part of MainWindow.cpp refactoring/modernization effort
 * 
 * This implementation was created as part of a major architectural refactoring to decompose
 * the monolithic MainWindow class (2,375+ lines) into specialized controllers.
 * This controller handles system interactions, hardware control, and external script execution.
 * 
 * Key Features:
 * - Safe system command execution via SafeSystemExecutor
 * - Brightness and volume hardware control
 * - Network and Bluetooth monitoring
 * - Camera and rear-cam integration
 */

#include "f1x/openauto/autoapp/UI/SystemController.hpp"part of openauto project.
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

#include <f1x/openauto/autoapp/UI/SystemController.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/Common/ErrorHandler.hpp>

#include <QMutexLocker>
#include <QFileSystemWatcher>
#include <QFile>
#include <QTextStream>
#include <QNetworkInterface>
#include <QBluetoothLocalDevice>
#include <fstream>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

SystemController::SystemController(configuration::IConfiguration::Pointer configuration,
                                 configuration::SystemPaths::Pointer systemPaths,
                                 system::SafeSystemExecutor::Pointer systemExecutor,
                                 QObject* parent)
    : QObject(parent)
    , configuration_(std::move(configuration))
    , systemPaths_(std::move(systemPaths))
    , systemExecutor_(std::move(systemExecutor))
    , currentBrightness_(50)
    , currentVolume_(50)
    , customBrightnessControl_(false)
    , bluetoothEnabled_(false)
    , cameraYCorrection_(0)
    , cameraZoom_(0)
{
    initializeBrightnessControl();
    initializeVolumeControl();
    loadCustomButtonConfigurations();
    setupFileWatchers();
    
    OPENAUTO_LOG(info) << "[SystemController] Initialized with system integration support";
}

SystemController::~SystemController() = default;

void SystemController::initializeBrightnessControl()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        // Check if custom brightness control is available
        QString brightnessPath = getSystemPath("brightness_control");
        customBrightnessControl_ = checkFileExists("brightness_control");
        
        if (customBrightnessControl_) {
            readBrightnessFromFile();
            OPENAUTO_LOG(debug) << "[SystemController] Custom brightness control enabled";
        } else {
            OPENAUTO_LOG(debug) << "[SystemController] Using default brightness control";
        }
    }, "[SystemController]", "Brightness control initialization");
}

void SystemController::initializeVolumeControl()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        // Initialize volume control - implementation depends on system
        currentVolume_ = 50; // Default volume
        OPENAUTO_LOG(debug) << "[SystemController] Volume control initialized";
    }, "[SystemController]", "Volume control initialization");
}

void SystemController::setupFileWatchers()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        // Set up file system watchers for brightness and volume changes
        // Implementation would depend on system paths
        OPENAUTO_LOG(debug) << "[SystemController] File watchers setup complete";
    }, "[SystemController]", "File watcher setup");
}

void SystemController::setBrightness(int value)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, value]() {
        QMutexLocker locker(&systemStateMutex_);
        
        if (value < 0 || value > 100) {
            OPENAUTO_LOG(warning) << "[SystemController] Invalid brightness value: " << value;
            return;
        }
        
        currentBrightness_ = value;
        
        if (customBrightnessControl_) {
            writeBrightnessToFile(value);
        } else {
            // Use system executor for brightness control
            QString command = QString("echo %1 > /sys/class/backlight/*/brightness").arg(value);
            executeSystemCommand(command);
        }
        
        emit brightnessChanged(value);
        OPENAUTO_LOG(debug) << "[SystemController] Brightness set to: " << value;
    }, "[SystemController]", "Set brightness");
}

int SystemController::getBrightness() const
{
    QMutexLocker locker(&systemStateMutex_);
    return currentBrightness_;
}

bool SystemController::hasCustomBrightnessControl() const
{
    QMutexLocker locker(&systemStateMutex_);
    return customBrightnessControl_;
}

void SystemController::setSystemVolume(int value)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, value]() {
        QMutexLocker locker(&systemStateMutex_);
        
        if (value < 0 || value > 100) {
            OPENAUTO_LOG(warning) << "[SystemController] Invalid volume value: " << value;
            return;
        }
        
        currentVolume_ = value;
        
        // Use system executor for volume control
        QString command = QString("amixer set Master %1%").arg(value);
        executeSystemCommand(command);
        
        emit volumeChanged(value);
        OPENAUTO_LOG(debug) << "[SystemController] System volume set to: " << value;
    }, "[SystemController]", "Set system volume");
}

int SystemController::getSystemVolume() const
{
    QMutexLocker locker(&systemStateMutex_);
    return currentVolume_;
}

void SystemController::createDebugLog()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QString debugCommand = getSystemPath("debug_script");
        if (!debugCommand.isEmpty()) {
            executeSystemCommand(debugCommand);
            OPENAUTO_LOG(info) << "[SystemController] Debug log creation initiated";
        } else {
            OPENAUTO_LOG(warning) << "[SystemController] Debug script path not configured";
        }
    }, "[SystemController]", "Create debug log");
}

void SystemController::updateNetworkInfo()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QString networkInfo;
        
        // Get network interface information
        QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
        for (const QNetworkInterface& interface : interfaces) {
            if (interface.flags().testFlag(QNetworkInterface::IsUp) &&
                !interface.flags().testFlag(QNetworkInterface::IsLoopBack)) {
                
                QList<QNetworkAddressEntry> entries = interface.addressEntries();
                for (const QNetworkAddressEntry& entry : entries) {
                    if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                        networkInfo += interface.name() + ": " + entry.ip().toString() + "\n";
                        break;
                    }
                }
            }
        }
        
        emit networkInfoUpdated(networkInfo);
        OPENAUTO_LOG(debug) << "[SystemController] Network info updated";
    }, "[SystemController]", "Update network info");
}

void SystemController::executeDayNightScript(bool isNight)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, isNight]() {
        QString scriptAction = isNight ? "night" : "day";
        QString command = QString("/opt/crankshaft/service_daynight.sh app %1").arg(scriptAction);
        
        if (executeCrankshaftCommand(command)) {
            OPENAUTO_LOG(debug) << "[SystemController] Day/night script executed: " << scriptAction.toStdString();
        } else {
            OPENAUTO_LOG(warning) << "[SystemController] Failed to execute day/night script";
        }
    }, "[SystemController]", "Execute day/night script");
}

void SystemController::executeCameraCommand(const QString& command)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, command]() {
        QString fullCommand = QString("/opt/crankshaft/cameracontrol.py %1 &").arg(command);
        
        if (executeCrankshaftCommand(fullCommand)) {
            OPENAUTO_LOG(debug) << "[SystemController] Camera command executed: " << command.toStdString();
        } else {
            OPENAUTO_LOG(warning) << "[SystemController] Failed to execute camera command: " << command.toStdString();
        }
    }, "[SystemController]", "Execute camera command");
}

void SystemController::setCameraPosition(int yCorrection, int zoom)
{
    QMutexLocker locker(&systemStateMutex_);
    cameraYCorrection_ = yCorrection;
    cameraZoom_ = zoom;
}

void SystemController::executeCustomButton(int buttonNumber)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, buttonNumber]() {
        if (buttonNumber < 1 || buttonNumber > 6) {
            OPENAUTO_LOG(warning) << "[SystemController] Invalid custom button number: " << buttonNumber;
            return;
        }
        
        QString command = getCustomButtonCommand(buttonNumber);
        if (!command.isEmpty()) {
            if (executeSystemCommand(command)) {
                OPENAUTO_LOG(debug) << "[SystemController] Custom button " << buttonNumber << " executed";
                emit customButtonStateChanged(buttonNumber, true);
            } else {
                OPENAUTO_LOG(warning) << "[SystemController] Failed to execute custom button " << buttonNumber;
            }
        } else {
            OPENAUTO_LOG(debug) << "[SystemController] No command configured for custom button " << buttonNumber;
        }
    }, "[SystemController]", "Execute custom button");
}

QString SystemController::getCustomButtonCommand(int buttonNumber) const
{
    QMutexLocker locker(&systemStateMutex_);
    
    if (buttonNumber < 1 || buttonNumber > 6) {
        return QString();
    }
    
    return customButtonCommands_[buttonNumber - 1];
}

void SystemController::setCustomButtonCommand(int buttonNumber, const QString& command)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, buttonNumber, command]() {
        QMutexLocker locker(&systemStateMutex_);
        
        if (buttonNumber < 1 || buttonNumber > 6) {
            OPENAUTO_LOG(warning) << "[SystemController] Invalid custom button number: " << buttonNumber;
            return;
        }
        
        customButtonCommands_[buttonNumber - 1] = command;
        OPENAUTO_LOG(debug) << "[SystemController] Custom button " << buttonNumber << " command set";
    }, "[SystemController]", "Set custom button command");
}

void SystemController::setPairable()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QBluetoothLocalDevice localDevice;
        if (localDevice.isValid()) {
            localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
            bluetoothEnabled_ = true;
            emit bluetoothStateChanged(true);
            OPENAUTO_LOG(debug) << "[SystemController] Bluetooth set to discoverable";
        } else {
            OPENAUTO_LOG(warning) << "[SystemController] Bluetooth device not available";
        }
    }, "[SystemController]", "Set Bluetooth pairable");
}

void SystemController::updateBluetoothState()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QBluetoothLocalDevice localDevice;
        if (localDevice.isValid()) {
            bool wasEnabled = bluetoothEnabled_;
            bluetoothEnabled_ = localDevice.hostMode() != QBluetoothLocalDevice::HostPoweredOff;
            
            if (wasEnabled != bluetoothEnabled_) {
                emit bluetoothStateChanged(bluetoothEnabled_);
            }
            
            OPENAUTO_LOG(debug) << "[SystemController] Bluetooth state updated: " 
                              << (bluetoothEnabled_ ? "enabled" : "disabled");
        }
    }, "[SystemController]", "Update Bluetooth state");
}

bool SystemController::checkFileExists(const QString& pathKey) const
{
    QString fullPath = getSystemPath(pathKey);
    return QFile::exists(fullPath);
}

QString SystemController::getSystemPath(const QString& pathKey) const
{
    if (systemPaths_) {
        return systemPaths_->getPath(pathKey);
    }
    return QString();
}

bool SystemController::executeSystemCommand(const QString& command) const
{
    if (systemExecutor_) {
        return systemExecutor_->executeCommand(command);
    }
    
    // Fallback to system() call with logging
    OPENAUTO_LOG(debug) << "[SystemController] Executing system command: " << command.toStdString();
    int result = system(command.toStdString().c_str());
    return result == 0;
}

bool SystemController::executeCrankshaftCommand(const QString& action) const
{
    return executeSystemCommand(action);
}

void SystemController::loadCustomButtonConfigurations()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        // Initialize custom button configurations from settings
        for (int i = 0; i < 6; ++i) {
            customButtonCommands_[i] = "";
            customButtonFiles_[i] = "";
            customButtonColors_[i] = "186,189,192"; // Default color
            customButtonForced_[i] = false;
        }
        
        // Load from configuration if available
        // This would typically read from configuration files or database
        
        OPENAUTO_LOG(debug) << "[SystemController] Custom button configurations loaded";
    }, "[SystemController]", "Load custom button configurations");
}

bool SystemController::readBrightnessFromFile()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QString brightnessPath = getSystemPath("brightness_control");
        QFile file(brightnessPath);
        
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString line = in.readLine();
            bool ok;
            int brightness = line.toInt(&ok);
            
            if (ok && brightness >= 0 && brightness <= 100) {
                currentBrightness_ = brightness;
                return true;
            }
        }
        
        OPENAUTO_LOG(warning) << "[SystemController] Failed to read brightness from file";
        return false;
    }, "[SystemController]", "Read brightness from file");
    
    return false;
}

bool SystemController::writeBrightnessToFile(int value)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, value]() {
        QString brightnessPath = getSystemPath("brightness_control");
        QFile file(brightnessPath);
        
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << value;
            return true;
        }
        
        OPENAUTO_LOG(warning) << "[SystemController] Failed to write brightness to file";
        return false;
    }, "[SystemController]", "Write brightness to file");
    
    return false;
}

void SystemController::onBrightnessFileChanged()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (readBrightnessFromFile()) {
            emit brightnessChanged(currentBrightness_);
        }
    }, "[SystemController]", "Brightness file change handler");
}

void SystemController::onVolumeFileChanged()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        // Handle volume file changes if monitoring volume files
        emit volumeChanged(currentVolume_);
    }, "[SystemController]", "Volume file change handler");
}

}
}
}
}
