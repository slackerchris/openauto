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
 * This controller handles all media playback, playlist management, and metadata operations.
 * 
 * Refactoring Goals:
 * - Single Responsibility Principle adherence
 * - Improved testability and maintainability  
 * - Thread-safe media operations
 * - Exception-safe error handling
 */

#pragma once

#include <memory>
#include <QObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QListWidget>
#include <QStandardItemModel>
#include <QFileSystemWatcher>
#include <QMutex>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

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
 * @brief Handles all media player functionality including playlist management,
 * playback control, and metadata handling.
 * 
 * This class encapsulates all media-related logic that was previously
 * scattered throughout MainWindow, providing a clean interface for
 * media operations.
 */
class MediaPlayerController : public QObject
{
    Q_OBJECT

public:
    explicit MediaPlayerController(Ui::MainWindow* ui,
                                 configuration::IConfiguration::Pointer configuration,
                                 QObject* parent = nullptr);
    ~MediaPlayerController() override;

    // Media player access
    QMediaPlayer* getPlayer() const { return player_; }
    QMediaPlaylist* getPlaylist() const { return playlist_; }

    // Playback control
    void play();
    void pause();
    void stop();
    void nextTrack();
    void previousTrack();
    void nextAlbum();
    void previousAlbum();

    // Playlist management
    void setCurrentIndex(int index);
    int getCurrentIndex() const;
    void scanFolders();
    void scanFiles();

    // Volume and position control
    void setVolume(int volume);
    void setPosition(qint64 position);

    // Folder management
    QString getMusicFolder() const;
    void setMusicFolder(const QString& folder);
    QString getAlbumFolder() const;
    void setAlbumFolder(const QString& folder);

    // State queries
    bool isMediaContentChanged() const;
    void setMediaContentChanged(bool changed);
    QString getSelectedMp3File() const;
    void setSelectedMp3File(const QString& file);

signals:
    void playbackStateChanged(QMediaPlayer::State state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void metaDataChanged();
    void playlistChanged();

private slots:
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onMetaDataChanged();
    void onStateChanged(QMediaPlayer::State state);
    void onPlaylistCurrentIndexChanged(int index);

private:
    void initializePlayer();
    void setupConnections();
    bool checkFileExists(const char* filename) const;
    void updateMetaDataDisplay();
    void updateTrackInfo();

    Ui::MainWindow* ui_;
    configuration::IConfiguration::Pointer configuration_;
    
    QMediaPlayer* player_;
    QMediaPlaylist* playlist_;
    QFileSystemWatcher* mediaWatcher_;
    
    QString musicFolder_;
    QString albumFolder_;
    QString selectedMp3File_;
    int currentPlaylistIndex_;
    bool mediaContentChanged_;
    
    mutable QMutex mediaStateMutex_;
};

}
}
}
}
