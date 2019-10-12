#include <SPI.h>
#include <WiFiNINA.h>

#include "error.h"
#include "wifi_display.h"
#include "wifi_secrets.h"

// Configure the pins used for the ESP32 connection
#define SPIWIFI SPI     // The SPI port
#define SPIWIFI_SS 13   // Chip select pin
#define ESP32_RESETN 12 // Reset pin
#define SPIWIFI_ACK 11  // a.k.a BUSY or READY pin
#define ESP32_GPIO0 -1

char ssid[] = SECRET_SSID; // your network SSID
char pass[] = SECRET_PASS; // your network password
// IPAddress serverIp(10, 0, 0, 101);
IPAddress serverIp(10, 0, 0, 4);
uint16_t serverPort = 2390;

extern Adafruit_Arcada arcada;
int status = WL_IDLE_STATUS;

uint16_t *frameBuffer;
size_t frameIdx;
constexpr size_t FRAME_BUFFER_BYTES = ARCADA_TFT_WIDTH * ARCADA_TFT_HEIGHT * sizeof(frameBuffer[0]);

char bufferResponse[] = "ok";

WiFiClient client;

void printWifiStatus()
{
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void flush()
{
  static uint8_t bufferDrain[100];
  while (client.available())
    client.read(bufferDrain, min(100, client.available()));
}

/** Request a new frame from the server. */
void requestFrame()
{
  delay(100); // Frame limit at 10 fps.
  flush();
  frameIdx = 0;
  client.print(bufferResponse);
}

void setColor(int r, int g, int b)
{
  for (size_t i = 0; i < arcada.pixels.numPixels(); i++)
    arcada.pixels.setPixelColor(i, arcada.pixels.Color(r, g, b));
  arcada.pixels.show();
}

void WifiDisplay::setup()
{
  WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);

  // Check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
    error("Communication with WiFi module failed!");

  if (WiFi.firmwareVersion() < "1.0.0")
    error("Please upgrade the firmware");

  // Attempt to connect to Wifi network:
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(ssid);

  delay(1000);
  WiFi.disconnect();
  delay(100);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED)
    Serial.println("Retrying...");
  WiFi.noLowPowerMode();
  printWifiStatus();

  if (!arcada.createFrameBuffer(ARCADA_TFT_WIDTH, ARCADA_TFT_HEIGHT))
    error("Failed to allocate framebuffer");
  frameBuffer = arcada.getFrameBuffer();
  bzero(frameBuffer, FRAME_BUFFER_BYTES);
}

void WifiDisplay::loop()
{
  setColor(255, 0, 0);

  // Check if we are still connected.
  if (!client.connected())
  {
    Serial.print("Connecting to server: ");
    Serial.print(serverIp);
    Serial.print(":");
    Serial.print(serverPort);
    Serial.print(" ... ");
    if (!client.connect(serverIp, serverPort))
    {
      Serial.println("failed.");
      return;
    }
    Serial.println("done.");
    requestFrame();
  }

  setColor(0, 0, 0);

  // Manually send a frame request.
  if (arcada.readButtons() & ARCADA_BUTTONMASK_B)
  {
    setColor(0, 255, 0);
    while (arcada.readButtons() & ARCADA_BUTTONMASK_B)
      delay(100);
    Serial.println("Request (manual).");
    requestFrame();
    return;
  }

  // Read the packet into buffer.
  int numBytes = 0;
  do
  {
    setColor(0, 0, 255);
    numBytes = client.read(reinterpret_cast<uint8_t *>(frameBuffer) + frameIdx, FRAME_BUFFER_BYTES - frameIdx);
    setColor(255, 128, 0);
    if (numBytes > 0)
      frameIdx += numBytes;
  } while (numBytes > 0);

  setColor(255, 0, 255);

  /**
  // Print number of bytes.
  Serial.print("Got ");
  Serial.print(frameIdx);
  Serial.print(" (+");
  Serial.print(numBytes);
  Serial.println(") bytes.");
  */

  // Ignore a partial buffer.
  if (frameIdx < FRAME_BUFFER_BYTES)
    return;
  Serial.println("Frame.");

  setColor(255, 255, 0);

  // Blit this content to the display buffer.
  arcada.blitFrameBuffer(0, 0, true, false);
  frameIdx = 0;

  // Request more data.
  requestFrame();

  setColor(255, 255, 255);
}
