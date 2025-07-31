#include <f1x/openauto/autoapp/System/SafeSystemExecutor.hpp>
#include <f1x/openauto/Common/ErrorHandler.hpp>
#include <QFileInfo>
#include <QDir>
#include <QTextStream>
#include <QRegularExpression>

namespace f1x::openauto::autoapp::system
{

SafeSystemExecutor::SafeSystemExecutor(configuration::SystemPaths::Pointer systemPaths)
    : systemPaths_(std::move(systemPaths))
    , timeoutMs_(30000) // 30 second default timeout
    , workingDirectory_(QDir::currentPath())
{
}

bool SafeSystemExecutor::executeCommand(const QString& command, QString* output, QString* error)
{
    return executeInternal(command, false, nullptr, output, error);
}

bool SafeSystemExecutor::executeHelperCommand(const QString& action, QString* output, QString* error)
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        QString command = systemPaths_->getHelperCommand(action);
        if (command.isEmpty()) {
            if (error) *error = QString("Helper command not found for action: %1").arg(action);
            return false;
        }
        return executeCommand(command, output, error);
    }, "[SafeSystemExecutor]", "executeHelperCommand");
}

bool SafeSystemExecutor::executeCrankshaftCommand(const QString& action, QString* output, QString* error)
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        QString command = systemPaths_->getCrankshaftCommand(action);
        if (command.isEmpty()) {
            if (error) *error = QString("Crankshaft command not found for action: %1").arg(action);
            return false;
        }
        return executeCommand(command, output, error);
    }, "[SafeSystemExecutor]", "executeCrankshaftCommand");
}

bool SafeSystemExecutor::executeCommandAsync(const QString& command, CompletionCallback callback)
{
    return executeInternal(command, true, callback, nullptr, nullptr);
}

bool SafeSystemExecutor::executeHelperCommandAsync(const QString& action, CompletionCallback callback)
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        QString command = systemPaths_->getHelperCommand(action);
        if (command.isEmpty()) {
            if (callback) {
                callback(-1, "", QString("Helper command not found for action: %1").arg(action));
            }
            return false;
        }
        return executeCommandAsync(command, callback);
    }, "[SafeSystemExecutor]", "executeHelperCommandAsync");
}

bool SafeSystemExecutor::executeCrankshaftCommandAsync(const QString& action, CompletionCallback callback)
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        QString command = systemPaths_->getCrankshaftCommand(action);
        if (command.isEmpty()) {
            if (callback) {
                callback(-1, "", QString("Crankshaft command not found for action: %1").arg(action));
            }
            return false;
        }
        return executeCommandAsync(command, callback);
    }, "[SafeSystemExecutor]", "executeCrankshaftCommandAsync");
}

bool SafeSystemExecutor::checkFileExists(const QString& filePath) const
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        if (!systemPaths_->validatePath(filePath)) {
            OPENAUTO_LOG(warning) << "[SafeSystemExecutor] Invalid file path: " << filePath.toStdString();
            return false;
        }
        
        return QFileInfo::exists(filePath);
    }, "[SafeSystemExecutor]", "checkFileExists");
}

bool SafeSystemExecutor::readFileContent(const QString& filePath, QString& content) const
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        if (!systemPaths_->validatePath(filePath)) {
            OPENAUTO_LOG(warning) << "[SafeSystemExecutor] Invalid file path: " << filePath.toStdString();
            return false;
        }
        
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            OPENAUTO_LOG(error) << "[SafeSystemExecutor] Cannot open file for reading: " 
                                << filePath.toStdString() << " - " << file.errorString().toStdString();
            return false;
        }
        
        QTextStream stream(&file);
        content = stream.readAll();
        return true;
    }, "[SafeSystemExecutor]", "readFileContent");
}

bool SafeSystemExecutor::writeFileContent(const QString& filePath, const QString& content) const
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        if (!systemPaths_->validatePath(filePath)) {
            OPENAUTO_LOG(warning) << "[SafeSystemExecutor] Invalid file path: " << filePath.toStdString();
            return false;
        }
        
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            OPENAUTO_LOG(error) << "[SafeSystemExecutor] Cannot open file for writing: " 
                                << filePath.toStdString() << " - " << file.errorString().toStdString();
            return false;
        }
        
        QTextStream stream(&file);
        stream << content;
        return true;
    }, "[SafeSystemExecutor]", "writeFileContent");
}

bool SafeSystemExecutor::removeFile(const QString& filePath) const
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        if (!systemPaths_->validatePath(filePath)) {
            OPENAUTO_LOG(warning) << "[SafeSystemExecutor] Invalid file path: " << filePath.toStdString();
            return false;
        }
        
        if (!QFile::exists(filePath)) {
            OPENAUTO_LOG(debug) << "[SafeSystemExecutor] File does not exist: " << filePath.toStdString();
            return true; // Consider non-existent file as successfully "removed"
        }
        
        bool result = QFile::remove(filePath);
        if (!result) {
            OPENAUTO_LOG(error) << "[SafeSystemExecutor] Failed to remove file: " << filePath.toStdString();
        }
        return result;
    }, "[SafeSystemExecutor]", "removeFile");
}

