#pragma once

#include <QString>
#include <QProcess>
#include <functional>
#include <memory>
#include <f1x/openauto/autoapp/Configuration/SystemPaths.hpp>

namespace f1x::openauto::autoapp::system
{

/**
 * @brief SafeSystemExecutor provides secure system command execution
 * 
 * This class replaces direct system() calls with safer, more controlled
 * execution that integrates with the ErrorHandler infrastructure.
 */
class SafeSystemExecutor
{
public:
    using Pointer = std::shared_ptr<SafeSystemExecutor>;
    using CompletionCallback = std::function<void(int exitCode, const QString& output, const QString& error)>;

    explicit SafeSystemExecutor(configuration::SystemPaths::Pointer systemPaths);
    virtual ~SafeSystemExecutor() = default;

    // Synchronous execution (blocks until completion)
    virtual bool executeCommand(const QString& command, QString* output = nullptr, QString* error = nullptr);
    virtual bool executeHelperCommand(const QString& action, QString* output = nullptr, QString* error = nullptr);
    virtual bool executeCrankshaftCommand(const QString& action, QString* output = nullptr, QString* error = nullptr);
    
    // Asynchronous execution (returns immediately, calls callback when done)
    virtual bool executeCommandAsync(const QString& command, CompletionCallback callback = nullptr);
    virtual bool executeHelperCommandAsync(const QString& action, CompletionCallback callback = nullptr);
    virtual bool executeCrankshaftCommandAsync(const QString& action, CompletionCallback callback = nullptr);
    
    // File operations
    virtual bool checkFileExists(const QString& filePath) const;
    virtual bool readFileContent(const QString& filePath, QString& content) const;
    virtual bool writeFileContent(const QString& filePath, const QString& content) const;
    virtual bool removeFile(const QString& filePath) const;
    
    // Configuration
    virtual void setTimeout(int milliseconds);
    virtual void setWorkingDirectory(const QString& directory);
    virtual void setEnvironment(const QStringList& environment);

private:
    configuration::SystemPaths::Pointer systemPaths_;
    int timeoutMs_;
    QString workingDirectory_;
    QStringList environment_;
    
    bool executeInternal(const QString& command, bool async, CompletionCallback callback, 
                        QString* output, QString* error);
    bool validateCommand(const QString& command) const;
    QString sanitizeCommand(const QString& command) const;
};

} // namespace f1x::openauto::autoapp::system
