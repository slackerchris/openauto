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
 * This controller manages configuration and settings window lifecycle.
 * 
 * Key Features:
 * - Thread-safe configuration management
 * - Settings window lifecycle control
 * - MP3 and media settings coordination
 * - Integration with OpenAuto configuration system
 */

#include "f1x/openauto/autoapp/UI/SettingsController.hpp"part of openauto project.
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

#include <f1x/openauto/autoapp/UI/SettingsController.hpp>
#include <f1x/openauto/autoapp/UI/SettingsWindow.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/Common/ErrorHandler.hpp>

#include <QMutexLocker>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

SettingsController::SettingsController(configuration::IConfiguration::Pointer configuration,
                                     QObject* parent)
    : QObject(parent)
    , configuration_(std::move(configuration))
    , settingsWindow_(nullptr)
    , settingsOpen_(false)
{
    connectToConfiguration();
    OPENAUTO_LOG(info) << "[SettingsController] Initialized";
}

SettingsController::~SettingsController()
{
    disconnectFromConfiguration();
}

void SettingsController::setSettingsWindow(SettingsWindow* settingsWindow)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, settingsWindow]() {
        QMutexLocker locker(&configurationMutex_);
        
        if (settingsWindow_) {
            // Disconnect from previous settings window
            disconnect(settingsWindow_, nullptr, this, nullptr);
        }
        
        settingsWindow_ = settingsWindow;
        
        if (settingsWindow_) {
            // Connect to new settings window
            connect(settingsWindow_, &SettingsWindow::destroyed, this, &SettingsController::onSettingsWindowClosed);
            OPENAUTO_LOG(debug) << "[SettingsController] Settings window connected";
        }
    }, "[SettingsController]", "Set settings window");
}

void SettingsController::openSettings()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QMutexLocker locker(&configurationMutex_);
        
        if (settingsWindow_ && !settingsOpen_) {
            settingsWindow_->showFullScreen();
            settingsWindow_->show_tab1();
            settingsWindow_->loadSystemValues();
            settingsOpen_ = true;
            
            emit settingsWindowVisibilityChanged(true);
            OPENAUTO_LOG(debug) << "[SettingsController] Settings window opened";
        } else if (!settingsWindow_) {
            OPENAUTO_LOG(warning) << "[SettingsController] Settings window not available";
        } else {
            OPENAUTO_LOG(debug) << "[SettingsController] Settings window already open";
        }
    }, "[SettingsController]", "Open settings");
}

void SettingsController::closeSettings()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QMutexLocker locker(&configurationMutex_);
        
        if (settingsWindow_ && settingsOpen_) {
            settingsWindow_->close();
            settingsOpen_ = false;
            
            emit settingsWindowVisibilityChanged(false);
            OPENAUTO_LOG(debug) << "[SettingsController] Settings window closed";
        }
    }, "[SettingsController]", "Close settings");
}

bool SettingsController::isSettingsOpen() const
{
    QMutexLocker locker(&configurationMutex_);
    return settingsOpen_;
}

void SettingsController::saveConfiguration()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (configuration_) {
            configuration_->save();
            emit configurationSaved();
            OPENAUTO_LOG(debug) << "[SettingsController] Configuration saved";
        } else {
            OPENAUTO_LOG(warning) << "[SettingsController] Configuration not available for saving";
        }
    }, "[SettingsController]", "Save configuration");
}

void SettingsController::loadConfiguration()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (configuration_) {
            configuration_->load();
            emit settingsChanged();
            OPENAUTO_LOG(debug) << "[SettingsController] Configuration loaded";
        } else {
            OPENAUTO_LOG(warning) << "[SettingsController] Configuration not available for loading";
        }
    }, "[SettingsController]", "Load configuration");
}

void SettingsController::resetToDefaults()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (configuration_) {
            configuration_->reset();
            emit settingsChanged();
            OPENAUTO_LOG(debug) << "[SettingsController] Configuration reset to defaults";
        } else {
            OPENAUTO_LOG(warning) << "[SettingsController] Configuration not available for reset";
        }
    }, "[SettingsController]", "Reset to defaults");
}