void SafeSystemExecutor::setTimeout(int milliseconds)
{
    timeoutMs_ = milliseconds;
}

void SafeSystemExecutor::setWorkingDirectory(const QString& directory)
{
    workingDirectory_ = directory;
}

void SafeSystemExecutor::setEnvironment(const QStringList& environment)
{
    environment_ = environment;
}

bool SafeSystemExecutor::executeInternal(const QString& command, bool async, CompletionCallback callback,
                                        QString* output, QString* error)
{
    return common::ErrorHandler::safeExecute([&]() -> bool {
        if (!validateCommand(command)) {
            QString errorMsg = QString("Invalid or unsafe command: %1").arg(command);
            OPENAUTO_LOG(error) << "[SafeSystemExecutor] " << errorMsg.toStdString();
            if (error) *error = errorMsg;
            if (callback) callback(-1, "", errorMsg);
            return false;
        }
        
        QString sanitizedCommand = sanitizeCommand(command);
        OPENAUTO_LOG(info) << "[SafeSystemExecutor] Executing: " << sanitizedCommand.toStdString();
        
        auto process = std::make_unique<QProcess>();
        
        // Configure process
        process->setWorkingDirectory(workingDirectory_);
        if (!environment_.isEmpty()) {
            process->setEnvironment(environment_);
        }
        
        if (async) {
            // Asynchronous execution
            if (callback) {
                QObject::connect(process.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    [callback, proc = process.get()](int exitCode, QProcess::ExitStatus) {
                        QString out = proc->readAllStandardOutput();
                        QString err = proc->readAllStandardError();
                        callback(exitCode, out, err);
                    });
            }
            
            process->start(sanitizedCommand);
            if (!process->waitForStarted(5000)) {
                QString errorMsg = QString("Failed to start command: %1").arg(process->errorString());
                OPENAUTO_LOG(error) << "[SafeSystemExecutor] " << errorMsg.toStdString();
                if (callback) callback(-1, "", errorMsg);
                return false;
            }
            
            // For async, we consider successful start as success
            // The actual result will be delivered via callback
            process.release(); // Transfer ownership to Qt's parent-child system
            return true;
        } else {
            // Synchronous execution
            process->start(sanitizedCommand);
            if (!process->waitForStarted(5000)) {
                QString errorMsg = QString("Failed to start command: %1").arg(process->errorString());
                OPENAUTO_LOG(error) << "[SafeSystemExecutor] " << errorMsg.toStdString();
                if (error) *error = errorMsg;
                return false;
            }
            
            if (!process->waitForFinished(timeoutMs_)) {
                process->kill();
                process->waitForFinished(5000);
                QString errorMsg = QString("Command timed out after %1ms").arg(timeoutMs_);
                OPENAUTO_LOG(error) << "[SafeSystemExecutor] " << errorMsg.toStdString();
                if (error) *error = errorMsg;
                return false;
            }
            
            int exitCode = process->exitCode();
            if (output) *output = process->readAllStandardOutput();
            if (error) *error = process->readAllStandardError();
            
            if (exitCode != 0) {
                OPENAUTO_LOG(warning) << "[SafeSystemExecutor] Command exited with code: " << exitCode;
                return false;
            }
            
            return true;
        }
    }, "[SafeSystemExecutor]", "executeInternal");
}

bool SafeSystemExecutor::validateCommand(const QString& command) const
{
    if (command.isEmpty()) {
        return false;
    }
    
    // Basic security checks
    QStringList dangerousPatterns = {
        ";", "&&", "||", "|", ">", ">>", "<", "`", "$(", "${", "rm -rf", "dd if=", "mkfs", "fdisk"
    };
    
    for (const QString& pattern : dangerousPatterns) {
        if (command.contains(pattern)) {
            OPENAUTO_LOG(warning) << "[SafeSystemExecutor] Dangerous pattern detected: " << pattern.toStdString();
            return false;
        }
    }
    
    // Only allow specific known commands
    QStringList allowedCommands = {
        "/usr/local/bin/autoapp_helper",
        "/usr/local/bin/crankshaft", 
        "crankshaft"
    };
    
    QString commandStart = command.split(' ').first();
    for (const QString& allowed : allowedCommands) {
        if (commandStart == allowed || commandStart.endsWith("/" + allowed)) {
            return true;
        }
    }
    
    OPENAUTO_LOG(warning) << "[SafeSystemExecutor] Command not in allowed list: " << commandStart.toStdString();
    return false;
}

QString SafeSystemExecutor::sanitizeCommand(const QString& command) const
{
    // For now, just return the command as-is since we've already validated it
    // In the future, this could perform additional sanitization
    return command;
}

} // namespace f1x::openauto::autoapp::system
