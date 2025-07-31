#pragma once

#include <QString>
#include <QMap>
#include <memory>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x::openauto::autoapp::configuration
{

/**
 * @brief SystemPaths manages all system-dependent file paths and commands
 * 
 * This class centralizes system dependencies to improve portability and
 * maintainability. Instead of hardcoding paths throughout the codebase,
 * all system interactions go through this configurable interface.
 */
class SystemPaths
{
public:
    using Pointer = std::shared_ptr<SystemPaths>;

    SystemPaths();
    virtual ~SystemPaths() = default;

    // System file paths
    virtual QString getBrightnessControlFile() const;
    virtual QString getAlternateBrightnessFile() const;
    virtual QString getNightModeFile() const;
    virtual QString getDevModeFile() const;
    virtual QString getWifiButtonFile() const;
    virtual QString getCameraButtonFile() const;
    virtual QString getBrightnessButtonFile() const;
    virtual QString getDebugModeFile() const;
    virtual QString getLightSensorFile() const;
    virtual QString getMusicFolder() const;
    
    // Custom button files
    virtual QString getCustomButtonFile(int buttonNumber) const;
    
    // Temporary files
    virtual QString getHotspotStatusFile() const;
    virtual QString getRecentListFile() const;
    virtual QString getGatewayDataFile() const;
    virtual QString getEntityExitFile() const;
    
    // Update system files
    virtual QString getUpdateStatusFile(const QString& component) const;
    virtual QString getUpdateAvailableFile(const QString& component) const;
    
    // System commands
    virtual QString getSystemCommand(const QString& operation) const;
    virtual QString getHelperCommand(const QString& action) const;
    virtual QString getCrankshaftCommand(const QString& action) const;
    
    // Configuration management
    virtual void loadFromConfiguration();
    virtual void setCustomPath(const QString& key, const QString& path);
    virtual QString getCustomPath(const QString& key, const QString& defaultValue = QString()) const;
    
    // Validation
    virtual bool validatePath(const QString& path) const;
    virtual bool isPathAccessible(const QString& path) const;

private:
    QMap<QString, QString> customPaths_;
    QMap<QString, QString> systemCommands_;
    
    void initializeDefaultPaths();
    void initializeDefaultCommands();
};

} // namespace f1x::openauto::autoapp::configuration