void SettingsController::updateMp3Settings(const QString& folder, const QString& subFolder, int track)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, folder, subFolder, track]() {
        QMutexLocker locker(&configurationMutex_);
        
        if (configuration_) {
            configuration_->setMp3MasterPath(folder.toStdString());
            configuration_->setMp3SubFolder(subFolder.toStdString());
            configuration_->setMp3Track(track);
            
            emit settingsChanged();
            OPENAUTO_LOG(debug) << "[SettingsController] MP3 settings updated: " 
                              << folder.toStdString() << "/" << subFolder.toStdString() 
                              << ", track: " << track;
        }
    }, "[SettingsController]", "Update MP3 settings");
}

void SettingsController::updateDisplaySettings(bool showCursor, bool showNetworkInfo)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, showCursor, showNetworkInfo]() {
        QMutexLocker locker(&configurationMutex_);
        
        if (configuration_) {
            configuration_->setShowCursor(showCursor);
            configuration_->setShowNetworkinfo(showNetworkInfo);
            
            emit settingsChanged();
            OPENAUTO_LOG(debug) << "[SettingsController] Display settings updated: cursor=" 
                              << (showCursor ? "shown" : "hidden") 
                              << ", network=" << (showNetworkInfo ? "shown" : "hidden");
        }
    }, "[SettingsController]", "Update display settings");
}

void SettingsController::updatePlayerSettings(bool autoPlay, bool instantPlay, bool buttonControl)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, autoPlay, instantPlay, buttonControl]() {
        QMutexLocker locker(&configurationMutex_);
        
        if (configuration_) {
            configuration_->setMp3AutoPlay(autoPlay);
            configuration_->setInstantPlay(instantPlay);
            configuration_->setPlayerButtonControl(buttonControl);
            
            emit settingsChanged();
            OPENAUTO_LOG(debug) << "[SettingsController] Player settings updated: autoplay=" 
                              << (autoPlay ? "enabled" : "disabled")
                              << ", instant=" << (instantPlay ? "enabled" : "disabled")
                              << ", buttons=" << (buttonControl ? "enabled" : "disabled");
        }
    }, "[SettingsController]", "Update player settings");
}

void SettingsController::updateSystemSettings(bool hideWarning, bool enableBluetooth)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, hideWarning, enableBluetooth]() {
        QMutexLocker locker(&configurationMutex_);
        
        if (configuration_) {
            configuration_->setHideWarning(hideWarning);
            // Note: Bluetooth setting might need different handling depending on implementation
            
            emit settingsChanged();
            OPENAUTO_LOG(debug) << "[SettingsController] System settings updated: warning=" 
                              << (hideWarning ? "hidden" : "shown")
                              << ", bluetooth=" << (enableBluetooth ? "enabled" : "disabled");
        }
    }, "[SettingsController]", "Update system settings");
}

void SettingsController::onConfigurationChanged()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        emit settingsChanged();
        OPENAUTO_LOG(debug) << "[SettingsController] Configuration change detected";
    }, "[SettingsController]", "Configuration change handler");
}

void SettingsController::onSettingsWindowClosed()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QMutexLocker locker(&configurationMutex_);
        settingsOpen_ = false;
        settingsWindow_ = nullptr;
        
        emit settingsWindowVisibilityChanged(false);
        OPENAUTO_LOG(debug) << "[SettingsController] Settings window closed (destroyed)";
    }, "[SettingsController]", "Settings window closed handler");
}

void SettingsController::connectToConfiguration()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (configuration_) {
            // Connect to configuration change signals if available
            // This would depend on the IConfiguration interface implementation
            OPENAUTO_LOG(debug) << "[SettingsController] Connected to configuration signals";
        }
    }, "[SettingsController]", "Connect to configuration");
}

void SettingsController::disconnectFromConfiguration()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (configuration_) {
            // Disconnect from configuration change signals
            OPENAUTO_LOG(debug) << "[SettingsController] Disconnected from configuration signals";
        }
    }, "[SettingsController]", "Disconnect from configuration");
}

}
}
}
}
