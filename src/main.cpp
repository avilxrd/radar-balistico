#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "Config.h"
#include "Game.h"

WiFiClient espClient;
PubSubClient client(espClient);

char serialBomba[8] = "-----";
int tempoRestante     = 0;
bool desafioAtivo     = false;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Projectile projectile;

/* Funcoes definidas pelo Grupo 1 */
void callbackMqtt(char* topic, byte* payload, unsigned int length) {
    char msg[length + 1];
    memcpy(msg, payload, length);
    msg[length] = '\0';

    // 1. Estados globais da partida
    if (strcmp(topic, "jogo/estado") == 0) {
        if (strcmp(msg, "START") == 0) {
            desafioAtivo = true;
            // Iniciar leituras de sensores / habilitar botoes
        } 
        else if (strcmp(msg, "RESET") == 0) {
            desafioAtivo = false;
            // Resetar variaveis locais e voltar para tela de espera
        } 
        else if (strcmp(msg, "VICTORY") == 0 || strcmp(msg, "DEFEAT") == 0) {
            desafioAtivo = false;
            // Travar interacoes do jogador
        }
    }

    // 2. Serial da partida (recebe no START)
    else if (strcmp(topic, "jogo/serial") == 0) {
        strncpy(serialBomba, msg, sizeof(serialBomba));
        // DICA: Atualizar display do modulo com o serial:
        // tft.drawString(serialBomba, 10, 10, 2);
    }

    // 3. Sincronização opcional de tempo
    else if (strcmp(topic, "jogo/tempo") == 0) {
        tempoRestante = atoi(msg);
    }

    // 4. Pistas recebidas de outro módulo
    else if (strcmp(topic, "jogo/modulo/" MODULO_ID "/dica") == 0) {
        // Processar dado recebido (ex: msg contem "8310")
    }
}

void reconectarMQTT() {
    while (!client.connected()) {
        if (client.connect("Modulo_" MODULO_ID)) {
            // Inscricoes obrigatorias
            client.subscribe("jogo/estado");
            client.subscribe("jogo/serial");

            // Inscricoes opcionais (comente se nao for usar)
            client.subscribe("jogo/tempo");
            client.subscribe("jogo/modulo/" MODULO_ID "/dica");
        } else {
            delay(1000);
        }
    }
}

// Funcoes de notificacao para a Central
void registrarSucesso() {
    client.publish("jogo/modulo/" MODULO_ID "/evento", "SOLVED");
}

void registrarErro() {
    client.publish("jogo/modulo/" MODULO_ID "/evento", "ERROR");
}


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
    pinMode(JOY_B, INPUT_PULLUP);

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
    // Tempo desde o quadro anterior, em segundos
    static unsigned long previous_time = millis();
    unsigned long current_time = millis();

    float dt = (current_time - previous_time) / 1000.0f;
    previous_time = current_time;

    float x = normalizeJoystickAxes(analogRead(JOY_X), CENTER_X, true);
    float y = normalizeJoystickAxes(analogRead(JOY_Y), CENTER_Y);

    static bool button_was_pressed = false;
    bool button_pressed = (digitalRead(JOY_B) == LOW);

    if (button_pressed && !button_was_pressed && !projectile.isActive())
    {
        projectile.launch(
            {(float)CENTER_GAME_AREA_X, (float)CENTER_GAME_AREA_Y},
            x,
            y
        );
    }

    button_was_pressed = button_pressed;

    projectile.update(dt);

    Serial.printf("X: %.2f    Y: %.2f    SW: %d\r\n", x, y, button_pressed);

    display.clearDisplay();
    drawGameArea();
    drawRadar();
    drawRadarSweep();
    projectile.draw(display);
    display.display();

    delay(50);
}