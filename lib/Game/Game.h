#pragma once
#include <Adafruit_SSD1306.h>
#include <vector>

#include "Config.h"

/* Representa um par ordenado de coordenadas no plano */
struct Point
{
    float x;
    float y;
};

struct Line
{
    Point start;
    Point end;    
};

/* Representa um alvo inimigo/aliado. */
class Target
{    
public:
    Point position;
    float radius;
    bool enemy;
    bool active;

    // Construtores e destrutor
    Target(Point position, float radius, bool enemy, bool active);    
    // ~Target();

    // Métodos
    void draw(Adafruit_SSD1306& display) const;

    // Getters e Setters

};

class Projectile
{    
public:
    Point prev_position;
    Point position;
    Point velocity;
    float radius;
    bool active;
    
    // Construtores e destrutor
    Projectile();
    Projectile(Point prev_posivion, Point position, Point velocity, float radius, bool active);

    // Métodos
    void launch(Point origin, float direction_x, float direction_y);
    void update(float dt);
    void draw(Adafruit_SSD1306& display) const;

    // Getters e Setters
    bool isActive();

};

/* ============ FUNCOES ============ */

// bool insideCircle(Point pos, int center, int radius);

/* Gera uma lista de alvos aleatórios dentro de um circulo */
std::vector<Target> generateRandomTargets(Point center, int target_radius, int radar_radius, int quant);

/* Converte os valores brutos lidos do Joystick para valores normalizados
 * entre -1.0 e 1.0. */
float normalizeJoystickAxes(int raw_value, int center, bool invert=false, float deadzone=DEADZONE);

/* Desenha a borda retangular que delimita a área total
 * do jogo. */
void drawGameArea(Adafruit_SSD1306& display);

/* Desenha a estrutura do radar. */
void drawRadar(Adafruit_SSD1306& display);

/* Desenha o efeito de varredura que passa sobre o radar. */
Line drawRadarSweep(Adafruit_SSD1306& display);

/*  */
void drawAim(Adafruit_SSD1306& display, float joy_x, float joy_y);

/* Verifica se um segmento de reta cruza um alvo. */
bool hitsTarget(Target target, Point start_point, Point end_point);

/* Função genérica que verifica se um segmento de reta passa por um círculo. */
bool intercept(Line reta, Point center, float radius);

bool hits(Target target, Projectile projectile);
