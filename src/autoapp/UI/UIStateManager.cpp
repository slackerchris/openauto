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
 * This controller manages UI state and widget visibility coordination.
 * 
 * Key Features:
 * - Centralized day/night mode management
 * - Widget visibility state coordination
 * - Thread-safe UI state access
 * - Background wallpaper management
 */

#include "f1x/openauto/autoapp/UI/UIStateManager.hpp"part of openauto project.
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

#include <f1x/openauto/autoapp/UI/UIStateManager.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/Common/ErrorHandler.hpp>
#include "ui_mainwindow.h"

#include <QMainWindow>
#include <QReadLocker>
#include <QWriteLocker>
#include <QMutexLocker>
#include <QFile>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

UIStateManager::UIStateManager(Ui::MainWindow* ui, QObject* parent)
    : QObject(parent)
    , ui_(ui)
    , nightModeEnabled_(false)
    , dayNightModeState_(false)
    , exitMenuVisible_(false)
    , rearCamVisible_(false)
    , dashCamRecording_(false)
    , toggleMute_(false)
    , oldGUIStyle_(false)
    , useBigClock_(false)
    , noClock_(false)
    , wallpaperDayFileExists_(false)
    , wallpaperNightFileExists_(false)
    , wallpaperClassicDayFileExists_(false)
    , wallpaperClassicNightFileExists_(false)
    , wallpaperEQFileExists_(false)
{
    // Check for wallpaper files
    wallpaperDayFileExists_ = QFile::exists("wallpaper-day.png");
    wallpaperNightFileExists_ = QFile::exists("wallpaper-night.png");
    wallpaperClassicDayFileExists_ = QFile::exists("wallpaper-classic-day.png");
    wallpaperClassicNightFileExists_ = QFile::exists("wallpaper-classic-night.png");
    wallpaperEQFileExists_ = QFile::exists("wallpaper-eq.png");
    
    OPENAUTO_LOG(info) << "[UIStateManager] Initialized with wallpaper support";
}

UIStateManager::~UIStateManager() = default;

bool UIStateManager::isNightModeEnabled() const
{
    QReadLocker locker(&stateLock_);
    return nightModeEnabled_;
}

void UIStateManager::setNightModeEnabled(bool enabled)
{
    QWriteLocker locker(&stateLock_);
    if (nightModeEnabled_ != enabled) {
        nightModeEnabled_ = enabled;
        locker.unlock();
        emit nightModeChanged(enabled);
        OPENAUTO_LOG(debug) << "[UIStateManager] Night mode: " << (enabled ? "enabled" : "disabled");
    }
}

bool UIStateManager::isDayNightModeState() const
{
    QReadLocker locker(&stateLock_);
    return dayNightModeState_;
}

void UIStateManager::setDayNightModeState(bool state)
{
    QWriteLocker locker(&stateLock_);
    dayNightModeState_ = state;
}

void UIStateManager::switchToDay()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        updateBackground();
        
        if (ui_->pushButtonNight && ui_->pushButtonNight2) {
            ui_->pushButtonNight->show();
            ui_->pushButtonNight2->show();
        }
        if (ui_->pushButtonDay && ui_->pushButtonDay2) {
            ui_->pushButtonDay->hide();
            ui_->pushButtonDay2->hide();
        }
        
        hideBrightnessSlider();
        if (ui_->mediaWidget && ui_->mediaWidget->isVisible()) {
            hideVolumeSlider();
        }
        
        setNightModeEnabled(false);
        OPENAUTO_LOG(debug) << "[UIStateManager] Switched to day mode";
    }, "[UIStateManager]", "Switch to day mode");
}

void UIStateManager::switchToNight()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        updateBackground();
        
        if (ui_->pushButtonDay && ui_->pushButtonDay2) {
            ui_->pushButtonDay->show();
            ui_->pushButtonDay2->show();
        }
        if (ui_->pushButtonNight && ui_->pushButtonNight2) {
            ui_->pushButtonNight->hide();
            ui_->pushButtonNight2->hide();
        }
        
        hideBrightnessSlider();
        if (ui_->mediaWidget && ui_->mediaWidget->isVisible()) {
            hideVolumeSlider();
        }
        
        setNightModeEnabled(true);
        OPENAUTO_LOG(debug) << "[UIStateManager] Switched to night mode";
    }, "[UIStateManager]", "Switch to night mode");
}

bool UIStateManager::isExitMenuVisible() const
{
    QReadLocker locker(&stateLock_);
    return exitMenuVisible_;
}

void UIStateManager::setExitMenuVisible(bool visible)
{
    QWriteLocker locker(&stateLock_);
    if (exitMenuVisible_ != visible) {
        exitMenuVisible_ = visible;
        locker.unlock();
        updateMenuVisibility();
        emit exitMenuVisibilityChanged(visible);
        OPENAUTO_LOG(debug) << "[UIStateManager] Exit menu visibility: " << (visible ? "visible" : "hidden");
    }
}

