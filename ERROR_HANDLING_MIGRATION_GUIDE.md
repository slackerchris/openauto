# Error Handling Migration Guide

## Overview
This guide shows how to systematically replace generic `catch(...)` blocks with specific error handling using the new ErrorHandler infrastructure.

## Before and After Examples

### Example 1: Simple Operation with Fallback

**BEFORE:**
```cpp
void SomeService::performOperation() {
    try {
        // Some risky operation
        riskOperation();
    } catch (...) {
        OPENAUTO_LOG(error) << "Operation failed";
    }
}
```

**AFTER:**
```cpp
#include <f1x/openauto/Common/ErrorHandler.hpp>

void SomeService::performOperation() {
    common::ErrorHandler::safeExecute([this]() {
        riskOperation();
    }, "[SomeService::performOperation]", "risky operation");
}
```

### Example 2: Operation with Custom Error Handling

**BEFORE:**
```cpp
void AudioService::initializeAudio() {
    try {
        audioDevice_->initialize();
        audioDevice_->start();
    } catch (...) {
        OPENAUTO_LOG(error) << "Audio initialization failed";
        // No recovery attempted
    }
}
```

**AFTER:**
```cpp
void AudioService::initializeAudio() {
    common::ErrorHandler::safeExecuteWithHandler([this]() {
        audioDevice_->initialize();
        audioDevice_->start();
    }, "[AudioService::initializeAudio]", "audio initialization", 
    [this](const auto& error) {
        // Custom error handling
        handleAudioError(error);
    });
}

void AudioService::handleAudioError(const std::exception& error) {
    OPENAUTO_LOG(warning) << "[AudioService] Attempting audio recovery after error: " << error.what();
    
    // Reset and retry
    audioDevice_->reset();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    try {
        audioDevice_->initialize();
    } catch (const std::exception& retryError) {
        OPENAUTO_LOG(error) << "[AudioService] Audio recovery failed: " << retryError.what();
    }
}
```

### Example 3: Resource Management with RAII

**BEFORE:**
```cpp
void VideoService::processFrame() {
    VideoFrame* frame = nullptr;
    try {
        frame = allocateFrame();
        processVideoFrame(frame);
        deallocateFrame(frame);
    } catch (...) {
        if (frame) {
            deallocateFrame(frame);  // Manual cleanup
        }
        OPENAUTO_LOG(error) << "Frame processing failed";
    }
}
```

**AFTER:**
```cpp
void VideoService::processFrame() {
    common::ErrorHandler::safeExecute([this]() {
        VideoFrame* frame = allocateFrame();
        auto frameGuard = common::makeResourceGuard(*frame, [this](VideoFrame& f) {
            deallocateFrame(&f);
        });
        
        processVideoFrame(frame);
        frameGuard.release(); // Success - disable automatic cleanup
    }, "[VideoService::processFrame]", "video frame processing");
}
```

## Migration Checklist

### Step 1: Identify Generic Exception Handlers
```bash
grep -r "catch\s*(\s*\.\.\.\s*)" src/ --include="*.cpp"
```

### Step 2: Classify Each Handler
- **Simple operations**: Use `ErrorHandler::safeExecute()`
- **Complex error handling**: Use `ErrorHandler::safeExecuteWithHandler()`
- **Resource management**: Use `ResourceGuard`
- **AASDK-specific**: Use `ErrorHandler::logAasdkError()`

### Step 3: Apply Patterns
1. Add ErrorHandler include
2. Replace catch block with appropriate pattern
3. Add specific error handling if needed
4. Test error scenarios

### Step 4: Verify Improvements
- Check logs are more informative
- Verify resource cleanup works
- Test error recovery paths
- Ensure no regressions

## Common Error Types to Handle

### AASDK Errors
```cpp
catch (const aasdk::error::Error& e) {
    common::ErrorHandler::logAasdkError(e, "[ClassName]", "operation name");
    if (common::ErrorHandler::isRecoverableError(e)) {
        // Attempt recovery
    }
}
```

### Qt Errors
```cpp
catch (const QException& e) {
    OPENAUTO_LOG(error) << "[Context] Qt error: " << e.what();
}
```

### Standard Library Errors
```cpp
catch (const std::bad_alloc& e) {
    OPENAUTO_LOG(fatal) << "[Context] Memory allocation failed: " << e.what();
}
catch (const std::runtime_error& e) {
    OPENAUTO_LOG(error) << "[Context] Runtime error: " << e.what();
}
catch (const std::logic_error& e) {
    OPENAUTO_LOG(error) << "[Context] Logic error: " << e.what();
}
```

## Priority Order for Migration

1. **Critical paths**: App startup/shutdown, service lifecycle
2. **Resource management**: File I/O, device handling, memory allocation
3. **Network/communication**: USB, TCP, Bluetooth operations  
4. **UI operations**: User interactions, media playback
5. **Utility functions**: Configuration, logging, helpers

## Testing Your Changes

```cpp
// Example unit test
TEST(ErrorHandlingTest, SafeExecuteHandlesExceptions) {
    bool errorHandled = false;
    
    bool success = common::ErrorHandler::safeExecute([&]() {
        throw std::runtime_error("Test error");
    }, "[Test]", "test operation");
    
    EXPECT_FALSE(success);
    // Verify error was logged appropriately
}
```

Remember: The goal is not just to replace generic handlers, but to add meaningful error recovery and better diagnostics!
