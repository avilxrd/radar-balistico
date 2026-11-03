#include "Game.h"
#include "Config.h"

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

    display.fillCircle(
        (int)position.x,
        (int)position.y,
        (int)radius,
        SSD1306_WHITE
    );
}

/* ========== PROJECTILE ========== */

// Construtores
Projectile::Projectile()
{
    this->prev_position = {0, 0};
    this->position      = {0, 0};
    this->velocity      = {0, 0};
    this->radius        = TARGET_RADIUS;
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

    velocity.x =  direction_x * PROJECTILE_SPEED;
    velocity.y = -direction_y * PROJECTILE_SPEED;

    active = true;
}

void Projectile::update(float dt)
{
    if (!active) return;

    prev_position = position;

    position.x += velocity.x * dt;
    position.y += velocity.y * dt;

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
