#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>

// Constantes Globais Úteis
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
#define TARGET_RADIUS 3 

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

struct Point
{
  float x;
  float y;
};

class Target
{
    Point position;
    float radius;
    bool enemy;
    bool active;
};

class Projectile
{
  Point previous_position;
  Point position;
  float radius;
  float speed;
  bool active;
};


/* Desenha a borda retangular que delimita a área total
 * do jogo. */
void drawGameArea() 
{
  // drawRect(X_INICIAL, Y_INICIAL, LARGURA, ALTURA)
  display.drawRect(0, 0, MAX_GAME_AREA, SCREEN_HEIGHT, SSD1306_WHITE);
  display.drawRect(3 + MAX_GAME_AREA, 0, (SCREEN_WIDTH-MAX_GAME_AREA-3), SCREEN_HEIGHT, SSD1306_WHITE);
}


/* Desenha a estrutura do radar. */
void drawRadar()
{
  display.drawCircle(CENTER_GAME_AREA_X, CENTER_GAME_AREA_Y, RADAR_RADIUS, SSD1306_WHITE);
  // display.fillCircle(CENTER_GAME_AREA_X, CENTER_GAME_AREA_Y, CENTER_RADIUS, SSD1306_WHITE);
}


/* Desenha o efeito de varredura que passa sobre o radar. */
float radar_angle = 0.0f;
void drawRadarSweep()
{
  int end_x = CENTER_GAME_AREA_X + cos(radar_angle) * RADAR_RADIUS;
  int end_y = CENTER_GAME_AREA_Y + sin(radar_angle) * RADAR_RADIUS;

  display.drawLine(
    CENTER_GAME_AREA_X,
    CENTER_GAME_AREA_Y,
    end_x,
    end_y,
    SSD1306_WHITE
  );

  radar_angle += 0.08f;
  if (radar_angle >= 2.0f * PI)
  {
    radar_angle -= 2.0f * PI;
  }
}


/* Converte os valores brutos lidos do Joystick para valores normalizados
 * entre -1.0 e 1.0. */
float normalizeJoystickAxes(int raw_value, int center, bool invert=false, float deadzone=DEADZONE)
{
  int delta = raw_value - center;

  // Ignora movimentos dentro da deadzone configurada
  if (abs(delta) < deadzone) 
  {
    return 0.0f;
  }

  float value;
  if (delta > 0)
  {
    value = (float)delta/(4095 - center);
  }
  else 
  {
    value = (float)delta/center;
  }

  value = constrain(value, -1.0f, 1.0f);

  // Inverte os valores do eixo
  if (invert) { value = -value; }

  return value;
}


void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22); // SDA, SCL

  pinMode(JOY_X, INPUT);
  pinMode(JOY_Y, INPUT);
  pinMode(JOY_B, INPUT_PULLDOWN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) 
  {
    Serial.println("Nao foi possivel iniciar o OLED.");
    while (true) delay(100);
  }

  randomSeed(esp_random());
  display.clearDisplay();
  display.display();
}


void loop() 
{
  // float x = analogRead(JOY_X);
  // float y = analogRead(JOY_Y);
  float x = normalizeJoystickAxes(analogRead(JOY_X), CENTER_X, true);
  float y = normalizeJoystickAxes(analogRead(JOY_Y), CENTER_Y);
  float b = analogRead(JOY_B);

  Serial.printf("X: %.2f  Y: %.2f  B: %.2f\r\n", x, y, b);

  display.clearDisplay();
  drawGameArea();
  drawRadar();
  drawRadarSweep();
  display.display();

  delay(50);
}