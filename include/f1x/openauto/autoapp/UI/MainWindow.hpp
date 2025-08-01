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

#include <memory>
#include <QMainWindow>
#include <QFile>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>
#include <f1x/openauto/autoapp/Configuration/SystemPaths.hpp>
#include <f1x/openauto/autoapp/System/SafeSystemExecutor.hpp>

#include <QMediaPlayer>
#include <QListWidgetItem>
#include <QListWidget>
#include <QMediaMetaData>
#include <QDir>
#include <QDirIterator>

#include <QMediaService>
#include <QMediaPlaylist>
#include <QVideoProbe>
#include <QAudioProbe>

#include <QFileDialog>

#include <qmediaplayer.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>

#include <QFileSystemWatcher>
#include <QKeyEvent>
#include <QMutex>
#include <QReadWriteLock>
#include <QMutexLocker>

#include <QBluetoothLocalDevice>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace ui {
    class CustomButtonController;
}
}
}
}

namespace Ui
{
class MainWindow;
}

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace configuration { class SystemPaths; }
namespace system { class SafeSystemExecutor; }
namespace ui
{
class SimpleMediaController;
class SystemController;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(configuration::IConfiguration::Pointer configuration, 
                       configuration::SystemPaths::Pointer systemPaths,
                       system::SafeSystemExecutor::Pointer systemExecutor,
                       QWidget *parent = nullptr);
    ~MainWindow() override;
    QFileSystemWatcher* watcher;
    QFileSystemWatcher* watcher_tmp; 
    
    // REFACTORING: Simple media controller for gradual refactoring
    // This controller handles media playback and playlist management
    // Goal: Replace all media-related code in MainWindow with controller methods
    SimpleMediaController* simpleMediaController;
    
    // REFACTORING: System controller for hardware and UI state management  
    // This controller handles brightness, volume, and day/night mode
    // Goal: Extract system control functionality from MainWindow
    SystemController* systemController;

signals:
    void exit();
    void reboot();
    void openSettings();
    void toggleCursor();
    void TriggerScriptDay();
    void TriggerScriptNight();
    void cameraShow();
    void cameraHide();
    void cameraStop();
    void cameraSave();
    void cameraRecord();
    void cameraPosYUp();
    void cameraPosYDown();
    void cameraZoomPlus();
    void cameraZoomMinus();
    void cameraFlipX();
    void cameraFlipY();
    void openConnectDialog();
    void openWifiDialog();
    void openUpdateDialog();
    void showBrightnessSlider();
    void showVolumeSlider();
    void showAlphaSlider();
    void showRearCam();
    void hideRearCam();
    void TriggerAppStart();
    void TriggerAppStop();
    void CloseAllDialogs();

private slots:
    void on_horizontalSliderBrightness_valueChanged(int value);
    void on_horizontalSliderVolume_valueChanged(int value);
    void updateAlpha();

private slots:
    void on_pushButtonBrightness_clicked();
    void on_pushButtonBrightness2_clicked();
    void on_pushButtonVolume_clicked();
    void on_pushButtonVolume2_clicked();
    void switchGuiToDay();
    void switchGuiToNight();
    void showTime();
    void cameraControlShow();
    void cameraControlHide();
    void toggleExit();
    void createDebuglog();
    void setPairable();
    void toggleMuteButton();
    void toggleGUI();
    void setMute();
    void setUnMute();
    // CustomButtonController handles all custom button logic
    void playerShow();
    void playerHide();
    void updateBG();

    void on_horizontalSliderProgressPlayer_sliderMoved(int position);
    void on_horizontalSliderVolumePlayer_sliderMoved(int position);
    void on_pushButtonList_clicked();
    void on_pushButtonPlayerStop_clicked();
    void on_pushButtonPlayerPause_clicked();
    void on_positionChanged(qint64 position);
    void on_durationChanged(qint64 position);
    void on_mp3List_itemClicked(QListWidgetItem *item);
    void metaDataChanged();
    void on_pushButtonPlayerPlayList_clicked();
    void on_pushButtonPlayerNextBig_clicked();
    void on_pushButtonPlayerPrevBig_clicked();
    void on_pushButtonPlayerPrevAlbum_clicked();
    void on_pushButtonPlayerNextAlbum_clicked();
    void on_pushButtonBackToPlayer_clicked();
    void on_comboBoxAlbum_currentIndexChanged(const QString &arg1);
    void on_mp3List_currentRowChanged(int currentRow);
    void on_StateChanged(QMediaPlayer::State state);
    void scanFolders();
    void scanFiles();
    void tmpChanged();
    void setTrigger();
    void setRetryUSBConnect();
    void resetRetryUSBMessage();
    void updateNetworkInfo();
    bool check_file_exist(const char *filename);
    void hostModeStateChanged(QBluetoothLocalDevice::HostMode);

