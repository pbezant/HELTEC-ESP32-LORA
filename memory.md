# Development Memory Log

## Implementation Decisions

### Library Structure
- Created three separate libraries (LoRaManager, DisplayManager, SensorManager) to encapsulate distinct functionality
- Each library has its own examples directory to demonstrate usage independently
- Libraries implement clean interfaces to hide implementation details from main application

### Build Configuration 
- Standardized on GNU++14 to ensure modern C++ features are available
- Added comprehensive warning flags (-Wall, -Wextra) to catch potential issues early
- Enabled high debug level (CORE_DEBUG_LEVEL=5) during development for verbose logging

## Edge Cases Handled

### Hardware Initialization
- Added sequential initialization for hardware components to prevent conflicts
- Implemented timeouts for sensor initialization to handle connection failures gracefully
- Created fallback modes for when sensors or display hardware is unavailable

### LoRa Communication
- Added packet validation to filter out corrupted transmissions
- Implemented retry logic for failed transmissions
- Created buffer management to handle varying payload sizes

## Problems Solved

### Memory Constraints
- Optimized string handling to reduce RAM usage
- Moved constant data to PROGMEM where appropriate
- Managed dynamic memory allocation carefully to prevent heap fragmentation

### Power Management
- Implemented sleep modes for the ESP32 to conserve battery during idle periods
- Created power-efficient sensing schedules based on application requirements
- Optimized radio transmission power based on required range

### Build Issues
- Resolved library dependency conflicts by specifying exact versions
- Fixed upload issues with serial port detection by configuring reset parameters

## Rejected Approaches

### Wireless Communication
- **Rejected**: Using WiFi as primary communication method
  - **Reason**: Higher power consumption compared to LoRa and shorter range for field deployments
  
### Sensor Integration
- **Rejected**: Using I2C multiplexer for additional sensors
  - **Reason**: Added complexity and reduced reliability without significant benefit
  - **Alternative**: Direct I2C bus connections with careful address management

### Display Interface
- **Rejected**: Custom graphics library
  - **Reason**: Would require significant development effort
  - **Alternative**: U8g2 library provides comprehensive functionality with optimized performance

### Software Architecture
- **Rejected**: Event-driven architecture with callbacks
  - **Reason**: Increased complexity and potential for difficult-to-debug race conditions
  - **Alternative**: Simple procedural approach with clear state management

### Data Storage
- **Rejected**: Using SD card for local data storage
  - **Reason**: Added power consumption and potential failure point
  - **Alternative**: Transmit data immediately and use RTC memory for temporary storage 