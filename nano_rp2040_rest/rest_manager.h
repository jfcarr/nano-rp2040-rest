#ifndef REST_MANAGER_H
#define REST_MANAGER_H
#include <cstring>
#include <Arduino_LSM6DSOX.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <WiFiNINA.h>

enum class RGB_ACTION
{
    On,
    Off,
    NoChange
};

/**
 * Request pattern and corresponding request name.
 */
struct RequestMapping
{
    const char *pattern;
    const char *name;
};

class RestManager
{
private:
public:
    RestManager(const char *new_ssid, const char *new_password, int new_key_index, int new_port, int new_status);
    ~RestManager();

    char *ssid;     // network SSID (name)
    char *password; // network password (use for WPA, or use as key for WEP)
    int key_index;  // network key index number (needed only for WEP)
    int port;
    int status;
    WiFiServer *server;

    void set_led_pin_modes();
    void init_serial_communication(long speed);
    void check_for_imu();
    void check_for_wifi_module();
    void check_firmware();
    void control_rgb_led(RGB_ACTION red_action, RGB_ACTION green_action, RGB_ACTION blue_action);
    void connect_to_wifi();
    void print_wifi_status();
    void start_web_server();
    void listen_for_client();
    void send_response(StaticJsonDocument<200> doc, WiFiClient &client);
    int get_temperature(bool as_fahrenheit);
    int celsius_to_fahrenheit(int celsius_value);
};

RestManager::RestManager(const char *new_ssid, const char *new_password, int new_key_index, int new_port, int new_status)
{
    ssid = new char[strlen(new_ssid) + 1];
    strcpy(ssid, new_ssid);

    password = new char[strlen(new_password) + 1];
    strcpy(password, new_password);

    key_index = new_key_index;

    port = new_port;

    status = new_status;

    server = new WiFiServer(port);
}

RestManager::~RestManager()
{
    delete[] ssid;
    delete[] password;
}

/**
 * Prepare the onboard RGB LEDs for output.
 */
void RestManager::set_led_pin_modes()
{
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
    pinMode(LEDB, OUTPUT);
}

/**
 * Initialize serial communication at the specified speed (baud)
 */
void RestManager::init_serial_communication(long speed)
{
    Serial.begin(speed);
}

/**
 * See if the IMU is available.  If it is not, display a message and block.
 */
void RestManager::check_for_imu()
{
    if (!IMU.begin())
    {
        Serial.println("Failed to initialize IMU!");

        while (1)
            ;
    }
}

/**
 * See if the WiFi module is available.  If it is not, display a message and block.
 */
void RestManager::check_for_wifi_module()
{
    if (WiFi.status() == WL_NO_MODULE)
    {
        Serial.println("Communication with WiFi module failed!");

        while (true)
            ;
    }
}

/**
 * See if the firmware is up to date.  If not, display a message, but continue.
 */
void RestManager::check_firmware()
{
    String fv = WiFi.firmwareVersion();
    if (fv < WIFI_FIRMWARE_LATEST_VERSION)
    {
        Serial.println("Please upgrade the firmware");
    }
}

/**
 * Control the on/off state of the red, green, and blue onboard LEDs.
 */
void RestManager::control_rgb_led(RGB_ACTION red_action, RGB_ACTION green_action, RGB_ACTION blue_action)
{
    if (red_action == RGB_ACTION::On)
        digitalWrite(LEDR, HIGH);
    if (red_action == RGB_ACTION::Off)
        digitalWrite(LEDR, LOW);

    if (green_action == RGB_ACTION::On)
        digitalWrite(LEDG, HIGH);
    if (green_action == RGB_ACTION::Off)
        digitalWrite(LEDG, LOW);

    if (blue_action == RGB_ACTION::On)
        digitalWrite(LEDB, HIGH);
    if (blue_action == RGB_ACTION::Off)
        digitalWrite(LEDB, LOW);
}

/**
 * Attempt connection to WiFi network, wait for connection, block on connection failure.
 */
