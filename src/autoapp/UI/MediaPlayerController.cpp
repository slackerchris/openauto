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
 * This controller handles all media playback, playlist management, and metadata operations.
 * 
 * Key Features:
 * - TagLib integration for metadata reading
 * - Thread-safe media state management
 * - Exception-safe file system operations
 * - Proper Qt signal/slot architecture
 */

#include "f1x/openauto/autoapp/UI/MediaPlayerController.hpp"part of openauto project.
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

#include <f1x/openauto/autoapp/UI/MediaPlayerController.hpp>
#include <f1x/openauto/Common/Log.hpp>
#include <f1x/openauto/Common/ErrorHandler.hpp>
#include "ui_mainwindow.h"

#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFont>
#include <QMutexLocker>
#include <QStandardItemModel>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <fstream>

namespace f1x
{
namespace openauto
{
namespace autoapp
{
namespace ui
{

MediaPlayerController::MediaPlayerController(Ui::MainWindow* ui,
                                           configuration::IConfiguration::Pointer configuration,
                                           QObject* parent)
    : QObject(parent)
    , ui_(ui)
    , configuration_(std::move(configuration))
    , player_(nullptr)
    , playlist_(nullptr)
    , mediaWatcher_(nullptr)
    , currentPlaylistIndex_(0)
    , mediaContentChanged_(true)
{
    initializePlayer();
    setupConnections();
    
    // Initialize paths from configuration
    musicFolder_ = QString::fromStdString(configuration_->getMp3MasterPath());
    albumFolder_ = QString::fromStdString(configuration_->getMp3SubFolder());
    
    OPENAUTO_LOG(info) << "[MediaPlayerController] Initialized with music folder: " << musicFolder_.toStdString();
}

MediaPlayerController::~MediaPlayerController()
{
    if (player_) {
        player_->stop();
    }
}

void MediaPlayerController::initializePlayer()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        player_ = new QMediaPlayer(this);
        playlist_ = new QMediaPlaylist(this);
        
        // Set up media watcher for USB drives
        mediaWatcher_ = new QFileSystemWatcher(this);
        mediaWatcher_->addPath("/media/USBDRIVES");
        
        OPENAUTO_LOG(debug) << "[MediaPlayerController] Player and playlist initialized";
    }, "[MediaPlayerController]", "Player initialization");
}

void MediaPlayerController::setupConnections()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        // Connect player signals
        connect(player_, &QMediaPlayer::positionChanged, this, &MediaPlayerController::onPositionChanged);
        connect(player_, &QMediaPlayer::durationChanged, this, &MediaPlayerController::onDurationChanged);
        connect(player_, &QMediaPlayer::metaDataAvailableChanged, this, &MediaPlayerController::onMetaDataChanged);
        connect(player_, &QMediaPlayer::stateChanged, this, &MediaPlayerController::onStateChanged);
        
        // Connect playlist signals
        connect(playlist_, &QMediaPlaylist::currentIndexChanged, this, &MediaPlayerController::onPlaylistCurrentIndexChanged);
        
        // Connect media watcher
        connect(mediaWatcher_, &QFileSystemWatcher::directoryChanged, this, &MediaPlayerController::scanFolders);
        
        OPENAUTO_LOG(debug) << "[MediaPlayerController] Signal connections established";
    }, "[MediaPlayerController]", "Signal connection setup");
}

void MediaPlayerController::play()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (player_ && playlist_) {
            player_->setPlaylist(playlist_);
            playlist_->setCurrentIndex(currentPlaylistIndex_);
            player_->play();
            OPENAUTO_LOG(debug) << "[MediaPlayerController] Playback started";
        }
    }, "[MediaPlayerController]", "Play operation");
}

void MediaPlayerController::pause()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (player_) {
            if (player_->state() == QMediaPlayer::PlayingState) {
                player_->pause();
                OPENAUTO_LOG(debug) << "[MediaPlayerController] Playback paused";
            } else {
                player_->play();
                OPENAUTO_LOG(debug) << "[MediaPlayerController] Playback resumed";
            }
        }
    }, "[MediaPlayerController]", "Pause/Resume operation");
}

