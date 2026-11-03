#pragma once
#include <Adafruit_SSD1306.h>

/* Representa um par ordenado de coordenadas no plano */
struct Point
{
    float x;
    float y;
};

/* Representa um alvo inimigo/aliado. */
class Target
{
private:
    Point position;
    float radius;
    bool enemy;
    bool active;

public:
    // Construtores e destrutor
    Target(Point position, float radius, bool enemy, bool active);    
    ~Target();

    // Métodos
    void draw(Adafruit_SSD1306& display) const;

    // Getters e Setters

};

class Projectile
{
private:
    Point prev_position;
    Point position;
    Point velocity;
    float radius;
    bool active;

public:
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