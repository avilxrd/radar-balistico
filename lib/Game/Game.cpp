#include "Game.h"
#include "Config.h"
#include <random>
#include <vector>

/* ============ TARGET ============ */

// Construtores
Target::Target(Point position, float radius, bool enemy, bool active)
{
    this->position = position;
    this->radius   = radius;
    this->enemy    = enemy;
    this->active   = active;
}

// Métodos
void Target::draw(Adafruit_SSD1306& display) const
{
    if (!active) return;

    if (this->active)
    {
        display.fillCircle(
            (int)position.x,
            (int)position.y,
            (int)radius,
            SSD1306_WHITE
        );
    }
}

/* ========== PROJECTILE ========== */

// Construtores
Projectile::Projectile()
{
    this->prev_position = {0, 0};
    this->position      = {0, 0};
    this->velocity      = {0, 0};
    this->radius        = PROJECTILE_RADIUS;
    this->active        = false;
}

Projectile::Projectile(Point prev_position, Point position, Point velocity, float radius, bool active)
{
    this->prev_position = prev_position;
    this->position      = position;
    this->velocity      = velocity;
    this->radius        = radius;
    this->active        = active;
}

// Métodos
void Projectile::launch(Point origin, float direction_x, float direction_y)
{
    float lenght = sqrt(direction_x*direction_x + direction_y*direction_y);
    
    if (lenght < 0.01f) return; // Deadzone

    direction_x /= lenght;
    direction_y /= lenght;

    position      = origin;
    prev_position = origin;

    velocity.x =  (direction_x * PROJECTILE_SPEED);
    velocity.y =  (-direction_y * PROJECTILE_SPEED);

    active = true;
}

void Projectile::update(float dt)
{
    if (!active) return;

    prev_position = position;

    position.x += (velocity.x * dt) + WIND_X;
    position.y += (velocity.y * dt) + WIND_Y;

    if (position.x < -radius ||
        position.x > SCREEN_WIDTH + radius || 
        position.y < -radius ||
        position.y > SCREEN_HEIGHT + radius)
    {
        active = false;
    }
}

void Projectile::draw(Adafruit_SSD1306& display) const
{
    if (!active) return;

    display.fillCircle(
        (int)position.x,
        (int)position.y,
        (int)radius,
        SSD1306_WHITE
    );
}

// Getters e Setters
bool Projectile::isActive()
{
    if (this->active)
    {
        return true;
    }

    return false;
}


/* ============ FUNCOES ============ */

std::vector<Target> generateRandomTargets(Point center, int target_radius, int radar_radius, int quant)
{
    std::vector<Target> targets;
    
    for (int i=0; i<quant; i++)
    {
        float theta = static_cast<float>(rand()) / RAND_MAX * (2.0f * M_PI);
        float r     = (radar_radius-target_radius) * std::sqrt(static_cast<float>(rand()) / RAND_MAX);

        Point pos;
        pos.x = center.x + cos(theta) * r;
        pos.y = center.y + sin(theta) * r;

        Target target(pos, target_radius, true, true);

        targets.push_back(target);
    }
    return targets;
}

float normalizeJoystickAxes(int raw_value, int center, bool invert, float deadzone)
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

void drawGameArea(Adafruit_SSD1306& display) 
{
    // drawRect(X_INICIAL, Y_INICIAL, LARGURA, ALTURA)
    display.drawRect(0, 0, MAX_GAME_AREA, SCREEN_HEIGHT, SSD1306_WHITE);
    display.drawRect(3 + MAX_GAME_AREA, 0, (SCREEN_WIDTH-MAX_GAME_AREA-3), SCREEN_HEIGHT, SSD1306_WHITE);
}

void drawRadar(Adafruit_SSD1306& display)
{
    display.drawCircle(CENTER_GAME_AREA_X, CENTER_GAME_AREA_Y, RADAR_RADIUS, SSD1306_WHITE);
    display.drawCircle(CENTER_GAME_AREA_X, CENTER_GAME_AREA_Y, CENTER_RADIUS, SSD1306_WHITE);
}

float radar_angle = 0.0f;
Line drawRadarSweep(Adafruit_SSD1306& display)
{
    int end_x = CENTER_GAME_AREA_X + cos(radar_angle) * RADAR_RADIUS;
    int end_y = CENTER_GAME_AREA_Y + sin(radar_angle) * RADAR_RADIUS;

    Point start = {CENTER_GAME_AREA_X, CENTER_GAME_AREA_Y};
    Point end   = {end_x, end_y};

    display.drawLine(
        start.x, start.y,
        end.x, end.y,
        SSD1306_WHITE
    );

    radar_angle += 0.08f;
    if (radar_angle >= 2.0f * PI)
    {
        radar_angle -= 2.0f * PI;
    }

    Line line = {start, end};
    
    return line;
}

float calcularDistancia(Point p1, Point p2) {
    float dx = p1.x - p2.x;
    float dy = p1.y - p2.y;
    return std::sqrt(dx * dx + dy * dy);
}

// Função auxiliar para limitar um valor entre um mínimo e um máximo
float clamp(float val, float minVal, float maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// Retorna o ponto do segmento AB mais próximo do centro do círculo
Point encontrarPontoMaisProximoNoSegmento(Point A, Point B, Point centroCirculo) {
    float ab_x = B.x - A.x;
    float ab_y = B.y - A.y;
    float ac_x = centroCirculo.x - A.x;
    float ac_y = centroCirculo.y - A.y;

    float ab_len_sq = ab_x * ab_x + ab_y * ab_y;

    // Se A e B forem o mesmo ponto, o mais próximo é o próprio ponto A
    if (ab_len_sq == 0.0f) {
        return A;
    }

    // Projeção escalar (t) do centro na linha AB
    float t = (ac_x * ab_x + ac_y * ab_y) / ab_len_sq;

    // LIMITA (clamp) o 't' estritamente entre 0.0 e 1.0 para ficar dentro do segmento
    t = clamp(t, 0.0f, 1.0f);

    // Calcula o ponto final usando o t limitado
    Point pontoMaisProximo;
    pontoMaisProximo.x = A.x + t * ab_x;
    pontoMaisProximo.y = A.y + t * ab_y;

    return pontoMaisProximo;
}

bool hitsTarget(Target target, Point start_point, Point end_point)
{
    Point close = encontrarPontoMaisProximoNoSegmento(start_point, end_point, target.position);

    // Acertou
    if (calcularDistancia(close, target.position) <= target.radius)
    {
        return true;
    }

    return false;
}