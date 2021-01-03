# LoRa Sensor Node

This project uses an Heltec Cubecell combined with a BME280 sensor to get
temperature, humidity and pressure information via LoRaWan.




## Getting the data into some useful form

### telegraf

I'm using telegraf mqtt_consumer to get the data from TTN into my InfluxDB. Here is my config snippet:

```
[[inputs.mqtt_consumer]]
  servers = ["tcp://eu.thethings.network:1883"]
  qos = 0
  connection_timeout = "30s"
  topics = [ "+/devices/+/up" ]
  client_id = "ttn"
  username = "lorasensor"
  password = "ttn-account-v2.password"
  data_format = "json"

```


### Grafana

I included my dashboard for these sensors. See (grafana_dashboard.json)[./grafana_dashboard.json]
