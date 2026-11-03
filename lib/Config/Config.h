#pragma once

// MQTT & WiFi
#define MODULO_ID "5"

constexpr char* WIFI_SSID     = "iot2026";
constexpr char* WIFI_PASSWORD = "Senhab0@";
constexpr char* MQTT_BROKER   = "10.150.207.154";
constexpr int   MQTT_PORT     = 1883;

// Display
#define OLED_ADDR 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define MAX_GAME_AREA SCREEN_WIDTH*3/4
#define CENTER_GAME_AREA_X (MAX_GAME_AREA/2)
#define CENTER_GAME_AREA_Y (SCREEN_HEIGHT/2)

// Radar
#define RADAR_RADIUS 29
#define CENTER_RADIUS 1

// Joystick
#define JOY_Y 33
#define JOY_X 32
#define JOY_B 25

#define DEADZONE 100
#define CENTER_X 2048
#define CENTER_Y 2048

// Alvo
#define TARGET_RADIUS 1.0f

// Projétil
#define PROJECTILE_SPEED 20.0f