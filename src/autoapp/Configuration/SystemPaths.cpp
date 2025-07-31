#include <f1x/openauto/autoapp/Configuration/SystemPaths.hpp>
#include <f1x/openauto/Common/ErrorHandler.hpp>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace f1x::openauto::autoapp::configuration
{

SystemPaths::SystemPaths()
{
    initializeDefaultPaths();
    initializeDefaultCommands();
}

void SystemPaths::initializeDefaultPaths()
{
    // Raspberry Pi specific paths (with fallbacks)
    customPaths_["brightness_control"] = "/sys/class/backlight/rpi_backlight/brightness";
    customPaths_["brightness_alt"] = "/tmp/custombrightness";
    customPaths_["night_mode"] = "/tmp/night_mode_enabled";
    customPaths_["dev_mode"] = "/tmp/dev_mode_enabled";
    customPaths_["wifi_button"] = "/etc/button_wifi_visible";
    customPaths_["camera_button"] = "/etc/button_camera_visible";
    customPaths_["brightness_button"] = "/etc/button_brightness_visible";
    customPaths_["debug_mode"] = "/tmp/usb_debug_mode";
    customPaths_["light_sensor"] = "/etc/cs_lightsensor";
    customPaths_["music_folder"] = "/media/CSSTORAGE/Music";
    
    // Custom button files
    for (int i = 1; i <= 6; ++i) {
        customPaths_[QString("custom_button_%1").arg(i)] = 
            QString("/boot/crankshaft/button_%1").arg(i);
    }
    
    // Temporary status files
    customPaths_["hotspot_status"] = "/tmp/hotspot_active";
    customPaths_["recent_list"] = "/tmp/temp_recent_list";
    customPaths_["gateway_data"] = "/tmp/gateway_wlan0";
    customPaths_["entity_exit"] = "/tmp/entityexit";
    
    // Update system files
    customPaths_["update_csmt_status"] = "/tmp/csmt_updating";
    customPaths_["update_csmt_available"] = "/tmp/csmt_update_available";
    customPaths_["update_udev_status"] = "/tmp/udev_updating";
    customPaths_["update_udev_available"] = "/tmp/udev_update_available";
    customPaths_["update_openauto_status"] = "/tmp/openauto_updating";
    customPaths_["update_openauto_available"] = "/tmp/openauto_update_available";
    customPaths_["update_system_ready"] = "/tmp/system_update_ready";
}

void SystemPaths::initializeDefaultCommands()
{
    // Helper commands
    systemCommands_["helper_path"] = "/usr/local/bin/autoapp_helper";
    systemCommands_["crankshaft_path"] = "/usr/local/bin/crankshaft";
    systemCommands_["crankshaft_fallback"] = "crankshaft";
    
    // Common operations
    systemCommands_["update_recent"] = "updaterecent";
    systemCommands_["usb_reset"] = "usbreset";
    systemCommands_["update_check"] = "update check";
    systemCommands_["update_cancel"] = "update cancel";
    
    // Update operations
    systemCommands_["update_csmt"] = "update csmt";
    systemCommands_["update_udev"] = "update udev";
    systemCommands_["update_openauto"] = "update openauto";
    systemCommands_["update_system"] = "update system";
}

QString SystemPaths::getBrightnessControlFile() const
{
    return getCustomPath("brightness_control");
}

QString SystemPaths::getAlternateBrightnessFile() const
{
    return getCustomPath("brightness_alt");
}

QString SystemPaths::getNightModeFile() const
{
    return getCustomPath("night_mode");
}

QString SystemPaths::getDevModeFile() const
{
    return getCustomPath("dev_mode");
}

QString SystemPaths::getWifiButtonFile() const
{
    return getCustomPath("wifi_button");
}

QString SystemPaths::getCameraButtonFile() const
{
    return getCustomPath("camera_button");
}

QString SystemPaths::getBrightnessButtonFile() const
{
    return getCustomPath("brightness_button");
}

QString SystemPaths::getDebugModeFile() const
{
    return getCustomPath("debug_mode");
}

QString SystemPaths::getLightSensorFile() const
{
    return getCustomPath("light_sensor");
}

QString SystemPaths::getMusicFolder() const
{
    QString musicPath = getCustomPath("music_folder");
    
    // Fallback to user's music directory if custom path doesn't exist
    if (!QDir(musicPath).exists()) {
        QString fallback = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
        if (!fallback.isEmpty() && QDir(fallback).exists()) {
            OPENAUTO_LOG(warning) << "[SystemPaths] Music folder not found at " << musicPath.toStdString() 
                                  << ", using fallback: " << fallback.toStdString();
            return fallback;
        }
    }
    
    return musicPath;
}

QString SystemPaths::getCustomButtonFile(int buttonNumber) const
{
    if (buttonNumber < 1 || buttonNumber > 6) {
        OPENAUTO_LOG(error) << "[SystemPaths] Invalid button number: " << buttonNumber;
        return QString();
    }
    
    return getCustomPath(QString("custom_button_%1").arg(buttonNumber));
}

QString SystemPaths::getHotspotStatusFile() const
{
    return getCustomPath("hotspot_status");
}

QString SystemPaths::getRecentListFile() const
{
    return getCustomPath("recent_list");
}

QString SystemPaths::getGatewayDataFile() const
{
    return getCustomPath("gateway_data");
}

QString SystemPaths::getEntityExitFile() const
{
    return getCustomPath("entity_exit");
}

QString SystemPaths::getUpdateStatusFile(const QString& component) const
{
    return getCustomPath(QString("update_%1_status").arg(component.toLower()));
}

QString SystemPaths::getUpdateAvailableFile(const QString& component) const
{
    return getCustomPath(QString("update_%1_available").arg(component.toLower()));
}

QString SystemPaths::getSystemCommand(const QString& operation) const
{
    if (operation == "usbreset") {
        QString helperPath = systemCommands_.value("helper_path", "/usr/local/bin/autoapp_helper");
        QString action = systemCommands_.value("usb_reset", "usbreset");
        return QString("%1 %2").arg(helperPath, action);
    }
    else if (operation == "updaterecent") {
        QString helperPath = systemCommands_.value("helper_path", "/usr/local/bin/autoapp_helper");
        QString action = systemCommands_.value("update_recent", "updaterecent");
        return QString("%1 %2").arg(helperPath, action);
    }
    
    OPENAUTO_LOG(warning) << "[SystemPaths] Unknown system operation: " << operation.toStdString();
    return QString();
}

QString SystemPaths::getHelperCommand(const QString& action) const
{
    QString helperPath = systemCommands_.value("helper_path", "/usr/local/bin/autoapp_helper");
    return QString("%1 %2").arg(helperPath, action);
}

QString SystemPaths::getCrankshaftCommand(const QString& action) const
{
    QString crankshaftPath = systemCommands_.value("crankshaft_path", "/usr/local/bin/crankshaft");
    
    // Try full path first, fall back to PATH lookup
    if (!QFileInfo::exists(crankshaftPath)) {
        crankshaftPath = systemCommands_.value("crankshaft_fallback", "crankshaft");
        OPENAUTO_LOG(info) << "[SystemPaths] Using crankshaft from PATH: " << crankshaftPath.toStdString();
    }
    
    if (action == "update_check") {
        return QString("%1 %2").arg(crankshaftPath, systemCommands_.value("update_check", "update check"));
    } else if (action == "update_cancel") {
        return QString("%1 %2").arg(crankshaftPath, systemCommands_.value("update_cancel", "update cancel"));
    } else if (action.startsWith("update_")) {
        QString component = action.mid(7); // Remove "update_" prefix
        QString updateCmd = systemCommands_.value(action, QString("update %1").arg(component));
        return QString("%1 %2").arg(crankshaftPath, updateCmd);
    }
    
    return QString("%1 %2").arg(crankshaftPath, action);
}

void SystemPaths::loadFromConfiguration()
{
    // This method can be extended to load paths from a configuration file
    // For now, we use environment variables as overrides
    
    auto loadEnvOverride = [this](const QString& key, const QString& envVar) {
        const char* envValue = std::getenv(envVar.toLocal8Bit().constData());
        if (envValue && strlen(envValue) > 0) {
            setCustomPath(key, QString::fromLocal8Bit(envValue));
            OPENAUTO_LOG(info) << "[SystemPaths] Override from " << envVar.toStdString() 
                               << ": " << key.toStdString() << " = " << envValue;
        }
    };
    
    // Load environment variable overrides
    loadEnvOverride("brightness_control", "OPENAUTO_BRIGHTNESS_PATH");
    loadEnvOverride("music_folder", "OPENAUTO_MUSIC_PATH");
    loadEnvOverride("helper_path", "OPENAUTO_HELPER_PATH");
    loadEnvOverride("crankshaft_path", "OPENAUTO_CRANKSHAFT_PATH");
}

void SystemPaths::setCustomPath(const QString& key, const QString& path)
{
    if (key.isEmpty() || path.isEmpty()) {
        OPENAUTO_LOG(warning) << "[SystemPaths] Ignoring empty key or path";
        return;
    }
    
    customPaths_[key] = path;
    OPENAUTO_LOG(debug) << "[SystemPaths] Set custom path: " << key.toStdString() 
                        << " = " << path.toStdString();
}

QString SystemPaths::getCustomPath(const QString& key, const QString& defaultValue) const
{
    return customPaths_.value(key, defaultValue);
}

bool SystemPaths::validatePath(const QString& path) const
{
    if (path.isEmpty()) {
        return false;
    }
    
    // Basic security check - prevent directory traversal
    if (path.contains("..") || path.contains("./")) {
        OPENAUTO_LOG(warning) << "[SystemPaths] Suspicious path detected: " << path.toStdString();
        return false;
    }
    
    return true;
}

bool SystemPaths::isPathAccessible(const QString& path) const
{
    if (!validatePath(path)) {
        return false;
    }
    
    QFileInfo info(path);
    return info.exists() && info.isReadable();
}

} // namespace f1x::openauto::autoapp::configuration
