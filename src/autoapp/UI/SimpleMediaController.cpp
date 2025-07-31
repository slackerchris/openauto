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

#include <f1x/openauto/autoapp/UI/SimpleMediaController.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <QDir>
#include <QFileInfo>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

SimpleMediaController::SimpleMediaController(configuration::IConfiguration::Pointer configuration, QObject* parent)
    : QObject(parent)
    , configuration_(std::move(configuration))
    , player_(new QMediaPlayer(this))
    , playlist_(new QMediaPlaylist(this))
    , currentPlaylistIndex_(0)
{
    // Set up basic connections
    connect(player_, &QMediaPlayer::positionChanged, this, &SimpleMediaController::onPositionChanged);
    connect(player_, &QMediaPlayer::durationChanged, this, &SimpleMediaController::onDurationChanged);
    connect(player_, &QMediaPlayer::stateChanged, this, &SimpleMediaController::onStateChanged);
    connect(player_, &QMediaPlayer::metaDataAvailableChanged, this, &SimpleMediaController::onMetaDataChanged);
    
    // Set up playlist
    player_->setPlaylist(playlist_);
    
    // Initialize folders
    musicFolder_ = QString::fromStdString(configuration_->getMp3MasterPath());
    albumFolder_ = QString::fromStdString(configuration_->getMp3SubFolder());
    
    OPENAUTO_LOG(debug) << "[SimpleMediaController] Initialized with music folder: " << musicFolder_.toStdString();
}

SimpleMediaController::~SimpleMediaController()
{
    if (player_) {
        player_->stop();
    }
}

void SimpleMediaController::play()
{
    if (player_) {
        player_->play();
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Play";
    }
}

void SimpleMediaController::pause()
{
    if (player_) {
        player_->pause();
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Pause";
    }
}

void SimpleMediaController::stop()
{
    if (player_) {
        player_->stop();
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Stop";
    }
}

void SimpleMediaController::next()
{
    if (playlist_) {
        playlist_->next();
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Next track";
    }
}

void SimpleMediaController::previous()
{
    if (playlist_) {
        playlist_->previous();
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Previous track";
    }
}

void SimpleMediaController::scanFolders()
{
    // Basic folder scanning implementation
    OPENAUTO_LOG(debug) << "[SimpleMediaController] Scanning folders in: " << musicFolder_.toStdString();
    
    QDir musicDir(musicFolder_);
    if (!musicDir.exists()) {
        OPENAUTO_LOG(warning) << "[SimpleMediaController] Music folder does not exist: " << musicFolder_.toStdString();
        return;
    }
    
    // This is a simplified version - in the full implementation we'd populate folder lists
}

void SimpleMediaController::scanFiles()
{
    // Basic file scanning implementation
    QString albumPath = musicFolder_ + "/" + albumFolder_;
    OPENAUTO_LOG(debug) << "[SimpleMediaController] Scanning files in: " << albumPath.toStdString();
    
    QDir albumDir(albumPath);
    if (!albumDir.exists()) {
        OPENAUTO_LOG(warning) << "[SimpleMediaController] Album folder does not exist: " << albumPath.toStdString();
        return;
    }
    
    // Clear existing playlist
    playlist_->clear();
    
    // Add music files to playlist
    QStringList filters;
    filters << "*.mp3" << "*.wav" << "*.flac" << "*.ogg";
    albumDir.setNameFilters(filters);
    
    QFileInfoList files = albumDir.entryInfoList(QDir::Files | QDir::Readable, QDir::Name);
    for (const QFileInfo& fileInfo : files) {
        playlist_->addMedia(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Added file: " << fileInfo.fileName().toStdString();
    }
    
    OPENAUTO_LOG(info) << "[SimpleMediaController] Loaded " << playlist_->mediaCount() << " files";
}

void SimpleMediaController::setCurrentTrack(int index)
{
    if (playlist_ && index >= 0 && index < playlist_->mediaCount()) {
        playlist_->setCurrentIndex(index);
        currentPlaylistIndex_ = index;
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Set current track to index: " << index;
    }
}

void SimpleMediaController::setPlaylist(QMediaPlaylist* playlist)
{
    if (player_) {
        player_->setPlaylist(playlist);
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Set playlist";
    }
}

void SimpleMediaController::setCurrentIndex(int index)
{
    if (playlist_) {
        playlist_->setCurrentIndex(index);
        currentPlaylistIndex_ = index;
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Set current index: " << index;
    }
}

QMediaPlayer::State SimpleMediaController::getState() const
{
    return player_ ? player_->state() : QMediaPlayer::StoppedState;
}

qint64 SimpleMediaController::getPosition() const
{
    return player_ ? player_->position() : 0;
}

void SimpleMediaController::setPosition(qint64 position)
{
    if (player_) {
        player_->setPosition(position);
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Set position: " << position;
    }
}

int SimpleMediaController::currentIndex() const
{
    return playlist_ ? playlist_->currentIndex() : -1;
}

void SimpleMediaController::setVolume(int volume)
{
    if (player_) {
        player_->setVolume(volume);
        OPENAUTO_LOG(debug) << "[SimpleMediaController] Set volume: " << volume;
    }
}

int SimpleMediaController::getVolume() const
{
    return player_ ? player_->volume() : 0;
}

void SimpleMediaController::onPositionChanged(qint64 position)
{
    emit positionChanged(position);
}

void SimpleMediaController::onDurationChanged(qint64 duration)
{
    emit durationChanged(duration);
}

void SimpleMediaController::onStateChanged(QMediaPlayer::State state)
{
    emit stateChanged(state);
}

void SimpleMediaController::onMetaDataChanged()
{
    emit metaDataChanged();
}

}
}
}
}