void MediaPlayerController::stop()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (player_) {
            player_->stop();
            OPENAUTO_LOG(debug) << "[MediaPlayerController] Playback stopped";
        }
    }, "[MediaPlayerController]", "Stop operation");
}

void MediaPlayerController::nextTrack()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (playlist_) {
            playlist_->next();
            if (playlist_->currentIndex() != -1 && player_) {
                player_->play();
                OPENAUTO_LOG(debug) << "[MediaPlayerController] Next track: " << playlist_->currentIndex();
            }
        }
    }, "[MediaPlayerController]", "Next track operation");
}

void MediaPlayerController::previousTrack()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (playlist_) {
            playlist_->previous();
            if (playlist_->currentIndex() != -1 && player_) {
                player_->play();
                OPENAUTO_LOG(debug) << "[MediaPlayerController] Previous track: " << playlist_->currentIndex();
            }
        }
    }, "[MediaPlayerController]", "Previous track operation");
}

void MediaPlayerController::nextAlbum()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (ui_->comboBoxAlbum) {
            int albumCount = ui_->comboBoxAlbum->count();
            int currentAlbum = ui_->comboBoxAlbum->currentIndex();
            
            if (currentAlbum < albumCount - 1) {
                currentAlbum++;
            } else {
                currentAlbum = 0;
            }
            
            ui_->comboBoxAlbum->setCurrentIndex(currentAlbum);
            if (player_) {
                player_->play();
            }
            OPENAUTO_LOG(debug) << "[MediaPlayerController] Next album: " << currentAlbum;
        }
    }, "[MediaPlayerController]", "Next album operation");
}

void MediaPlayerController::previousAlbum()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (ui_->comboBoxAlbum) {
            int albumCount = ui_->comboBoxAlbum->count();
            int currentAlbum = ui_->comboBoxAlbum->currentIndex();
            
            if (currentAlbum >= 1) {
                currentAlbum--;
            } else {
                currentAlbum = albumCount - 1;
            }
            
            ui_->comboBoxAlbum->setCurrentIndex(currentAlbum);
            if (player_) {
                player_->play();
            }
            OPENAUTO_LOG(debug) << "[MediaPlayerController] Previous album: " << currentAlbum;
        }
    }, "[MediaPlayerController]", "Previous album operation");
}

void MediaPlayerController::setVolume(int volume)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, volume]() {
        if (player_) {
            player_->setVolume(volume);
            if (ui_->volumeValueLabelPlayer) {
                ui_->volumeValueLabelPlayer->setText(QString::number(volume) + "%");
            }
            OPENAUTO_LOG(debug) << "[MediaPlayerController] Volume set to: " << volume;
        }
    }, "[MediaPlayerController]", "Volume change operation");
}

void MediaPlayerController::setPosition(qint64 position)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, position]() {
        if (player_) {
            player_->setPosition(position);
            OPENAUTO_LOG(debug) << "[MediaPlayerController] Position set to: " << position;
        }
    }, "[MediaPlayerController]", "Position change operation");
}

void MediaPlayerController::setCurrentIndex(int index)
{
    QMutexLocker locker(&mediaStateMutex_);
    currentPlaylistIndex_ = index;
    
    if (playlist_) {
        playlist_->setCurrentIndex(index);
    }
}

int MediaPlayerController::getCurrentIndex() const
{
    QMutexLocker locker(&mediaStateMutex_);
    return currentPlaylistIndex_;
}

QString MediaPlayerController::getMusicFolder() const
{
    QMutexLocker locker(&mediaStateMutex_);
    return musicFolder_;
}

void MediaPlayerController::setMusicFolder(const QString& folder)
{
    QMutexLocker locker(&mediaStateMutex_);
    musicFolder_ = folder;
    setMediaContentChanged(true);
}

QString MediaPlayerController::getAlbumFolder() const
{
    QMutexLocker locker(&mediaStateMutex_);
    return albumFolder_;
}

void MediaPlayerController::setAlbumFolder(const QString& folder)
{
    QMutexLocker locker(&mediaStateMutex_);
    albumFolder_ = folder;
    setMediaContentChanged(true);
}