    //void on_AlbumCoverListView_clicked(const QModelIndex &index);
    void on_AlbumCoverListView_clicked(const QModelIndex &index);

    void on_pushButtonAlbum_clicked();

private:
    CustomButtonController* customButtonController;
    Ui::MainWindow* ui_;
    configuration::IConfiguration::Pointer configuration_;
    configuration::SystemPaths::Pointer systemPaths_;
    system::SafeSystemExecutor::Pointer systemExecutor_;

    // CLEANUP: Removed brightness/volume control members - now handled by SystemController
    // QFile *brightnessFile;           // → SystemController
    // QFile *brightnessFileAlt;        // → SystemController  
    // char brightness_str[6];          // → SystemController
    // char volume_str[6];              // → SystemController
    int alpha_current_str;
    QString bversion;
    QString bdate;

    // Custom button config now handled by CustomButtonController

    QString selectedMp3file;
    QString date_text;
    QString musicfolder;
    QString albumfolder;

    QMediaPlaylist *playlist;

    bool customBrightnessControl = false;

    bool wifiButtonForce = false;
    bool cameraButtonForce = false;
    // CLEANUP: Removed brightness control flag - now handled by SystemController
    // bool brightnessButtonForce = false;  // → SystemController

    // CLEANUP: Removed theme state variables - now handled by SystemController
    // bool nightModeEnabled = false;       // → SystemController
    // bool DayNightModeState = false;      // → SystemController

    bool devModeEnabled = false;

    bool wallpaperDayFileExists = false;
    bool wallpaperNightFileExists = false;
    bool wallpaperClassicDayFileExists = false;
    bool wallpaperClassicNightFileExists = false;
    bool wallpaperEQFileExists = false;

    bool exitMenuVisible = false;

    bool rearCamEnabled = false;
    bool rearCamVisible = false;

    bool dashCamRecording = false;
    bool systemDebugmode = false;

    bool bluetoothEnabled = false;

    // CLEANUP: Removed mute state - now handled by SystemController
    // bool toggleMute = false;             // → SystemController
    bool oldGUIStyle = false;
    bool UseBigClock = false;
    bool NoClock = false;

    bool c1ButtonForce = false;
    bool c2ButtonForce = false;
    bool c3ButtonForce = false;
    bool c4ButtonForce = false;
    bool c5ButtonForce = false;
    bool c6ButtonForce = false;

    bool hotspotActive = false;
    int currentPlaylistIndex = 0;
    bool background_set = false;
    bool mediacontentchanged = true;

    bool lightsensor = false;
    bool holidaybg = false;

    bool csmtupdate = false;
    bool udevupdate = false;
    bool openautoupdate = false;
    bool systemupdate = false;

    int camera_ycorection;
    int camera_zoom;

    QBluetoothLocalDevice *localDevice;

    // Thread Safety Infrastructure
    mutable QReadWriteLock stateLock_;           // For read-heavy UI state variables
    mutable QMutex mediaStateMutex_;             // For media player state
    mutable QMutex systemStateMutex_;            // For system control state  
    mutable QMutex configurationMutex_;          // For configuration access

protected:
    void keyPressEvent(QKeyEvent *event);

private:
private:
    // Thread-safe accessors for critical shared state
    // CLEANUP: Removed system control accessors - now handled by SystemController
    // bool isNightModeEnabled() const;      // → SystemController
    // void setNightModeEnabled(bool enabled); // → SystemController
    // bool isDayNightModeState() const;     // → SystemController
    // void setDayNightModeState(bool state); // → SystemController
    // bool isToggleMute() const;            // → SystemController
    // void setToggleMute(bool mute);        // → SystemController
    bool isExitMenuVisible() const;
    void setExitMenuVisible(bool visible);
    bool isRearCamVisible() const;
    void setRearCamVisible(bool visible);
    bool isDashCamRecording() const;
    void setDashCamRecording(bool recording);
    bool isMediaContentChanged() const;
    void setMediaContentChanged(bool changed);
    
    QString getSelectedMp3File() const;
    void setSelectedMp3File(const QString& file);
    int getCurrentPlaylistIndex() const;
    void setCurrentPlaylistIndex(int index);
    
    QString getMusicFolder() const;
    void setMusicFolder(const QString& folder);
    QString getAlbumFolder() const;
    void setAlbumFolder(const QString& folder);
    
    // Helper methods for system paths
    bool checkFileExists(const QString& pathKey) const;
    QString getSystemPath(const QString& pathKey) const;
    bool executeSystemCommand(const QString& command) const;
    bool executeCrankshaftCommand(const QString& action) const;

};

}
}
}
}
