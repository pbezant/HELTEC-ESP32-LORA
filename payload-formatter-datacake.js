// Constants for sensor ranges and configuration
const SENSOR_LIMITS = {
  TEMPERATURE: { SCALE_FACTOR: 100 },
  HUMIDITY: { SCALE_FACTOR: 2 },
  PRESSURE: { 
    OFFSET: 900,
    SCALE_FACTOR: 10
  }
};

// Byte positions in payload
const BYTE_POSITIONS = {
  TEMPERATURE: { START: 0, LENGTH: 2 },
  HUMIDITY: { START: 2, LENGTH: 1 },
  PRESSURE: { START: 3, LENGTH: 2 },
  MOTION: { START: 5, LENGTH: 1 },
  RSSI: { START: 6, LENGTH: 2 }
};

// Utility functions for byte conversion
const ByteConverter = {
  toInt16: (bytes, startIndex) => {
    const value = (bytes[startIndex] << 8) | bytes[startIndex + 1];
    return value > 0x7FFF ? value - 0x10000 : value;
  },
  toUInt16: (bytes, startIndex) => {
    return (bytes[startIndex] << 8) | bytes[startIndex + 1];
  }
};

// Datacake expects a function named "decode"
function Decoder(payload) {
  try {
    // Handle TTN payload structure
    let bytes;
    if (typeof payload === 'object' && payload.uplink_message) {
      // Extract bytes from TTN payload
      const base64Payload = payload.uplink_message.frm_payload;
      bytes = Buffer.from(base64Payload, 'base64');
    } else if (Array.isArray(payload)) {
      // Direct byte array input
      bytes = payload;
    } else {
      throw new Error('Invalid payload format');
    }

    // Check payload length
    if (bytes.length !== 8) {
      throw new Error(`Invalid payload length: ${bytes.length}`);
    }

    // Temperature (°C)
    const tempRaw = ByteConverter.toInt16(bytes, BYTE_POSITIONS.TEMPERATURE.START);
    const temperature = tempRaw / SENSOR_LIMITS.TEMPERATURE.SCALE_FACTOR;
    
    // Humidity (%)
    const humidity = bytes[BYTE_POSITIONS.HUMIDITY.START] / SENSOR_LIMITS.HUMIDITY.SCALE_FACTOR;
    
    // Pressure (hPa)
    const pressRaw = ByteConverter.toUInt16(bytes, BYTE_POSITIONS.PRESSURE.START);
    const pressure = (pressRaw / SENSOR_LIMITS.PRESSURE.SCALE_FACTOR) + SENSOR_LIMITS.PRESSURE.OFFSET;
    
    // Motion detected (boolean)
    const motion = bytes[BYTE_POSITIONS.MOTION.START] === 1;
    
    // RSSI (dBm)
    const rssi = ByteConverter.toInt16(bytes, BYTE_POSITIONS.RSSI.START);

    // Additional metadata from TTN payload
    const metadata = [];
    if (payload.uplink_message && payload.uplink_message.rx_metadata && payload.uplink_message.rx_metadata[0]) {
      const rx = payload.uplink_message.rx_metadata[0];
      metadata.push(
        { field: "GATEWAY_ID", value: rx.gateway_ids.gateway_id },
        { field: "SNR", value: rx.snr },
        { field: "GATEWAY_RSSI", value: rx.rssi }
      );
    }

    // Return array of Datacake fields
    return [
      {
        field: "TEMPERATURE",
        value: temperature
      },
      {
        field: "TEMPERATURE_F",
        value: (temperature * 9/5) + 32
      },
      {
        field: "HUMIDITY",
        value: humidity
      },
      {
        field: "PRESSURE",
        value: pressure
      },
      {
        field: "MOTION",
        value: motion ? 1 : 0
      },
      {
        field: "RSSI",
        value: rssi
      }
    ];
  } catch (error) {
    console.error('Decoder error:', error);
    return [];
  }
}
