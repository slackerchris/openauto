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
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <f1x/openauto/autoapp/Configuration/IConfiguration.hpp>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

/**
 * @brief Simple Media Player Controller
 * 
 * A basic controller to handle media player functionality extracted from MainWindow.
 * This is a first step in refactoring - start simple and working.
 */
class SimpleMediaController : public QObject
{
    Q_OBJECT

public:
    explicit SimpleMediaController(configuration::IConfiguration::Pointer configuration, QObject* parent = nullptr);
    ~SimpleMediaController() override;

    // Basic media controls
    void play();
    void pause();
    void stop();
    void next();
    void previous();
    
    // Playlist management
    void scanFolders();
    void scanFiles();
    void setCurrentTrack(int index);
    void setPlaylist(QMediaPlaylist* playlist);
    void setCurrentIndex(int index);
    
    // State and position
    QMediaPlayer::State getState() const;
    qint64 getPosition() const;
    void setPosition(qint64 position);
    int currentIndex() const;
    void setVolume(int volume);
    int getVolume() const;
    
    // Getters
    QMediaPlayer* getPlayer() const { return player_; }
    QMediaPlaylist* getPlaylist() const { return playlist_; }
    QString getMusicFolder() const { return musicFolder_; }
    QString getAlbumFolder() const { return albumFolder_; }

signals:
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void stateChanged(QMediaPlayer::State state);
    void metaDataChanged();

private slots:
    void onPositionChanged(qint64 position);
    void onDurationChanged(qint64 duration);
    void onStateChanged(QMediaPlayer::State state);
    void onMetaDataChanged();

private:
    configuration::IConfiguration::Pointer configuration_;
    QMediaPlayer* player_;
    QMediaPlaylist* playlist_;
    QString musicFolder_;
    QString albumFolder_;
    int currentPlaylistIndex_;
};

}
}
}
}