void UIStateManager::toggleExitMenu()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        bool currentlyVisible = isExitMenuVisible();
        setExitMenuVisible(!currentlyVisible);
    }, "[UIStateManager]", "Toggle exit menu");
}

void UIStateManager::showMediaPlayer()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (wallpaperEQFileExists_) {
            setWallpaper("wallpaper-eq.png");
        } else {
            setWallpaper(":/black.png");
        }
        
        updateMenuVisibility();
        
        if (ui_->mediaWidget) {
            ui_->mediaWidget->show();
        }
        if (ui_->VolumeSliderControlPlayer) {
            ui_->VolumeSliderControlPlayer->show();
        }
        hideVolumeSlider();
        hideBrightnessSlider();
        if (ui_->networkInfo) {
            ui_->networkInfo->hide();
        }
        
        OPENAUTO_LOG(debug) << "[UIStateManager] Media player shown";
    }, "[UIStateManager]", "Show media player");
}

void UIStateManager::hideMediaPlayer()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        updateMenuVisibility();
        
        if (ui_->mediaWidget) {
            ui_->mediaWidget->hide();
        }
        showVolumeSlider();
        if (ui_->VolumeSliderControlPlayer) {
            ui_->VolumeSliderControlPlayer->hide();
        }
        hideBrightnessSlider();
        
        updateBackground();
        
        OPENAUTO_LOG(debug) << "[UIStateManager] Media player hidden";
    }, "[UIStateManager]", "Hide media player");
}

void UIStateManager::showBrightnessSlider()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (ui_->BrightnessSliderControl) {
            ui_->BrightnessSliderControl->show();
        }
        hideVolumeSlider();
        OPENAUTO_LOG(debug) << "[UIStateManager] Brightness slider shown";
    }, "[UIStateManager]", "Show brightness slider");
}

void UIStateManager::hideBrightnessSlider()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (ui_->BrightnessSliderControl) {
            ui_->BrightnessSliderControl->hide();
        }
    }, "[UIStateManager]", "Hide brightness slider");
}

void UIStateManager::showVolumeSlider()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (ui_->VolumeSliderControl) {
            ui_->VolumeSliderControl->show();
        }
        hideBrightnessSlider();
        OPENAUTO_LOG(debug) << "[UIStateManager] Volume slider shown";
    }, "[UIStateManager]", "Show volume slider");
}

void UIStateManager::hideVolumeSlider()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (ui_->VolumeSliderControl) {
            ui_->VolumeSliderControl->hide();
        }
    }, "[UIStateManager]", "Hide volume slider");
}

bool UIStateManager::isRearCamVisible() const
{
    QReadLocker locker(&stateLock_);
    return rearCamVisible_;
}

void UIStateManager::setRearCamVisible(bool visible)
{
    QWriteLocker locker(&stateLock_);
    if (rearCamVisible_ != visible) {
        rearCamVisible_ = visible;
        locker.unlock();
        emit rearCamVisibilityChanged(visible);
        OPENAUTO_LOG(debug) << "[UIStateManager] Rear cam visibility: " << (visible ? "visible" : "hidden");
    }
}

void UIStateManager::showRearCam()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        setRearCamVisible(true);
        // Additional rear cam show logic would go here
    }, "[UIStateManager]", "Show rear camera");
}

void UIStateManager::hideRearCam()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        setRearCamVisible(false);
        // Additional rear cam hide logic would go here
    }, "[UIStateManager]", "Hide rear camera");
}

void UIStateManager::showCameraControls()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (ui_->cameraWidget) {
            ui_->cameraWidget->show();
        }
        updateMenuVisibility();
        OPENAUTO_LOG(debug) << "[UIStateManager] Camera controls shown";
    }, "[UIStateManager]", "Show camera controls");
}

void UIStateManager::hideCameraControls()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (ui_->cameraWidget) {
            ui_->cameraWidget->hide();
        }
        updateMenuVisibility();
        OPENAUTO_LOG(debug) << "[UIStateManager] Camera controls hidden";
    }, "[UIStateManager]", "Hide camera controls");
}

bool UIStateManager::isDashCamRecording() const
{
    QReadLocker locker(&stateLock_);
    return dashCamRecording_;
}

void UIStateManager::setDashCamRecording(bool recording)
{
    QWriteLocker locker(&stateLock_);
    dashCamRecording_ = recording;
}

bool UIStateManager::isToggleMute() const
{
    QReadLocker locker(&stateLock_);
    return toggleMute_;
}

void UIStateManager::setToggleMute(bool mute)
{
    QWriteLocker locker(&stateLock_);
    if (toggleMute_ != mute) {
        toggleMute_ = mute;
        locker.unlock();
        updateButtonVisibility();
        emit muteStateChanged(mute);
        OPENAUTO_LOG(debug) << "[UIStateManager] Mute state: " << (mute ? "muted" : "unmuted");
    }
}

void UIStateManager::toggleMute()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        bool currentMute = isToggleMute();
        setToggleMute(!currentMute);
    }, "[UIStateManager]", "Toggle mute");
}

