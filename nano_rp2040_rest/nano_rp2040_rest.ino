#include "arduino_secrets.h"
#include <Arduino_LSM6DSOX.h>
#include <ArduinoJson.h>
#include <WiFiNINA.h>

char ssid[] = SECRET_SSID;     // your network SSID (name)
char pass[] = SECRET_PASSWORD; // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;              // your network key index number (needed only for WEP)
int port = SECRET_PORT;

int status = WL_IDLE_STATUS;
WiFiServer server(port);

void setup()
{
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
}

void loop()
{
  WiFiClient client = server.available(); // listen for incoming clients

  if (client) // if you get a client,
  {
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
            sendResponse(client, "Hello from Arduino RP2040! Valid endpoints are /Temperature/Current/F and /Temperature/Current/C", -99, "invalid");
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

        // If the current temperature was requested:
        if (request_name == "Fahrenheit" || request_name == "Celsius")
        {
          bool is_fahrenheit = (request_name == "Fahrenheit");
          int current_temperature = getTemperature(is_fahrenheit);
          String temp_units = is_fahrenheit ? "F" : "C";
          String message = "Current temperature is " + String(current_temperature) + "° " + temp_units;

          sendResponse(client, message, current_temperature, "success");
          break;
        }
      }
    }

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

/**
 * Build a response and send it back to the client.
 */
void sendResponse(WiFiClient &client, String message, int value, String status)
{
  // Send a standard HTTP response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type: application/json");
  client.println("Connection: close");
  client.println();

  // Create a JSON object
  StaticJsonDocument<200> doc;

  doc["message"] = message;
  doc["value"] = value;
  doc["status"] = status;

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
