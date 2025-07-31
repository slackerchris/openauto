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
 * This controller manages UI state and widget visibility coordination.
 * 
 * Refactoring Goals:
 * - Centralized UI state management
 * - Day/night mode coordination
 * - Menu state and widget visibility control
 * - Thread-safe UI operations
 */

#pragma once

#include <memory>
#include <QObject>
#include <QReadWriteLock>
#include <QMutex>

namespace Ui { class MainWindow; }

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

/**
 * @brief Manages UI state and visibility of various UI elements.
 * 
 * This class centralizes UI state management that was previously
 * scattered throughout MainWindow, providing thread-safe access
 * to UI state variables and coordinated visibility control.
 */
class UIStateManager : public QObject
{
    Q_OBJECT

public:
    explicit UIStateManager(Ui::MainWindow* ui, QObject* parent = nullptr);
    ~UIStateManager() override;

    // Day/Night mode management
    bool isNightModeEnabled() const;
    void setNightModeEnabled(bool enabled);
    bool isDayNightModeState() const;
    void setDayNightModeState(bool state);
    void switchToDay();
    void switchToNight();

    // Menu and widget visibility
    bool isExitMenuVisible() const;
    void setExitMenuVisible(bool visible);
    void toggleExitMenu();
    void showMediaPlayer();
    void hideMediaPlayer();
    void showBrightnessSlider();
    void hideBrightnessSlider();
    void showVolumeSlider();
    void hideVolumeSlider();

    // Camera controls
    bool isRearCamVisible() const;
    void setRearCamVisible(bool visible);
    void showRearCam();
    void hideRearCam();
    void showCameraControls();
    void hideCameraControls();

    // Recording state
    bool isDashCamRecording() const;
    void setDashCamRecording(bool recording);

    // Audio state
    bool isToggleMute() const;
    void setToggleMute(bool mute);
    void toggleMute();
    void setMute();
    void setUnMute();

    // GUI style
    bool isOldGUIStyle() const;
    void setOldGUIStyle(bool oldStyle);
    void toggleGUI();

    // Background and wallpaper management
    void updateBackground();
    bool checkWallpaperExists(const QString& wallpaperType) const;

signals:
    void nightModeChanged(bool enabled);
    void exitMenuVisibilityChanged(bool visible);
    void rearCamVisibilityChanged(bool visible);
    void muteStateChanged(bool muted);
    void guiStyleChanged(bool oldStyle);

private:
    void updateButtonVisibility();
    void updateMenuVisibility();
    void setWallpaper(const QString& wallpaperPath);

    Ui::MainWindow* ui_;
    
    // UI State variables
    bool nightModeEnabled_;
    bool dayNightModeState_;
    bool exitMenuVisible_;
    bool rearCamVisible_;
    bool dashCamRecording_;
    bool toggleMute_;
    bool oldGUIStyle_;
    bool useBigClock_;
    bool noClock_;
    
    // Wallpaper state
    bool wallpaperDayFileExists_;
    bool wallpaperNightFileExists_;
    bool wallpaperClassicDayFileExists_;
    bool wallpaperClassicNightFileExists_;
    bool wallpaperEQFileExists_;
    
    // Thread safety
    mutable QReadWriteLock stateLock_;
    mutable QMutex uiMutex_;
};

}
}
}
}
