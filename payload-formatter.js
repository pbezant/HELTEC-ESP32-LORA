// Constants for sensor ranges and configuration
const SENSOR_LIMITS = {
  TEMPERATURE: {
    MIN: -40,
    MAX: 85,
    SCALE_FACTOR: 100  // Divide by 100 to get actual value
  },
  HUMIDITY: {
    MIN: 0,
    MAX: 100,
    SCALE_FACTOR: 2    // Divide by 2 to get actual value
  },
  PRESSURE: {
    MIN: 900,
    MAX: 1100,
    OFFSET: 900,       // Add this to get actual value
    SCALE_FACTOR: 10   // Divide by 10 to get actual value
  },
  RSSI: {
    MIN: -120,        // Typical minimum RSSI value
    MAX: 0           // Typical maximum RSSI value
  }
};

// Byte positions in payload
const BYTE_POSITIONS = {
  TEMPERATURE: { START: 0, LENGTH: 2 },
  HUMIDITY: { START: 2, LENGTH: 1 },
  PRESSURE: { START: 3, LENGTH: 2 },
  COUNTER: { START: 7, LENGTH: 1 },
  MOTION: { START: 8, LENGTH: 1 },
  RSSI: { START: 9, LENGTH: 2 }
};

// Utility functions for byte conversion
const ByteConverter = {
  // Convert 2 bytes to signed int16
  toInt16: (bytes, startIndex) => {
    const value = (bytes[startIndex] << 8) | bytes[startIndex + 1];
    return value > 0x7FFF ? value - 0x10000 : value;
  },

  // Convert 2 bytes to unsigned int16
  toUInt16: (bytes, startIndex) => {
    return (bytes[startIndex] << 8) | bytes[startIndex + 1];
  }
};

// Sensor data decoders
const SensorDecoder = {
  temperature: (bytes) => {
    const raw = ByteConverter.toInt16(bytes, BYTE_POSITIONS.TEMPERATURE.START);
    const celsius = raw / SENSOR_LIMITS.TEMPERATURE.SCALE_FACTOR;
    return celsius;
  },

  humidity: (bytes) => {
    return bytes[BYTE_POSITIONS.HUMIDITY.START] / SENSOR_LIMITS.HUMIDITY.SCALE_FACTOR;
  },

  pressure: (bytes) => {
    const raw = ByteConverter.toUInt16(bytes, BYTE_POSITIONS.PRESSURE.START);
    return (raw / SENSOR_LIMITS.PRESSURE.SCALE_FACTOR) + SENSOR_LIMITS.PRESSURE.OFFSET;
  },

  counter: (bytes) => {
    return bytes[BYTE_POSITIONS.COUNTER.START];
  },

  motion: (bytes) => {
    return bytes[BYTE_POSITIONS.MOTION.START] === 1;
  },

  rssi: (bytes) => {
    return ByteConverter.toInt16(bytes, BYTE_POSITIONS.RSSI.START);
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
    const EXPECTED_LENGTH = 11; // Updated to include RSSI
    if (input.bytes.length !== EXPECTED_LENGTH) {
      throw new Error(`Invalid payload length. Expected ${EXPECTED_LENGTH} bytes, got ${input.bytes.length}`);
    }

    // Decode sensor data
    const decoded = {
      temperature: {
        celsius: SensorDecoder.temperature(input.bytes),
        fahrenheit: (SensorDecoder.temperature(input.bytes) * 9/5) + 32
      },
      humidity: SensorDecoder.humidity(input.bytes),
      pressure: SensorDecoder.pressure(input.bytes),
      counter: SensorDecoder.counter(input.bytes),
      motion_detected: SensorDecoder.motion(input.bytes),
      last_rssi: SensorDecoder.rssi(input.bytes)
    };

    // Validate readings
    decoded.status = {
      temperature_valid: Validator.validateReading('temperature', decoded.temperature.celsius, SENSOR_LIMITS.TEMPERATURE),
      humidity_valid: Validator.validateReading('humidity', decoded.humidity, SENSOR_LIMITS.HUMIDITY),
      pressure_valid: Validator.validateReading('pressure', decoded.pressure, SENSOR_LIMITS.PRESSURE),
      rssi_valid: Validator.validateReading('rssi', decoded.last_rssi, SENSOR_LIMITS.RSSI)
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