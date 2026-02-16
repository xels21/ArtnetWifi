/*
Example, transmit all received ArtNet messages (DMX) out of the serial port in plain text.

Stephan Ruloff 2016,2019
https://github.com/rstephan
*/
#include <ArtnetWifi.h>
#include <Arduino.h>

//Wifi settings - can be overridden via build flags
#ifndef WIFI_SSID
  #error "WIFI_SSID is not defined. Please define it in platformio.ini or in the code."
#endif

#ifndef WIFI_PASSWORD
  #error "WIFI_PASSWORD is not defined. Please define it in platformio.ini or in the code."
#endif

// DMX512 Output Configuration
#ifndef DMX_ENABLE
  #define DMX_ENABLE 0  // Set to 1 to enable DMX output via UART
#endif

#ifndef DMX_TX_PIN
  #define DMX_TX_PIN 17  // Default TX pin for Serial2 on ESP32
#endif

#ifndef DMX_DE_PIN
  #define DMX_DE_PIN 4  // Driver Enable pin for MAX485 (safe for all ESP32 variants)
#endif

#ifndef DMX_REFRESH_MS
  #define DMX_REFRESH_MS 10  // Maximum time between DMX frames in milliseconds
#endif

#define DMX_SERIAL_CONFIG SERIAL_8N2  // DMX512 requires 8 data bits, no parity, 2 stop bits

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

WiFiUDP UdpSend;
ArtnetWifi artnet;

#if DMX_ENABLE
uint8_t dmxData[513];  // DMX buffer: 0 = start code, 1-512 = channel data
uint16_t dmxDataLength = 512;  // Current DMX data length
unsigned long lastDmxSend = 0;  // Timestamp of last DMX frame sent
#endif

// connect to wifi – returns true if successful or false if not
bool ConnectWifi(void)
{
  bool state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(IPAddress(WiFi.localIP()));
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
  }
  
  return state;
}

#if DMX_ENABLE
void sendDMX512()
{
  // Enable MAX485 transmit mode
  digitalWrite(DMX_DE_PIN, HIGH);
  
  // Send BREAK (88-176 microseconds low)
  Serial2.end();
  pinMode(DMX_TX_PIN, OUTPUT);
  digitalWrite(DMX_TX_PIN, LOW);
  delayMicroseconds(88);
  
  // Send MAB (Mark After Break - 8-16 microseconds high)
  digitalWrite(DMX_TX_PIN, HIGH);
  delayMicroseconds(8);
  
  // Restart UART and send data
  Serial2.begin(250000, DMX_SERIAL_CONFIG, -1, DMX_TX_PIN);
  Serial2.write(dmxData, dmxDataLength + 1);  // +1 for start code
  Serial2.flush();
  
  // Update timestamp
  lastDmxSend = millis();
  
  // Optional: Disable MAX485 transmit after sending (if using half-duplex)
  // digitalWrite(DMX_DE_PIN, LOW);
}
#endif

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
#if DMX_ENABLE
  // Update DMX buffer with new data
  if (length > 512) length = 512;
  dmxData[0] = 0;  // DMX512 start code
  memcpy(&dmxData[1], data, length);
  dmxDataLength = length;
  
#endif
  // Optional debug output (reduced to avoid slowing down DMX)
  // static unsigned long lastDebug = 0;
  // if (millis() - lastDebug > 1000) {
  //   Serial.print("Art-Net: Univ ");
  //   Serial.print(universe);
  //   Serial.print(", Seq ");
  //   Serial.print(sequence);
  //   Serial.print(", Ch ");
  //   Serial.println(length);
  //   lastDebug = millis();
  // }

  // Debug mode: print to serial
  bool tail = false;
  
  Serial.print("DMX: Univ: ");
  Serial.print(universe, DEC);
  Serial.print(", Seq: ");
  Serial.print(sequence, DEC);
  Serial.print(", Data (");
  Serial.print(length, DEC);
  Serial.print("): ");
  
  if (length > 16) {
    length = 16;
    tail = true;
  }
  // send out the buffer
  for (uint16_t i = 0; i < length; i++)
  {
    Serial.print(data[i], HEX);
    Serial.print(" ");
  }
  if (tail) {
    Serial.print("...");
  }
  Serial.println();
}

void setup()
{
  // set-up serial for debug output
  Serial.begin(115200);
  
#if DMX_ENABLE
  // Initialize DMX output
  pinMode(DMX_DE_PIN, OUTPUT);
  digitalWrite(DMX_DE_PIN, HIGH);  // Enable MAX485 transmit
  Serial2.begin(250000, DMX_SERIAL_CONFIG, -1, DMX_TX_PIN);
  memset(dmxData, 0, sizeof(dmxData));
  
  Serial.println("DMX512 Output Enabled");
  Serial.print("TX Pin: ");
  Serial.println(DMX_TX_PIN);
  Serial.print("DE Pin: ");
  Serial.println(DMX_DE_PIN);
#else
  Serial.println("Debug Mode - DMX output disabled");
#endif
  
  ConnectWifi();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
  artnet.begin();
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
  
#if DMX_ENABLE
  // Continuously send DMX frames at regular intervals
  unsigned long now = millis();
  if (now - lastDmxSend >= DMX_REFRESH_MS) {
    sendDMX512();
  }
#endif
}
