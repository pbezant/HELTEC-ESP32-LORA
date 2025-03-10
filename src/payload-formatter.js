// Constants for sensor ranges and configuration
const SENSOR_LIMITS = {
  TEMPERATURE: {
    MIN: -40,
    MAX: 85,
    SCALE_FACTOR: 10,   // Correctly adjusted for temperature
    OFFSET: 0          
  },
  HUMIDITY: {
    MIN: 0,
    MAX: 100,
    SCALE_FACTOR: 10,   // Adjusted to match payload format
    OFFSET: 0          
  },
  PRESSURE: {
    MIN: 900,
    MAX: 1100,
    OFFSET: 0,       
    SCALE_FACTOR: 10   
  },
  RSSI: {
    MIN: -120,        
    MAX: 0           
  }
};

// Byte positions in payload - Updated to match actual payload format from main.cpp
const BYTE_POSITIONS = {
  TEMPERATURE: { START: 0, LENGTH: 2 },
  HUMIDITY: { START: 2, LENGTH: 2 },    // Updated to match actual position
  PRESSURE: { START: 4, LENGTH: 2 },    // Updated to match actual position
  MOTION: { START: 6, LENGTH: 1 },      // Motion flag is at byte 6
  RESERVED: { START: 7, LENGTH: 1 }     // Reserved byte at position 7
};

// Downlink message types
const DOWNLINK_TYPES = {
  CONFIG: 0x01,
  COMMAND: 0x02
};

// Downlink commands
const COMMANDS = {
  RESET: 0x01,
  FORCE_READ: 0x02
};

// Utility functions for byte conversion
const ByteConverter = {
  toInt16: (bytes, startIndex) => {
    const value = (bytes[startIndex] << 8) | bytes[startIndex + 1];
    return value > 0x7FFF ? value - 0x10000 : value;
  },

  toUInt16: (bytes, startIndex) => {
    return (bytes[startIndex] << 8) | bytes[startIndex + 1];
  },
  
  getBit: (byte, bitPosition) => {
    return (byte >> bitPosition) & 1;
  }
};

// Sensor data decoders
const SensorDecoder = {
  temperature: (bytes) => {
    // Temperature calculation - correctly adjusted
    const raw = ByteConverter.toInt16(bytes, BYTE_POSITIONS.TEMPERATURE.START);
    return raw / SENSOR_LIMITS.TEMPERATURE.SCALE_FACTOR;
  },

  humidity: (bytes) => {
    // Updated to match actual format - humidity is 2 bytes at position 2-3
    const raw = ByteConverter.toUInt16(bytes, BYTE_POSITIONS.HUMIDITY.START);
    return raw / SENSOR_LIMITS.HUMIDITY.SCALE_FACTOR;
  },

  pressure: (bytes) => {
    const raw = ByteConverter.toUInt16(bytes, BYTE_POSITIONS.PRESSURE.START);
    // According to main.cpp, pressure was divided by 10 before sending
    return raw * 10; // Convert back to original value
  },
  
  motion: (bytes) => {
    // Motion is now bit 0 of byte 6, exactly as sent in main.cpp
    return (bytes[BYTE_POSITIONS.MOTION.START] & 0x01) === 0x01;
  }
};

// Validation functions
const Validator = {
  isInRange: (value, min, max) => {
    return value >= min && value <= max;
  },

  validateReading: (name, value, limits) => {
    if (limits && (limits.MIN !== undefined || limits.MAX !== undefined)) {
      return Validator.isInRange(
        value,
        limits.MIN !== undefined ? limits.MIN : -Infinity,
        limits.MAX !== undefined ? limits.MAX : Infinity
      );
    }
    return true;
  }
};

