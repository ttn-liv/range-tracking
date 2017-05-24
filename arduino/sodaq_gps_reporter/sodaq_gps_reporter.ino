/*
 * Original Author: JP Meijers
 * Date: 2016-09-02
 *
 * Modified by DefProc
 * Date: May 2017
 *
 * Transmit GPS coordinates via TTN. This happens as fast as possible, while still keeping to
 * the 1% duty cycle rules enforced by the RN2483's built in LoRaWAN stack. Even though this is
 * allowed by the radio regulations of the 868MHz band, the fair use policy of TTN may prohibit this.
 *
 * CHECK THE RULES BEFORE USING THIS PROGRAM!
 *
 * CHANGE ADDRESS!
 * Change the device address, network (session) key, and app (session) key to the values
 * that are registered via the TTN dashboard.
 *
 * This sketch assumes you are using the Sodaq One rev2 node in its original configuration.
 *
 * This program makes use of the Sodaq UBlox library, but with minor changes to include altitude and HDOP.
 *
 * LED indicators:
 * Blue: Busy transmitting a packet
 * Green waiting for a new GPS fix
 * Red: GPS fix taking a long time. Try to go outdoors.
 *
 * To decode the binary payload, use `../../console/payload_function.js`
 * as the payload function in the TTN console
 *
 */

#include "Sodaq_UBlox_GPS.h" // https://github.com/SodaqMoja/Sodaq_UBlox_GPS
#include <TheThingsNetwork.h> // https://github.com/TheThingsNetwork/arduino-device-lib

// gps-lorawan-mapping
const byte appEui[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const byte appKey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

#define debugSerial SerialUSB
#define loraSerial Serial1
TheThingsNetwork ttn;

String toLog;
uint8_t txBuffer[9];
uint32_t LatitudeBinary, LongitudeBinary;
uint16_t altitudeGps;
uint8_t hdopGps;
int dr = 0;

void do_flash_led(int pin, uint8_t num = 2)
{
  if (num >= 10){ num = 10; } // max 10 flashes
  pinMode(pin, OUTPUT);
  for (uint8_t i = 0; i < num; ++i) {
    digitalWrite(pin, LOW); // on
    delay(50);
    digitalWrite(pin, HIGH); // off
    delay(150);
  }
  pinMode(pin, INPUT_PULLUP);
}

void setup()
{
    SerialUSB.begin(115200);
    loraSerial.begin(57600);

    // make sure usb serial connection is available,
    // or after 5s go on anyway for 'headless' use of the node.
    while ((!SerialUSB) && (millis() < 5000));

    SerialUSB.println(F("SODAQ ONE TTN Mapper starting"));

    // Hard reset the RN module
    #if defined(LORA_RESET)
    pinMode(LORA_RESET, OUTPUT);
    digitalWrite(LORA_RESET, LOW);
    delay(100);
    digitalWrite(LORA_RESET, HIGH);
    delay(100);
    #endif

    // initialize_radio();
    ttn.init(loraSerial, debugSerial);

    // Enable next line to enable debug information of the sodaq_gps
    //sodaq_gps.setDiag(SerialUSB);

    // initialize GPS
    sodaq_gps.init(GPS_ENABLE);

    ttn.reset();
    while (!ttn.join(appEui, appKey)){
      // didn't join the network
      debugSerial.print(F("couldn't join The Things Network…"));
      do_flash_led(LED_RED, 5);
      delay(6000);
      debugSerial.println(F("retrying"));
    }
    ttn.showStatus();

    // LED pins as outputs. HIGH=Off, LOW=On
    pinMode(LED_BLUE, OUTPUT);
    digitalWrite(LED_BLUE, HIGH);
    pinMode(LED_RED, OUTPUT);
    digitalWrite(LED_RED, HIGH);
    pinMode(LED_GREEN, OUTPUT);
    digitalWrite(LED_GREEN, HIGH);

    debugSerial.println(F("Setup for The Things Network."));
    do_flash_led(LED_GREEN, 5);

  delay(1000);
}

void loop()
{
  SerialUSB.println(F("Waiting for GPS fix"));

  do_flash_led(LED_GREEN, 1);
  // Keep the GPS enabled after we do a scan, increases accuracy
  // but also increases battery usage
  sodaq_gps.scan(true);

  // if the latitude is 0, we likely do not have a GPS fix yet, so wait longer
  while(sodaq_gps.getLat()==0.0)
  {
    SerialUSB.println(F("Latitude still 0.0, doing another scan"));

    do_flash_led(LED_RED, 3);
    // Keep the GPS enabled after we do a scan, increases accuracy
    sodaq_gps.scan(true);
  }
  do_flash_led(LED_GREEN, 3);

  LatitudeBinary = ((sodaq_gps.getLat() + 90) / 180) * 16777215;
  LongitudeBinary = ((sodaq_gps.getLon() + 180) / 360) * 16777215;

  txBuffer[0] = ( LatitudeBinary >> 16 ) & 0xFF;
  txBuffer[1] = ( LatitudeBinary >> 8 ) & 0xFF;
  txBuffer[2] = LatitudeBinary & 0xFF;

  txBuffer[3] = ( LongitudeBinary >> 16 ) & 0xFF;
  txBuffer[4] = ( LongitudeBinary >> 8 ) & 0xFF;
  txBuffer[5] = LongitudeBinary & 0xFF;

  altitudeGps = sodaq_gps.getAlt();
  txBuffer[6] = ( altitudeGps >> 8 ) & 0xFF;
  txBuffer[7] = altitudeGps & 0xFF;

  hdopGps = sodaq_gps.getHDOP()*10;
  txBuffer[8] = hdopGps & 0xFF;

  toLog = "";
  for(size_t i = 0; i<sizeof(txBuffer); i++)
  {
    char buffer[3];
    sprintf(buffer, "%02x", txBuffer[i]);
    toLog = toLog + String(buffer);
  }

  SerialUSB.print("Transmit on DR");
  SerialUSB.print(dr);
  SerialUSB.print(" coordinates ");
  SerialUSB.print(sodaq_gps.getLat(), 13);
  SerialUSB.print(" ");
  SerialUSB.print(sodaq_gps.getLon(), 13);
  SerialUSB.print(" altitude ");
  SerialUSB.print(sodaq_gps.getAlt(), 1);
  SerialUSB.print(" and HDOP ");
  SerialUSB.print(sodaq_gps.getHDOP(), 2);
  SerialUSB.print(" hex ");
  SerialUSB.println(toLog);

  do_flash_led(LED_BLUE, 2);
  ttn.sendBytes(txBuffer, sizeof(txBuffer));
  do_flash_led(LED_BLUE, 5);

  SerialUSB.println(F("TX done"));

  // TODO: turn the lorawan to sleep, power down the GPS and Accelerometer
  // and put the processor to sleep
  delay(30000); // wait 30seconds
}