void UIStateManager::setMute()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        setToggleMute(true);
        if (ui_->pushButtonMute) {
            ui_->pushButtonMute->hide();
        }
        if (ui_->pushButtonUnMute) {
            ui_->pushButtonUnMute->show();
        }
    }, "[UIStateManager]", "Set mute");
}

void UIStateManager::setUnMute()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        setToggleMute(false);
        if (ui_->pushButtonUnMute) {
            ui_->pushButtonUnMute->hide();
        }
        if (ui_->pushButtonMute) {
            ui_->pushButtonMute->show();
        }
    }, "[UIStateManager]", "Set unmute");
}

bool UIStateManager::isOldGUIStyle() const
{
    QReadLocker locker(&stateLock_);
    return oldGUIStyle_;
}

void UIStateManager::setOldGUIStyle(bool oldStyle)
{
    QWriteLocker locker(&stateLock_);
    if (oldGUIStyle_ != oldStyle) {
        oldGUIStyle_ = oldStyle;
        locker.unlock();
        updateMenuVisibility();
        emit guiStyleChanged(oldStyle);
        OPENAUTO_LOG(debug) << "[UIStateManager] GUI style: " << (oldStyle ? "old" : "new");
    }
}

void UIStateManager::toggleGUI()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        bool currentStyle = isOldGUIStyle();
        setOldGUIStyle(!currentStyle);
    }, "[UIStateManager]", "Toggle GUI style");
}

void UIStateManager::updateBackground()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QMainWindow* mainWindow = qobject_cast<QMainWindow*>(ui_->centralwidget->parent());
        if (!mainWindow) return;
        
        if (isNightModeEnabled()) {
            if (wallpaperNightFileExists_) {
                setWallpaper("wallpaper-night.png");
            } else if (isOldGUIStyle() && wallpaperClassicNightFileExists_) {
                setWallpaper("wallpaper-classic-night.png");
            } else {
                setWallpaper(":/black.png");
            }
        } else {
            if (wallpaperDayFileExists_) {
                setWallpaper("wallpaper-day.png");
            } else if (isOldGUIStyle() && wallpaperClassicDayFileExists_) {
                setWallpaper("wallpaper-classic-day.png");
            } else {
                setWallpaper(":/black.png");
            }
        }
        
        OPENAUTO_LOG(debug) << "[UIStateManager] Background updated for " 
                          << (isNightModeEnabled() ? "night" : "day") << " mode";
    }, "[UIStateManager]", "Update background");
}

bool UIStateManager::checkWallpaperExists(const QString& wallpaperType) const
{
    if (wallpaperType == "day") return wallpaperDayFileExists_;
    if (wallpaperType == "night") return wallpaperNightFileExists_;
    if (wallpaperType == "classic-day") return wallpaperClassicDayFileExists_;
    if (wallpaperType == "classic-night") return wallpaperClassicNightFileExists_;
    if (wallpaperType == "eq") return wallpaperEQFileExists_;
    return false;
}

void UIStateManager::updateButtonVisibility()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QMutexLocker locker(&uiMutex_);
        
        // Update mute button visibility
        if (isToggleMute()) {
            if (ui_->pushButtonMute) ui_->pushButtonMute->hide();
            if (ui_->pushButtonUnMute) ui_->pushButtonUnMute->show();
        } else {
            if (ui_->pushButtonUnMute) ui_->pushButtonUnMute->hide();
            if (ui_->pushButtonMute) ui_->pushButtonMute->show();
        }
    }, "[UIStateManager]", "Update button visibility");
}

void UIStateManager::updateMenuVisibility()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QMutexLocker locker(&uiMutex_);
        
        // Update main menu visibility based on GUI style
        if (isOldGUIStyle()) {
            if (ui_->menuWidget) ui_->menuWidget->hide();
            if (ui_->oldmenuWidget) ui_->oldmenuWidget->show();
        } else {
            if (ui_->oldmenuWidget) ui_->oldmenuWidget->hide();
            if (ui_->menuWidget) ui_->menuWidget->show();
        }
        
        // Update exit menu visibility
        if (isExitMenuVisible()) {
            if (ui_->exitWidget) ui_->exitWidget->show();
            if (ui_->horizontalWidgetPower) ui_->horizontalWidgetPower->show();
        } else {
            if (ui_->exitWidget) ui_->exitWidget->hide();
            if (ui_->horizontalWidgetPower) ui_->horizontalWidgetPower->hide();
        }
    }, "[UIStateManager]", "Update menu visibility");
}

void UIStateManager::setWallpaper(const QString& wallpaperPath)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, wallpaperPath]() {
        QMainWindow* mainWindow = qobject_cast<QMainWindow*>(ui_->centralwidget->parent());
        if (mainWindow) {
            QString styleSheet = QString("QMainWindow { background: url(%1); background-repeat: no-repeat; background-position: center; }").arg(wallpaperPath);
            mainWindow->setStyleSheet(styleSheet);
        }
    }, "[UIStateManager]", "Set wallpaper");
}

}
}
}
}