// Main decoder function
function decodeUplink(input) {
  try {
    // Check if input is valid
    if (!input || !input.bytes || !Array.isArray(input.bytes)) {
      throw new Error('Invalid input format');
    }

    // Check payload length
    const EXPECTED_LENGTH = 8; // Matches current payload length
    if (input.bytes.length !== EXPECTED_LENGTH) {
      throw new Error(`Invalid payload length. Expected ${EXPECTED_LENGTH} bytes, got ${input.bytes.length}`);
    }

    // Enhanced motion detection analysis
    const motionByte = input.bytes[BYTE_POSITIONS.MOTION.START];
    const motionDetected = (motionByte & 0x01) === 0x01;

    // Add debug information
    const debugRawValues = {
      tempRaw: ByteConverter.toInt16(input.bytes, BYTE_POSITIONS.TEMPERATURE.START),
      tempValue: SensorDecoder.temperature(input.bytes),
      humidityRaw: ByteConverter.toUInt16(input.bytes, BYTE_POSITIONS.HUMIDITY.START),
      humidityValue: SensorDecoder.humidity(input.bytes),
      pressureRaw: ByteConverter.toUInt16(input.bytes, BYTE_POSITIONS.PRESSURE.START),
      pressureActual: SensorDecoder.pressure(input.bytes),
      motionByte: motionByte,
      motionDetected: motionDetected
    };

    // Decode sensor data
    const decoded = {
      temperature: {
        celsius: SensorDecoder.temperature(input.bytes),
        fahrenheit: (SensorDecoder.temperature(input.bytes) * 9/5) + 32
      },
      humidity: SensorDecoder.humidity(input.bytes),
      pressure: SensorDecoder.pressure(input.bytes),
      motion_detected: motionDetected,
      debug: {
        raw_values: debugRawValues,
        raw_bytes: Array.from(input.bytes).map(b => b.toString(16).padStart(2, '0')).join(''),
        notes: "Payload mapping: temp(0-1), humidity(2-3), pressure(4-5), motion(6), reserved(7)"
      }
    };

    // Validate readings
    decoded.status = {
      temperature_valid: Validator.validateReading('temperature', decoded.temperature.celsius, SENSOR_LIMITS.TEMPERATURE),
      humidity_valid: Validator.validateReading('humidity', decoded.humidity, SENSOR_LIMITS.HUMIDITY),
      pressure_valid: Validator.validateReading('pressure', decoded.pressure, SENSOR_LIMITS.PRESSURE)
    };

    return {
      data: decoded,
      warnings: [],
      errors: []
    };

  } catch (error) {
    return {
      data: {},
      warnings: [],
      errors: [error.message]
    };
  }
}

// Downlink decoder and encoder
function decodeDownlink(input) {
  const decoded = {
    type: input.bytes[0] === DOWNLINK_TYPES.CONFIG ? 'config' : 'command',
    action: 'unknown'
  };

  if (input.bytes[0] === DOWNLINK_TYPES.COMMAND) {
    switch(input.bytes[1]) {
      case COMMANDS.RESET:
        decoded.action = 'reset';
        break;
      case COMMANDS.FORCE_READ:
        decoded.action = 'force_read';
        break;
    }
  } else if (input.bytes[0] === DOWNLINK_TYPES.CONFIG && input.bytes.length >= 5) {
    decoded.action = 'set_interval';
    decoded.value = (input.bytes[2] << 16) | (input.bytes[3] << 8) | input.bytes[4];
  }
  
  return {
    data: decoded,
    warnings: [],
    errors: []
  };
}

function encodeDownlink(input) {
  let bytes = [];
  
  switch(input.data.command) {
    case 'reset':
      bytes = [DOWNLINK_TYPES.COMMAND, COMMANDS.RESET];
      break;
    case 'force_read':
      bytes = [DOWNLINK_TYPES.COMMAND, COMMANDS.FORCE_READ];
      break;
    case 'set_interval':
      if (typeof input.data.value === 'number') {
        bytes = [
          DOWNLINK_TYPES.CONFIG,
          0x01,
          (input.data.value >> 16) & 0xFF,
          (input.data.value >> 8) & 0xFF,
          input.data.value & 0xFF
        ];
      }
      break;
  }
  
  return {
    bytes: bytes,
    fPort: 1
  };
} 