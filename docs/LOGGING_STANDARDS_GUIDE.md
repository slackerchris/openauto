# OpenAuto Logging Standards Guide - August 2025

**Date**: August 1, 2025  
**Author**: GitHub Copilot  
**Purpose**: Standardize logging practices across the OpenAuto codebase

## Overview

This document establishes consistent logging standards for the OpenAuto project to improve debugging, monitoring, and maintenance. These standards apply to all new code and should be gradually adopted in existing code during maintenance.

## Logging Levels

### 📊 **Log Level Usage Guidelines**

| Level | Purpose | When to Use | Examples |
|-------|---------|-------------|----------|
| **error** | Critical failures | System errors, exceptions, failed operations that affect functionality | Connection failures, file I/O errors, protocol violations |
| **warning** | Non-critical issues | Recoverable problems, deprecated usage, configuration issues | Missing optional configs, fallback behaviors, resource constraints |
| **info** | Important events | Service lifecycle, major state changes, user actions | Service start/stop, connections, configuration changes |
| **debug** | Detailed tracking | Development debugging, detailed state information | Internal state changes, detailed operation flows |
| **trace** | Very detailed flow | Step-by-step execution tracking (rarely used) | Function entry/exit, loop iterations |

## Message Format Standards

### 🎯 **Standard Format**
```cpp
OPENAUTO_LOG(level) << "[ComponentName] Action: details";
```

### **Format Components:**

1. **Component Name**: Class name or logical component in brackets
   - Use PascalCase class names: `[MediaPlayerController]`
   - Use logical names for utilities: `[SystemExecutor]`
   - Keep consistent within each file

2. **Action/Message**: Clear, concise description of what happened
   - Use present tense for states: `"Connected to device"`
   - Use past tense for completed actions: `"Configuration saved"`
   - Use present continuous for ongoing: `"Scanning files"`

3. **Details**: Additional context when helpful
   - Include relevant parameters, values, or context
   - Use key=value format for multiple details
   - Avoid sensitive information (passwords, tokens)

### **Examples of Good Logging:**

```cpp
// Service lifecycle
OPENAUTO_LOG(info) << "[BluetoothService] Service started";
OPENAUTO_LOG(info) << "[BluetoothService] Service stopped";

// State changes with details
OPENAUTO_LOG(debug) << "[UIStateManager] Night mode: " << (enabled ? "enabled" : "disabled");
OPENAUTO_LOG(debug) << "[MediaPlayer] Playback state changed: " << stateToString(newState);

// Operations with context
OPENAUTO_LOG(info) << "[ConfigurationManager] Configuration loaded from: " << configFile;
OPENAUTO_LOG(debug) << "[SystemController] Brightness set to: " << brightness << "%";

// Errors with context
OPENAUTO_LOG(error) << "[FileManager] Failed to read config file: " << filename << " - " << error.what();
OPENAUTO_LOG(warning) << "[NetworkManager] Connection timeout, retrying in " << retrySeconds << "s";
```

## Component Naming Conventions

### **Standard Component Names:**

| File/Class | Component Name | Example |
|------------|----------------|---------|
| `MediaPlayerController` | `[MediaPlayerController]` | `OPENAUTO_LOG(debug) << "[MediaPlayerController] Track changed: " << track;` |
| `UIStateManager` | `[UIStateManager]` | `OPENAUTO_LOG(info) << "[UIStateManager] Day mode activated";` |
| `SettingsController` | `[SettingsController]` | `OPENAUTO_LOG(debug) << "[SettingsController] Settings saved";` |
| `SystemController` | `[SystemController]` | `OPENAUTO_LOG(info) << "[SystemController] Volume set: " << level;` |
| `BluetoothService` | `[BluetoothService]` | `OPENAUTO_LOG(info) << "[BluetoothService] Device paired: " << deviceName;` |
| `AndroidAutoEntity` | `[AndroidAutoEntity]` | `OPENAUTO_LOG(info) << "[AndroidAutoEntity] Connection established";` |
| Service classes | `[ServiceName]` | `OPENAUTO_LOG(info) << "[MediaSinkService] Audio channel opened";` |
| Utility classes | `[UtilityName]` | `OPENAUTO_LOG(debug) << "[ErrorHandler] Exception handled safely";` |

## Anti-Patterns to Avoid

### ❌ **Don't Do This:**

