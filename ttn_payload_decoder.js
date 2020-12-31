function Decoder(bytes, port) {
  var temperature = (bytes[0] << 8) | bytes[1];
  var humidity = (bytes[2] << 8) | bytes[3];
  var pressure = ((bytes[4]) << 24) + ((bytes[5]) << 16) + ((bytes[6]) << 8) + ((bytes[7]));
  var battery = (bytes[8] << 8) | bytes[9];

  var data = {};

  // Verify that values are in range of the sensor
  // https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/#technical
  if (temperature / 100 >= -45 && temperature / 100 <= 85) {
    data.temperature = temperature / 100;
  }
  if (humidity >= 0 && humidity <= 100) {
    data.humidity = humidity;
  }
  if (pressure >= 300 && pressure <= 1100) {
    data.pressure = pressure;
  }
  if (battery / 1000 >= 2 && battery / 1000 <= 5) {
    data.voltage = battery / 1000;
  }

  return data;
}