void RestManager::connect_to_wifi()
{
    // Not connected yet: green LED off, red LED on:
    control_rgb_led(RGB_ACTION::On, RGB_ACTION::Off, RGB_ACTION::NoChange);

    bool is_connected = false;
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid); // print the network name (SSID)

    // Wait up to 8 seconds for a connection:
    status = WiFi.begin(ssid, password);
    for (int try_connect = 1; try_connect <= 8; try_connect++)
    {
        delay(1000);
        if (status == WL_CONNECTED)
        {
            is_connected = true;
            break;
        }
    }

    // If still not connected, blink the red LED to indicate connection failure:
    if (status != WL_CONNECTED)
    {
        while (true)
        {
            control_rgb_led(RGB_ACTION::On, RGB_ACTION::NoChange, RGB_ACTION::NoChange);
            delay(500);
            control_rgb_led(RGB_ACTION::Off, RGB_ACTION::NoChange, RGB_ACTION::NoChange);
            delay(500);
        }
    }

    // Connected: red LED off, green LED on:
    control_rgb_led(RGB_ACTION::Off, RGB_ACTION::On, RGB_ACTION::NoChange);
}

/**
 * Display status information about the WiFi connection.
 */
void RestManager::print_wifi_status()
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
 * Start the web server, making it available for the client process.
 */
void RestManager::start_web_server()
{
    server->begin();
}

/**
 * Initialize the client, wait for requests, and handle them.
 */
void RestManager::listen_for_client()
{
    // Initialize an array of request mappings.
    RequestMapping mappings[] = {
        {"GET /Temperature/Current/F", "Fahrenheit"},
        {"GET /Temperature/Current/C", "Celsius"},
        {"GET /Connection/Info", "ConnectionInfo"}};

    WiFiClient client = server->available(); // listen for incoming clients

    if (client) // if you get a client,
    {
        // Client connected: blue LED on:
        control_rgb_led(RGB_ACTION::NoChange, RGB_ACTION::NoChange, RGB_ACTION::On);

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

                        send_response(doc, client);
                        break;
                    }
                    else
                        currentLine = ""; // if you got a newline, then clear currentLine
                }
                else if (c != '\r')
                    // if you got anything else but a carriage return character, add it to the end of the currentLine:
                    currentLine += c;

                const int numMappings = sizeof(mappings) / sizeof(mappings[0]);

                // Default request name in case no pattern matches:
                String request_name = "Default";

                // See if a known endpoint was sent:
                for (int i = 0; i < numMappings; i++)
                {
                    if (currentLine.indexOf(mappings[i].pattern) != -1)
                    {
                        request_name = mappings[i].name;
                        break;
                    }
                }

                // If the current temperature was requested:
                if (request_name == "Fahrenheit" || request_name == "Celsius")
                {
                    bool is_fahrenheit = (request_name == "Fahrenheit");
                    int current_temperature = get_temperature(is_fahrenheit);
                    String temp_units = is_fahrenheit ? "F" : "C";

                    StaticJsonDocument<200> doc;
                    doc["message"] = "Current temperature is " + String(current_temperature) + "° " + temp_units;
                    doc["value"] = current_temperature;
                    doc["status"] = "success";

                    send_response(doc, client);
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
                    send_response(doc, client);
                    break;
                }
            }
        }

        delay(500);
        // Client disconnected: blue LED off:
        control_rgb_led(RGB_ACTION::NoChange, RGB_ACTION::NoChange, RGB_ACTION::Off);

        // close the connection:
        client.stop();
        Serial.println("client disconnected");
    }
}

/**
 * Build a response and send it back to the client.
 */
void RestManager::send_response(StaticJsonDocument<200> doc, WiFiClient &client)
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
 * Get the current temperature from the IMU.
 */
int RestManager::get_temperature(bool as_fahrenheit)
{
    if (IMU.temperatureAvailable())
    {
        int temperature_deg = 0;
        IMU.readTemperature(temperature_deg);

        if (as_fahrenheit == true)
            temperature_deg = celsius_to_fahrenheit(temperature_deg);

        return temperature_deg;
    }
    else
    {
        return -99;
    }
}

/**
 * Convert given celsius value to fahrenheit.
 */
int RestManager::celsius_to_fahrenheit(int celsius_value)
{
    return (celsius_value * (9 / 5)) + 32;
}
#endif