```cpp
// Missing component name
OPENAUTO_LOG(info) << "Started";

// Inconsistent brackets
OPENAUTO_LOG(info) << "MainWindow: Started";
OPENAUTO_LOG(info) << "(BluetoothService) Connected";

// Unclear messages
OPENAUTO_LOG(debug) << "[Service] OK";
OPENAUTO_LOG(info) << "[Component] Something happened";

// Too verbose for level
OPENAUTO_LOG(error) << "[Manager] Button clicked, processing, checking state, updating UI";

// Missing context for errors
OPENAUTO_LOG(error) << "[FileManager] Failed";
```

### ✅ **Do This Instead:**

```cpp
// Clear component and message
OPENAUTO_LOG(info) << "[MainWindow] Application started";

// Consistent formatting
OPENAUTO_LOG(info) << "[BluetoothService] Device connected";

// Clear, specific messages
OPENAUTO_LOG(debug) << "[ServiceManager] Service health check passed";
OPENAUTO_LOG(info) << "[ConfigurationManager] Configuration validation completed";

// Appropriate verbosity
OPENAUTO_LOG(debug) << "[UIManager] Button clicked: " << buttonName;

// Contextual error information
OPENAUTO_LOG(error) << "[FileManager] Failed to read config: " << filename << " - " << error.what();
```

## Specific Use Cases

### **Initialization and Cleanup**
```cpp
OPENAUTO_LOG(info) << "[ComponentName] Initialized successfully";
OPENAUTO_LOG(info) << "[ComponentName] Shutdown completed";
OPENAUTO_LOG(debug) << "[ComponentName] Resource cleanup: " << resourceCount << " items";
```

### **Configuration and Settings**
```cpp
OPENAUTO_LOG(info) << "[ConfigManager] Configuration loaded: " << configFile;
OPENAUTO_LOG(debug) << "[SettingsController] Setting updated: " << key << "=" << value;
OPENAUTO_LOG(warning) << "[ConfigManager] Using default value for: " << missingKey;
```

### **Network and Connections**
```cpp
OPENAUTO_LOG(info) << "[NetworkManager] Connected to: " << endpoint;
OPENAUTO_LOG(warning) << "[NetworkManager] Connection timeout, retrying: attempt " << attemptNumber;
OPENAUTO_LOG(error) << "[NetworkManager] Connection failed: " << endpoint << " - " << error.what();
```

### **Media and Playback**
```cpp
OPENAUTO_LOG(info) << "[MediaPlayer] Now playing: " << trackTitle;
OPENAUTO_LOG(debug) << "[MediaPlayer] Playback position: " << position << "/" << duration;
OPENAUTO_LOG(warning) << "[MediaPlayer] Codec not supported: " << codecName;
```

### **System Operations**
```cpp
OPENAUTO_LOG(info) << "[SystemController] Brightness changed: " << percentage << "%";
OPENAUTO_LOG(debug) << "[SystemController] Command executed: " << command;
OPENAUTO_LOG(error) << "[SystemController] System command failed: " << command << " - " << exitCode;
```

## Implementation Guidelines

### **For New Code:**
- Follow these standards from the start
- Use appropriate log levels for the audience (end users vs developers)
- Include relevant context but avoid information overload

### **For Existing Code:**
- Update logging when making other changes to a file
- Focus on error and warning messages first (most important for troubleshooting)
- Gradually improve debug and info messages during maintenance

### **Testing Logging:**
- Verify log messages provide useful information for debugging
- Check that error messages include enough context to diagnose issues
- Ensure log levels are appropriate for the information being logged

## Performance Considerations

- Log level checks are handled by the logging framework
- Avoid expensive operations in log statements (use debug level for expensive string operations)
- Consider using lazy evaluation for complex log message construction

```cpp
// Good: Simple message
OPENAUTO_LOG(debug) << "[Component] State: " << simpleValue;

// Consider cost: Complex operation only if debug level enabled
if (OPENAUTO_LOG_LEVEL >= DEBUG) {
    OPENAUTO_LOG(debug) << "[Component] Complex state: " << expensiveOperation();
}
```

## Migration Priority

1. **High Priority**: Error and warning messages (critical for troubleshooting)
2. **Medium Priority**: Info messages (important for understanding application flow)
3. **Low Priority**: Debug messages (helpful for development)

This standardization will improve debugging efficiency, log readability, and overall maintainability of the OpenAuto project.
