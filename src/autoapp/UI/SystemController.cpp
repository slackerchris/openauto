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

#include <f1x/openauto/autoapp/UI/SystemController.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <QStandardPaths>
#include <QDir>
#include <cstdlib>

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
    , brightnessFile_(nullptr)
    , brightnessFileAlt_(nullptr)
    , customBrightnessControl_(false)
    , currentBrightness_(100)
    , currentVolume_(50)
    , isNightMode_(false)
    , isMuted_(false)
{
    OPENAUTO_LOG(info) << "[SystemController] Initializing system controller";
    
    initializeBrightnessControl();
    
    // Initialize from configuration
    isNightMode_ = false; // Default value - will be updated from actual config later
    isMuted_ = false; // Will be updated from system state
    
    OPENAUTO_LOG(info) << "[SystemController] System controller initialized";
}

SystemController::~SystemController()
{
    delete brightnessFile_;
    delete brightnessFileAlt_;
}

void SystemController::setBrightness(int value)
{
    if (!isBrightnessControlAvailable()) {
        OPENAUTO_LOG(warning) << "[SystemController] Brightness control not available";
        return;
    }
    
    currentBrightness_ = qBound(0, value, 100);
    
    // Write to brightness file
    if (brightnessFile_ && brightnessFile_->isOpen()) {
        brightnessFile_->seek(0);
        brightnessFile_->write(QString::number(currentBrightness_).toUtf8());
        brightnessFile_->flush();
    }
    
    // Try alternative brightness file if available
    if (brightnessFileAlt_ && brightnessFileAlt_->isOpen()) {
        brightnessFileAlt_->seek(0);
        brightnessFileAlt_->write(QString::number(currentBrightness_).toUtf8());
        brightnessFileAlt_->flush();
    }
    
    OPENAUTO_LOG(debug) << "[SystemController] Brightness set to: " << currentBrightness_;
    emit brightnessChanged(currentBrightness_);
}

int SystemController::getBrightness() const
{
    return currentBrightness_;
}

bool SystemController::isBrightnessControlAvailable() const
{
    return customBrightnessControl_ && (brightnessFile_ || brightnessFileAlt_);
}

void SystemController::setVolume(int value)
{
    currentVolume_ = qBound(0, value, 100);
    
    // Execute system volume command
    QString volumeCommand = QString("amixer -D pulse sset Master %1%").arg(currentVolume_);
    if (executeSystemCommand(volumeCommand)) {
        OPENAUTO_LOG(debug) << "[SystemController] Volume set to: " << currentVolume_;
        emit volumeChanged(currentVolume_);
    } else {
        OPENAUTO_LOG(warning) << "[SystemController] Failed to set volume";
    }
}

int SystemController::getVolume() const
{
    return currentVolume_;
}

void SystemController::switchToNightMode()
{
    if (!isNightMode_) {
        isNightMode_ = true;
        // TODO: Save to configuration when proper interface is available
        
        // Execute night mode script if available
        executeSystemCommand("/opt/crankshaft/service_daynight.sh app night");
        
        OPENAUTO_LOG(info) << "[SystemController] Switched to night mode";
        emit themeChanged(true);
    }
}

void SystemController::switchToDayMode()
{
    if (isNightMode_) {
        isNightMode_ = false;
        // TODO: Save to configuration when proper interface is available
        
        // Execute day mode script if available
        executeSystemCommand("/opt/crankshaft/service_daynight.sh app day");
        
        OPENAUTO_LOG(info) << "[SystemController] Switched to day mode";
        emit themeChanged(false);
    }
}

bool SystemController::isNightMode() const
{
    return isNightMode_;
}

void SystemController::setMute(bool muted)
{
    if (isMuted_ != muted) {
        isMuted_ = muted;
        
        // Execute mute/unmute command
        QString muteCommand = muted ? "amixer -D pulse sset Master mute" : "amixer -D pulse sset Master unmute";
        if (executeSystemCommand(muteCommand)) {
            OPENAUTO_LOG(debug) << "[SystemController] Mute state set to: " << muted;
            emit muteStateChanged(muted);
        } else {
            OPENAUTO_LOG(warning) << "[SystemController] Failed to set mute state";
        }
    }
}

bool SystemController::isMuted() const
{
    return isMuted_;
}

void SystemController::initializeBrightnessControl()
{
    // Check if custom brightness control is enabled
    customBrightnessControl_ = !configuration_->hideBrightnessControl();
    
    if (!customBrightnessControl_) {
        OPENAUTO_LOG(debug) << "[SystemController] Custom brightness control disabled";
        return;
    }
    
    // Try primary brightness file
    QString brightnessPath = getSystemPath("brightness_file");
    if (!brightnessPath.isEmpty() && checkFileExists("brightness_file")) {
        brightnessFile_ = new QFile(brightnessPath, this);
        if (brightnessFile_->open(QIODevice::WriteOnly | QIODevice::Text)) {
            OPENAUTO_LOG(info) << "[SystemController] Primary brightness control available: " << brightnessPath.toStdString();
        } else {
            OPENAUTO_LOG(warning) << "[SystemController] Failed to open brightness file: " << brightnessPath.toStdString();
            delete brightnessFile_;
            brightnessFile_ = nullptr;
        }
    }
    
    // Try alternative brightness file
    QString brightnessAltPath = getSystemPath("brightness_file_alt");
    if (!brightnessAltPath.isEmpty() && checkFileExists("brightness_file_alt")) {
        brightnessFileAlt_ = new QFile(brightnessAltPath, this);
        if (brightnessFileAlt_->open(QIODevice::WriteOnly | QIODevice::Text)) {
            OPENAUTO_LOG(info) << "[SystemController] Alternative brightness control available: " << brightnessAltPath.toStdString();
        } else {
            OPENAUTO_LOG(warning) << "[SystemController] Failed to open alternative brightness file: " << brightnessAltPath.toStdString();
            delete brightnessFileAlt_;
            brightnessFileAlt_ = nullptr;
        }
    }
}

bool SystemController::checkFileExists(const QString& pathKey) const
{
    if (!systemPaths_) return false;
    
    try {
        QString path = getSystemPath(pathKey);
        return !path.isEmpty() && QFile::exists(path);
    } catch (...) {
        return false;
    }
}

QString SystemController::getSystemPath(const QString& pathKey) const
{
    if (!systemPaths_) return QString();
    
    try {
        // This would need to match the actual SystemPaths interface
        // For now, return hardcoded paths that match the existing implementation
        if (pathKey == "brightness_file") {
            return "/sys/class/backlight/rpi_backlight/brightness";
        } else if (pathKey == "brightness_file_alt") {
            return "/sys/class/backlight/10-0045/brightness";
        }
        return QString();
    } catch (...) {
        return QString();
    }
}

bool SystemController::executeSystemCommand(const QString& command) const
{
    if (!systemExecutor_) return false;
    
    try {
        // Use the safe system executor
        // For now, use system() call - this should be replaced with proper SafeSystemExecutor usage
        int result = std::system(command.toStdString().c_str());
        return result == 0;
    } catch (...) {
        OPENAUTO_LOG(error) << "[SystemController] Exception executing command: " << command.toStdString();
        return false;
    }
}

}
}
}
}
