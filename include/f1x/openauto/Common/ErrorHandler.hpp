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

#include <functional>
#include <string>
#include <aasdk/Error/Error.hpp>
#include <f1x/openauto/Common/Log.hpp>

namespace f1x::openauto::common {

/**
 * @brief Utility class for improved error handling and logging
 */
class ErrorHandler {
public:
    /**
     * @brief Execute a function with comprehensive exception handling
     * @param operation The operation to execute
     * @param context Context string for logging (e.g., "[ClassName::methodName]")
     * @param operationName Human-readable name of the operation
     * @return true if operation succeeded, false if exception was caught
     */
    template<typename Func>
    static bool safeExecute(Func&& operation, const std::string& context, const std::string& operationName) {
        try {
            operation();
            return true;
        } catch (const aasdk::error::Error& e) {
            OPENAUTO_LOG(error) << context << " - aasdk error during " << operationName << ": " << e.what();
            return false;
        } catch (const std::exception& e) {
            OPENAUTO_LOG(error) << context << " - standard exception during " << operationName << ": " << e.what();
            return false;
        } catch (...) {
            OPENAUTO_LOG(error) << context << " - unknown exception during " << operationName;
            return false;
        }
    }

    /**
     * @brief Execute a function with comprehensive exception handling and custom error handling
     * @param operation The operation to execute
     * @param context Context string for logging
     * @param operationName Human-readable name of the operation
     * @param errorHandler Custom error handler function
     * @return true if operation succeeded, false if exception was caught
     */
    template<typename Func, typename ErrorFunc>
    static bool safeExecuteWithHandler(Func&& operation, const std::string& context, 
                                     const std::string& operationName, ErrorFunc&& errorHandler) {
        try {
            operation();
            return true;
        } catch (const aasdk::error::Error& e) {
            OPENAUTO_LOG(error) << context << " - aasdk error during " << operationName << ": " << e.what();
            errorHandler(e);
            return false;
        } catch (const std::exception& e) {
            OPENAUTO_LOG(error) << context << " - standard exception during " << operationName << ": " << e.what();
            errorHandler(e);
            return false;
        } catch (...) {
            OPENAUTO_LOG(error) << context << " - unknown exception during " << operationName;
            errorHandler(std::runtime_error("Unknown exception"));
            return false;
        }
    }

    /**
     * @brief Log detailed error information for aasdk errors
     * @param error The aasdk error
     * @param context Context string for logging
     * @param operation Operation that failed
     */
    static void logAasdkError(const aasdk::error::Error& error, const std::string& context, const std::string& operation);

    /**
     * @brief Check if an aasdk error is recoverable
     * @param error The aasdk error to check
     * @return true if the error is considered recoverable
     */
    static bool isRecoverableError(const aasdk::error::Error& error);
};

/**
 * @brief RAII helper for resource cleanup with exception safety
 */
template<typename Resource, typename Cleanup>
class ResourceGuard {
public:
    ResourceGuard(Resource& resource, Cleanup cleanup) 
        : resource_(resource), cleanup_(cleanup), engaged_(true) {}
    
    ~ResourceGuard() {
        if (engaged_) {
            ErrorHandler::safeExecute([this]() {
                cleanup_(resource_);
            }, "[ResourceGuard]", "resource cleanup");
        }
    }
    
    void release() { engaged_ = false; }
    
    ResourceGuard(const ResourceGuard&) = delete;
    ResourceGuard& operator=(const ResourceGuard&) = delete;
    ResourceGuard(ResourceGuard&& other) noexcept : resource_(other.resource_), cleanup_(std::move(other.cleanup_)), engaged_(other.engaged_) {
        other.engaged_ = false;
    }
    
private:
    Resource& resource_;
    Cleanup cleanup_;
    bool engaged_;
};

/**
 * @brief Helper function to create a resource guard
 */
template<typename Resource, typename Cleanup>
auto makeResourceGuard(Resource& resource, Cleanup cleanup) {
    return ResourceGuard<Resource, Cleanup>(resource, cleanup);
}

} // namespace f1x::openauto::common
