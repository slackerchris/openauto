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

#include <f1x/openauto/Common/ErrorHandler.hpp>
#include <aasdk/Error/ErrorCode.hpp>

namespace f1x::openauto::common {

void ErrorHandler::logAasdkError(const aasdk::error::Error& error, const std::string& context, const std::string& operation) {
    OPENAUTO_LOG(error) << context << " - AASDK Error during " << operation;
    OPENAUTO_LOG(error) << "  Error Code: " << static_cast<int>(error.getCode());
    OPENAUTO_LOG(error) << "  Error Message: " << error.what();
    
    // Add specific handling for known error codes
    switch (error.getCode()) {
        case aasdk::error::ErrorCode::USB_TRANSFER:
            OPENAUTO_LOG(error) << "  USB transfer error - check USB connection";
            break;
        case aasdk::error::ErrorCode::OPERATION_ABORTED:
            OPENAUTO_LOG(warning) << "  Operation was aborted - this may be expected during shutdown";
            break;
        case aasdk::error::ErrorCode::OPERATION_IN_PROGRESS:
            OPENAUTO_LOG(warning) << "  Operation already in progress";
            break;
        case aasdk::error::ErrorCode::USB_CLAIM_INTERFACE:
            OPENAUTO_LOG(error) << "  USB interface claim failed - device may be in use";
            break;
        case aasdk::error::ErrorCode::USB_INVALID_DEVICE_HANDLE:
            OPENAUTO_LOG(error) << "  Invalid USB device handle - device may have been disconnected";
            break;
        default:
            OPENAUTO_LOG(error) << "  Unknown AASDK error code";
            break;
    }
}

bool ErrorHandler::isRecoverableError(const aasdk::error::Error& error) {
    switch (error.getCode()) {
        case aasdk::error::ErrorCode::OPERATION_ABORTED:
        case aasdk::error::ErrorCode::OPERATION_IN_PROGRESS:
            // These are often expected during normal operation
            return true;
        case aasdk::error::ErrorCode::USB_TRANSFER:
        case aasdk::error::ErrorCode::USB_CLAIM_INTERFACE:
        case aasdk::error::ErrorCode::USB_INVALID_DEVICE_HANDLE:
            // USB errors might be recoverable by reconnecting
            return true;
        default:
            // Conservative approach: treat unknown errors as non-recoverable
            return false;
    }
}

} // namespace f1x::openauto::common
