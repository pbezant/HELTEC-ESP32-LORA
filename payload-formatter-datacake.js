function decodeUplink(input) {
  // For Datacake, we need to handle base64 input
  if (!input || !input.bytes) {
      return [];
  }

  var bytes = input.bytes;
  
  try {
      // Temperature (2 bytes, scaled by 100)
      var tempRaw = (bytes[0] << 8) | bytes[1];
      var temperature = tempRaw / 100;
      
      // Humidity (1 byte, scaled by 2)
      var humidity = bytes[2] / 2;
      
      // Pressure (2 bytes, offset by 900, scaled by 10)
      var pressRaw = (bytes[3] << 8) | bytes[4];
      var pressure = (pressRaw / 10) + 900;
      
      // Motion detected (1 byte)
      var motion = bytes[5] === 1;
      
      // RSSI (2 bytes)
      var rssi = (bytes[6] << 8) | bytes[7];

      // Return Datacake formatted fields
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
  } catch (e) {
      return [];
  }
}

function Decoder(bytes, port) {
  // Datacake legacy support
  return decodeUplink({ bytes: bytes, fPort: port });
}