bool MediaPlayerController::isMediaContentChanged() const
{
    QMutexLocker locker(&mediaStateMutex_);
    return mediaContentChanged_;
}

void MediaPlayerController::setMediaContentChanged(bool changed)
{
    QMutexLocker locker(&mediaStateMutex_);
    mediaContentChanged_ = changed;
}

QString MediaPlayerController::getSelectedMp3File() const
{
    QMutexLocker locker(&mediaStateMutex_);
    return selectedMp3File_;
}

void MediaPlayerController::setSelectedMp3File(const QString& file)
{
    QMutexLocker locker(&mediaStateMutex_);
    selectedMp3File_ = file;
    setMediaContentChanged(true);
}

void MediaPlayerController::scanFolders()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (isMediaContentChanged()) {
            setMediaContentChanged(false);
            
            // Clear existing album list
            int cleaner = ui_->comboBoxAlbum->count();
            while (cleaner > -1) {
                ui_->comboBoxAlbum->removeItem(cleaner);
                cleaner--;
            }
            
            QString currentMusicFolder = getMusicFolder();
            QDir directory(currentMusicFolder);
            QStringList folders = directory.entryList(QStringList() << "*", QDir::AllDirs, QDir::Name);
            QStandardItemModel* model = new QStandardItemModel(this);
            
            for (const QString& folderName : folders) {
                if (folderName != "." && folderName != "..") {
                    ui_->comboBoxAlbum->addItem(folderName);
                    
                    QStandardItem* item = new QStandardItem(folderName);
                    QString coverPath = currentMusicFolder + "/" + folderName + "/folder.png";
                    if (checkFileExists(coverPath.toStdString().c_str())) {
                        QPixmap cover(coverPath);
                        item->setIcon(QIcon(cover.scaled(150, 150, Qt::KeepAspectRatio)));
                    }
                    model->appendRow(item);
                }
            }
            
            ui_->AlbumCoverListView->setModel(model);
            setCurrentIndex(0);
            ui_->SysinfoTopLeft->hide();
            
            emit playlistChanged();
            OPENAUTO_LOG(debug) << "[MediaPlayerController] Folders scanned, found " << folders.size() << " albums";
        }
    }, "[MediaPlayerController]", "Folder scanning operation");
}

void MediaPlayerController::scanFiles()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        if (!mediaContentChanged_) {
            // Clear existing playlist
            int cleaner = ui_->mp3List->count();
            while (cleaner > -1) {
                ui_->mp3List->takeItem(cleaner);
                cleaner--;
            }
            playlist_->clear();
            
            QList<QMediaContent> content;
            QString fullPath = musicFolder_ + "/" + albumFolder_;
            QDir directory(fullPath);
            QStringList musicFiles = directory.entryList(
                QStringList() << "*.mp3" << "*.flac" << "*.aac" << "*.ogg" << "*.mp4" << "*.mp4a" << "*.wma" << "*.strm",
                QDir::Files, QDir::Name);
            
            for (const QString& filename : musicFiles) {
                try {
                    if (filename.endsWith(".strm")) {
                        // Handle stream files
                        QString url = configuration_->readFileContent(fullPath + "/" + filename);
                        content.push_back(QMediaContent(QUrl(url)));
                        ui_->mp3List->addItem(filename.chopped(5)); // Remove .strm extension
                    } else {
                        // Handle regular audio files
                        content.push_back(QMediaContent(QUrl::fromLocalFile(fullPath + "/" + filename)));
                        
                        // Read metadata using taglib
                        TagLib::FileRef file((fullPath + "/" + filename).toUtf8(), true);
                        if (!file.isNull() && file.tag()) {
                            TagLib::String artist = file.tag()->artist();
                            TagLib::String title = file.tag()->title();
                            TagLib::uint track = file.tag()->track();
                            
                            QString artistStr = QString::fromStdWString(artist.toCWString());
                            QString titleStr = QString::fromStdWString(title.toCWString());
                            QString trackStr = QString::number(track);
                            
                            if (trackStr.length() < 2) {
                                trackStr = "0" + trackStr;
                            }
                            
                            QString id3Entry = trackStr + ": " + artistStr + " - " + titleStr;
                            ui_->mp3List->addItem(id3Entry);
                        } else {
                            ui_->mp3List->addItem(filename);
                        }
                    }
                } catch (const std::exception& e) {
                    OPENAUTO_LOG(warning) << "[MediaPlayerController] Error reading metadata for " 
                                        << filename.toStdString() << ": " << e.what();
                    ui_->mp3List->addItem(filename);
                }
            }
            
            // Set playlist
            playlist_->addMedia(content);
            OPENAUTO_LOG(debug) << "[MediaPlayerController] Files scanned, found " << musicFiles.size() << " tracks";
        }
    }, "[MediaPlayerController]", "File scanning operation");
}

