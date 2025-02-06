function decodeUplink(input) {
  var bytes = input.bytes;
  var decoded = {};
  
  // Temperature (bytes 0-1)
  // Convert 2 bytes to signed int16 and divide by 100 to get temperature in °C
  var temp = (bytes[0] << 8 | bytes[1]);
  decoded.temperature = temp / 100;
  
  // Humidity (byte 2)
  // Divide by 2 to get humidity percentage with 1 decimal place
  decoded.humidity = bytes[2] / 2;
  
  // Pressure (bytes 3-4)
  // Convert 2 bytes to uint16, multiply by 0.1 and add 900 to get pressure in hPa
  var press = (bytes[3] << 8 | bytes[4]);
  decoded.pressure = (press / 10) + 900;
  
  // Light (bytes 5-6)
  // Convert 2 bytes to uint16 to get light level in lux
  decoded.light = (bytes[5] << 8 | bytes[6]);
  
  // Counter (byte 7)
  decoded.counter = bytes[7];
  
  // PIR Status (byte 8)
  decoded.motion_detected = bytes[8] === 1;
  
  // Add decoded data quality indicators
  decoded.status = {
    temperature_valid: decoded.temperature > -40 && decoded.temperature < 85,
    humidity_valid: decoded.humidity >= 0 && decoded.humidity <= 100,
    pressure_valid: decoded.pressure >= 900 && decoded.pressure <= 1100,
    light_valid: decoded.light >= 0
  };

  return {
    data: decoded,
    warnings: [],
    errors: []
  };
} 