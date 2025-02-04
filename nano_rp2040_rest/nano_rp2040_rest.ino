#include "arduino_secrets.h"
#include <Arduino_LSM6DSOX.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WiFiNINA.h>

char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASSWORD; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;              // your network key index number (needed only for WEP)
int port = SECRET_PORT;

int status = WL_IDLE_STATUS;
WiFiServer server(port);

void setup()
{
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  Serial.begin(9600); // initialize serial communication at 9600 baud

  // Check for the IMU:
  if (!IMU.begin())
  {
    Serial.println("Failed to initialize IMU!");

    while (1)
      ;
  }

  // Check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println("Communication with WiFi module failed!");

    while (true)
      ;
  }

  // Check the firmware:
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println("Please upgrade the firmware");
  }

  // Not connected: green LED off, red LED on:
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDR, HIGH);

  // Connect to WiFi network:
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid); // print the network name (SSID)

    status = WiFi.begin(ssid, pass);

    delay(5000); // wait 5 seconds for connection
  }

  server.begin();    // start the web server
  printWifiStatus(); // print out the status

  // Connected: red LED off, green LED on:
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, HIGH);
}

void loop()
{
  WiFiClient client = server.available(); // listen for incoming clients

  if (client) // if you get a client,
  {
    // Client connected: blue LED on:
    digitalWrite(LEDB, HIGH);

    Serial.println("new client"); // print a message out the serial port

    String currentLine = ""; // make a String to hold incoming data from the client

    while (client.connected()) // loop while the client's connected
    {
      if (client.available()) // if there's bytes to read from the client,
      {
        char c = client.read(); // read a byte, then
        Serial.write(c);        // print it out on the serial monitor

        if (c == '\n') // if the byte is a newline character
        {
          if (currentLine.length() == 0)
          {
            StaticJsonDocument<200> doc;
            doc["message"] = "Hello from Arduino RP2040! Valid endpoints are /Temperature/Current/F, /Temperature/Current/C, and /Connection/Info";
            doc["value"] = -99;
            doc["status"] = "invalid";

            sendResponse(client, doc);
            break;
          }
          else
            currentLine = ""; // if you got a newline, then clear currentLine
        }
        else if (c != '\r')
          // if you got anything else but a carriage return character, add it to the end of the currentLine:
          currentLine += c;

        // See if a known endpoint was sent:
        String request_name = "Default";
        if (currentLine.indexOf("GET /Temperature/Current/F") != -1)
          request_name = "Fahrenheit";
        if (currentLine.indexOf("GET /Temperature/Current/C") != -1)
          request_name = "Celsius";
        if (currentLine.indexOf("GET /Connection/Info") != -1)
          request_name = "ConnectionInfo";

        // If the current temperature was requested:
        if (request_name == "Fahrenheit" || request_name == "Celsius")
        {
          bool is_fahrenheit = (request_name == "Fahrenheit");
          int current_temperature = getTemperature(is_fahrenheit);
          String temp_units = is_fahrenheit ? "F" : "C";

          StaticJsonDocument<200> doc;
          doc["message"] = "Current temperature is " + String(current_temperature) + "° " + temp_units;
          doc["value"] = current_temperature;
          doc["status"] = "success";

          sendResponse(client, doc);
          break;
        }

        if (request_name == "ConnectionInfo")
        {
          String ssid = WiFi.SSID();
          IPAddress ip = WiFi.localIP();
          long rssi = WiFi.RSSI();

          String signalStrength = "Unknown";
          if (rssi > -50)
            signalStrength = "Excellent";
          else if (rssi <= -50 && rssi >= -60)
            signalStrength = "Good";
          else if (rssi < -60 && rssi >= -70)
            signalStrength = "Fair";
          else if (rssi < -70)
            signalStrength = "Weak";

          StaticJsonDocument<200> doc;
          doc["ssid"] = ssid;
          doc["ipAddress"] = ip;
          doc["rssi"] = rssi;
          doc["signalStrength"] = signalStrength;
          sendResponse(client, doc);
          break;
        }
      }
    }

    delay(500);
    // Client disconnected: blue LED off:
    digitalWrite(LEDB, LOW);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

/**
 * Build a response and send it back to the client.
 */
void sendResponse(WiFiClient &client, StaticJsonDocument<200> doc)
{
  // Send a standard HTTP response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type: application/json");
  client.println("Connection: close");
  client.println();

  // Serialize JSON to client
  serializeJson(doc, client);
}

/**
 * Display status information about the WiFi connection.
 */
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
  Serial.print("signal strength (RSSI): ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

/**
 * Get the current temperature from the IMU.
 */
int getTemperature(bool as_fahrenheit)
{
  if (IMU.temperatureAvailable())
  {
    int temperature_deg = 0;
    IMU.readTemperature(temperature_deg);

    if (as_fahrenheit == true)
      temperature_deg = celsiusToFahrenheit(temperature_deg);

    return temperature_deg;
  }
  else
  {
    return -99;
  }
}

/**
 * Given a celsius value, return the value converted to fahrenheit.
 */
int celsiusToFahrenheit(int celsius_value)
{
  return (celsius_value * (9 / 5)) + 32;
}
