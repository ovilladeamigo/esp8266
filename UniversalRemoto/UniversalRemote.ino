#include <Arduino.h>
#include <IRrecv.h>
#include <IRremoteESP8266.h>
#include <IRac.h>
#include <IRutils.h>

const uint16_t kIrLed = 4; // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRsend irsend(kIrLed);     // Set the GPIO to be used to sending the message.

const uint16_t kRecvPin = 14;
const uint32_t kBaudRate = 115200;
const uint16_t kCaptureBufferSize = 1024;

#if DECODE_AC
const uint8_t kTimeout = 50;
#else  // DECODE_AC
const uint8_t kTimeout = 15;
#endif // DECODE_AC

const uint16_t kMinUnknownSize = 12;

enum universal
{
  UP = 0x205DC03F,
  DOWN = 0x205D40BF,
  LEFT = 0x205D708F,
  RIGHT = 0x205D58A7,
  OK = 0x205DE01F,
  TV = 0x205D22DD,
  SOURCE = 0x205DCA35,
  VOL_UP = 0x205D38C7,
  VOL_DOWN = 0x205D38C7,
  CH_UP = 0x205D04FB,
  CH_DOWN = 0x205D847B,
  POWER = 0x205D38C7,
  REPEAT = 0xFFFFFFFFFFFFFFFF
};

// ==================== end of TUNEABLE PARAMETERS ====================

// Use turn on the save buffer feature for more complete capture coverage.
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results; // Somewhere to store the results

// This section of code runs only once at start-up.
void setup()
{
  irsend.begin();
#if defined(ESP8266)
  Serial.begin(kBaudRate, SERIAL_8N1, SERIAL_TX_ONLY);
#else             // ESP8266
  Serial.begin(kBaudRate, SERIAL_8N1);
#endif            // ESP8266
  while (!Serial) // Wait for the serial connection to be establised.
    delay(50);
  Serial.printf("\nIRrecvDumpV2 is now running and waiting for IR input on Pin "
                "%d\n",
                kRecvPin);
#if DECODE_HASH
  // Ignore messages with less than minimum on or off pulses.
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif                 // DECODE_HASH
  irrecv.enableIRIn(); // Start the receiver
}
void loop()
{
  // Check if the IR code has been received.
  if (irrecv.decode(&results))
  {
    switch (results.value)
    {
    case UP:
      irsend.sendNEC(0x20DF6A95, 32);
      Serial.println("--------------------------------- UP");
      break;
    case DOWN:
      irsend.sendNEC(0x20DFEA15, 32);
      Serial.println("--------------------------------- DOWN");
      break;
    case LEFT:
      irsend.sendNEC(0x20DF9A65, 32);
      Serial.println("--------------------------------- LEFT");
      break;
    case RIGHT:
      irsend.sendNEC(0x20DF1AE5, 32);
      Serial.println("--------------------------------- RIGHT");
      break;
    default:
      Serial.println("--------------------------------- NONE");

      break;
    }

    Serial.print(resultToHumanReadableBasic(&results));
    // Display any extra A/C info if we have it.
    //yield(); // Feed the WDT (again)
    delay(200);
    String returnRedad = resultToHexidecimal(&results);
    if (returnRedad == "205D44BB")
    {

      irsend.sendPanasonic64(0x400405000401, 48);
      Serial.println("      Send data volumen UP 400405000401");
    }
    else if (returnRedad == "205D10EF")
    {
      irsend.sendPanasonic64(0x400405008481, 48);
      Serial.println("      Send data volumen Down 400405008481");
    }
    else if (returnRedad == "205DC03F")
    {
      irsend.sendNEC(0x20DF6A95, 32);
      Serial.println("      Send data Up");
    }
    else if (returnRedad == "205D40BF")
    {
      irsend.sendNEC(0x20DFEA15, 32);
      Serial.println("      Send data down");
    }
    else if (returnRedad == "205D58A7")
    {
      irsend.sendNEC(0x20DF9A65, 32);
      Serial.println("      Send data right");
    }
    else if (returnRedad == "205D708F")
    {
      irsend.sendNEC(0x20DF1AE5, 32);
      Serial.println("      Send data left");
    }
    else if (returnRedad == "205DE01F")
    {
      irsend.sendNEC(0x20DF5AA5, 32);
      Serial.println("      Send data OK");
    }
    else if (returnRedad == "205DCA35")
    {
      irsend.sendNEC(0x20DFD02F, 32);
      Serial.println("      Send data Source");
    }
    else if (returnRedad == "205D22DD")
    {
      irsend.sendNEC(0x20DFB847, 32);
      Serial.println("      Send data TV");
    }
    else if (returnRedad == "205D38C7")
    {
      irsend.sendNEC(0x20DF10EF, 32);
      Serial.println("      Send Power TV");
      delay(2000);
      irsend.sendPanasonic64(0x40040500BCB9, 48);
      Serial.println("      Send Power Sound");
    }
  }
}