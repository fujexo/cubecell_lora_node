#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <CayenneLPP.h>
#include "Seeed_BME280.h"
#include "ttnparams.h"

#define ALTITUDE 720
bool ENABLE_SERIAL = false;

/*
   set LoraWan_RGB to Active,the RGB active in loraWan
   RGB red means sending;
   RGB purple means joined done;
   RGB blue means RxWindow1;
   RGB yellow means RxWindow2;
   RGB green means received done;
*/

/*LoraWan channelsmask, default channels 0-7*/
uint16_t userChannelsMask[6]={ 0x00FF,0x0000,0x0000,0x0000,0x0000,0x0000 };
LoRaMacRegion_t loraWanRegion = ACTIVE_REGION;
DeviceClass_t  loraWanClass = LORAWAN_CLASS;
bool overTheAirActivation = LORAWAN_NETMODE;
bool loraWanAdr = LORAWAN_ADR;
bool keepNet = LORAWAN_NET_RESERVE;
bool isTxConfirmed = LORAWAN_UPLINKMODE;
uint8_t appPort = 2;
uint8_t confirmedNbTrials = 4;

float temperature, humidity, batteryVoltage;
long pressure;

BME280 bme280;

/*the application data transmission duty cycle.  value in [ms].*/
uint32_t appTxDutyCycle = 300000; // 5min
//uint32_t appTxDutyCycle = 30000; // 30s
//uint32_t appTxDutyCycle = 15000; // 15s


/* Prepares the payload of the frame */
static void prepareTxFrame( uint8_t port )
{

  // Enable the power on the VEXT pin so the sensor can start running
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
  delay(500);

  // Wait until the sensor is runnning
  if (!bme280.init()) {
    if(ENABLE_SERIAL) {
      Serial.println("BME280 error!");
    }
  }

  // Wait for a little bit longer so the sensor can initialize correctly
  delay(500);

  temperature = bme280.getTemperature();
  humidity = bme280.getHumidity();
  pressure = bme280.getPressure();

  Wire.end();

  // Turn the power to the sensor off again
  digitalWrite(Vext, HIGH);

  batteryVoltage = getBatteryVoltage();

  // Convert millivolts to volts for CayenneLPP
  batteryVoltage /= 1000;

  // Correct the pressure based on the approximate altitude
  int i_pressure_corrected = ((pressure / pow((1 - ALTITUDE / 44330), 5.255)) / 100);

  CayenneLPP lpp(LORAWAN_APP_DATA_MAX_SIZE);
  lpp.addTemperature(1, temperature);
  lpp.addRelativeHumidity(1, humidity);
  lpp.addBarometricPressure(1, i_pressure_corrected);
  lpp.addVoltage(1, batteryVoltage);

  appDataSize = lpp.getSize();
  memcpy(appData, lpp.getBuffer(), appDataSize);
}

void setup() {
  boardInitMcu();
  if(ENABLE_SERIAL) {
    Serial.begin(115200);
  }
#if(AT_SUPPORT)
  enableAt();
#endif

  deviceState = DEVICE_STATE_INIT;
  LoRaWAN.ifskipjoin();
  

}

void loop()
{
  switch ( deviceState )
  {
    case DEVICE_STATE_INIT:
      {
#if(AT_SUPPORT)
        getDevParam();
#endif
        printDevParam();
        LoRaWAN.init(loraWanClass, loraWanRegion);
        deviceState = DEVICE_STATE_JOIN;
        break;
      }
    case DEVICE_STATE_JOIN:
      {
        LoRaWAN.join();
        break;
      }
    case DEVICE_STATE_SEND:
      {
        prepareTxFrame( appPort );
        LoRaWAN.send();
        deviceState = DEVICE_STATE_CYCLE;
        break;
      }
    case DEVICE_STATE_CYCLE:
      {
        // Schedule next packet transmission
        txDutyCycleTime = appTxDutyCycle + randr( 0, APP_TX_DUTYCYCLE_RND );
        LoRaWAN.cycle(txDutyCycleTime);
        deviceState = DEVICE_STATE_SLEEP;
        break;
      }
    case DEVICE_STATE_SLEEP:
      {
        LoRaWAN.sleep();
        break;
      }
    default:
      {
        deviceState = DEVICE_STATE_INIT;
        break;
      }
  }
}