bool MediaPlayerController::checkFileExists(const char* filename) const
{
    std::ifstream file(filename);
    return file.good();
}

void MediaPlayerController::onPositionChanged(qint64 position)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, position]() {
        if (ui_->horizontalSliderProgressPlayer) {
            ui_->horizontalSliderProgressPlayer->setValue(position);
        }
        
        // Update time display
        QString timeElapsed, timeTotal;
        int totalSeconds = (player_->duration() / 1000) % 60;
        int totalMinutes = (player_->duration() / 1000) / 60;
        
        if (totalMinutes >= 60) {
            int totalHours = totalMinutes / 60;
            totalMinutes = totalMinutes - (totalHours * 60);
            timeTotal = QString("%1").arg(totalHours, 2, 10, QChar('0')) + ':' +
                       QString("%1").arg(totalMinutes, 2, 10, QChar('0')) + ':' +
                       QString("%1").arg(totalSeconds, 2, 10, QChar('0'));
        } else {
            timeTotal = QString("%1").arg(totalMinutes, 2, 10, QChar('0')) + ':' +
                       QString("%1").arg(totalSeconds, 2, 10, QChar('0'));
        }
        
        int currentSeconds = (position / 1000) % 60;
        int currentMinutes = (position / 1000) / 60;
        
        if (currentMinutes >= 60) {
            int currentHours = currentMinutes / 60;
            currentMinutes = currentMinutes - (currentHours * 60);
            timeElapsed = QString("%1").arg(currentHours, 2, 10, QChar('0')) + ':' +
                         QString("%1").arg(currentMinutes, 2, 10, QChar('0')) + ':' +
                         QString("%1").arg(currentSeconds, 2, 10, QChar('0'));
        } else {
            timeElapsed = QString("%1").arg(currentMinutes, 2, 10, QChar('0')) + ':' +
                         QString("%1").arg(currentSeconds, 2, 10, QChar('0'));
        }
        
        if (ui_->playerPositionTime) {
            ui_->playerPositionTime->setText(timeElapsed + " / " + timeTotal);
        }
        
        emit positionChanged(position);
    }, "[MediaPlayerController]", "Position change handling");
}

void MediaPlayerController::onDurationChanged(qint64 duration)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, duration]() {
        if (ui_->horizontalSliderProgressPlayer) {
            ui_->horizontalSliderProgressPlayer->setMaximum(duration);
        }
        emit durationChanged(duration);
    }, "[MediaPlayerController]", "Duration change handling");
}

void MediaPlayerController::onMetaDataChanged()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        updateMetaDataDisplay();
        emit metaDataChanged();
    }, "[MediaPlayerController]", "Metadata change handling");
}

void MediaPlayerController::onStateChanged(QMediaPlayer::State state)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, state]() {
        // Update play state indicator files
        if (state == QMediaPlayer::StoppedState || state == QMediaPlayer::PausedState) {
            std::remove("/tmp/media_playing");
        } else {
            std::ofstream("/tmp/media_playing");
        }
        
        emit playbackStateChanged(state);
        OPENAUTO_LOG(debug) << "[MediaPlayerController] State changed to: " << static_cast<int>(state);
    }, "[MediaPlayerController]", "State change handling");
}

