#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_system.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>

#include "Config.h"
#include "Game.h"

/* Variáveis Globais */
char serialBomba[8] = "-----";
int  tempoRestante  = 0;
bool desafioAtivo   = false;

WiFiClient espClient;
PubSubClient client(espClient);

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

std::vector<Target> targets;

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

    targets = generateRandomTargets({CENTER_GAME_AREA_X, CENTER_GAME_AREA_Y}, TARGET_RADIUS, RADAR_RADIUS, 5);

    display.clearDisplay();
    display.display();
}


Line radar_line;
void loop()
{
    // Tempo desde o quadro anterior em segundos
    static unsigned long previous_time = millis();
    unsigned long        current_time  = millis();

    float dt = (current_time - previous_time) / 1000.0f;
    previous_time = current_time;

    // Posição dos eixos do joystick
    float x = normalizeJoystickAxes(analogRead(JOY_X), CENTER_X, true);
    float y = normalizeJoystickAxes(analogRead(JOY_Y), CENTER_Y);


    static bool button_was_pressed = false;
    bool        button_pressed     = (digitalRead(JOY_B) == LOW);

    if (button_pressed && !button_was_pressed && !projectile.isActive())
    {
        projectile.launch( {(float)CENTER_GAME_AREA_X, (float)CENTER_GAME_AREA_Y}, x, y );
    }

    button_was_pressed = button_pressed;

    projectile.update(dt);
    
    // Serial.printf("X: %.2f    Y: %.2f    SW: %d\r\n", x, y, button_pressed);
    
    display.clearDisplay();
    drawGameArea(display);
    drawRadar(display);
    radar_line = drawRadarSweep(display);
    drawAim(display, x, y);
 
    for (int i=0; i<5; i++)
    {
        if (hitsTarget(targets[i], radar_line.start, radar_line.end))
        {
            targets[i].active = true; // Radar passando pelo alvo
        }
        else
        {
            targets[i].active = false; // Radar fora do alvo
        }

        targets[i].draw(display);
    }

    projectile.draw(display);
    
    display.display();

    // delay(50);
}