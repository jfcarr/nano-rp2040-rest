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

#endif