void MediaPlayerController::onPlaylistCurrentIndexChanged(int index)
{
    f1x::openauto::common::ErrorHandler::safeExecute([this, index]() {
        setCurrentIndex(index);
        updateTrackInfo();
        
        // Save current track to configuration
        configuration_->setMp3Track(index);
        configuration_->setMp3SubFolder(albumFolder_.toStdString());
        configuration_->save();
        
        OPENAUTO_LOG(debug) << "[MediaPlayerController] Playlist index changed to: " << index;
    }, "[MediaPlayerController]", "Playlist index change handling");
}

void MediaPlayerController::updateMetaDataDisplay()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QString fullPathPlaying = player_->currentMedia().request().url().toString();
        QString filename = QFileInfo(fullPathPlaying).fileName();
        
        // Update cover art
        QImage img = player_->metaData(QMediaMetaData::CoverArtImage).value<QImage>();
        if (!img.isNull()) {
            QImage imgScaled = img.scaled(270, 270, Qt::IgnoreAspectRatio);
            if (ui_->pushButtonBack) {
                ui_->pushButtonBack->setIcon(QPixmap::fromImage(imgScaled));
            }
        } else {
            // Look for local cover art
            QString coverPath = musicFolder_ + "/" + albumFolder_ + "/folder.png";
            if (checkFileExists(coverPath.toStdString().c_str())) {
                QPixmap cover(coverPath);
                if (ui_->pushButtonBack) {
                    ui_->pushButtonBack->setIcon(cover.scaled(270, 270, Qt::KeepAspectRatio));
                }
            } else if (ui_->pushButtonBack) {
                ui_->pushButtonBack->setIcon(QPixmap("://coverlogo.png"));
            }
        }
        
        // Update track information
        updateTrackInfo();
    }, "[MediaPlayerController]", "Metadata display update");
}

void MediaPlayerController::updateTrackInfo()
{
    f1x::openauto::common::ErrorHandler::safeExecute([this]() {
        QString currentPlaying;
        
        // Try to get metadata from playlist first
        if (playlist_->currentIndex() != -1 && ui_->mp3List->count() > playlist_->currentIndex()) {
            QString currentSong = ui_->mp3List->item(playlist_->currentIndex())->text();
            if (ui_->labelCurrentPlaying) {
                ui_->labelCurrentPlaying->setText(currentSong);
            }
            
            // Adjust font size for long titles
            if (currentSong.length() > 48) {
                int id = QFontDatabase::addApplicationFont(":/Roboto-Regular.ttf");
                QString family = QFontDatabase::applicationFontFamilies(id).at(0);
                QFont font(family, 12, QFont::Bold);
                font.setItalic(true);
                if (ui_->labelCurrentPlaying) {
                    ui_->labelCurrentPlaying->setFont(font);
                }
            } else {
                int id = QFontDatabase::addApplicationFont(":/Roboto-Regular.ttf");
                QString family = QFontDatabase::applicationFontFamilies(id).at(0);
                QFont font(family, 16, QFont::Bold);
                font.setItalic(true);
                if (ui_->labelCurrentPlaying) {
                    ui_->labelCurrentPlaying->setFont(font);
                }
            }
        } else {
            // Fallback to player metadata
            QString albumArtist = player_->metaData(QMediaMetaData::AlbumArtist).toString();
            QString title = player_->metaData(QMediaMetaData::Title).toString();
            
            if (albumArtist.isEmpty() && ui_->comboBoxAlbum && ui_->comboBoxAlbum->currentText() != ".") {
                albumArtist = ui_->comboBoxAlbum->currentText();
            }
            
            if (!albumArtist.isEmpty()) {
                currentPlaying.append(albumArtist);
            }
            if (!title.isEmpty() && !albumArtist.isEmpty()) {
                currentPlaying.append(" - ");
            }
            if (!title.isEmpty()) {
                currentPlaying.append(title);
            }
            
            if (ui_->labelCurrentPlaying) {
                ui_->labelCurrentPlaying->setText(currentPlaying);
            }
        }
        
        // Update track counter
        if (ui_->labelTrack && ui_->labelTrackCount) {
            ui_->labelTrack->setText(QString::number(playlist_->currentIndex() + 1));
            ui_->labelTrackCount->setText(QString::number(playlist_->mediaCount()));
        }
    }, "[MediaPlayerController]", "Track info update");
}

}
}
}
}
