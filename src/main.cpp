#include <iostream>
#include <vector>
#include <TFT_eSPI.h>
#include "more.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

void drawFrame(const uint16_t* frameData, int xOffset);
void drawSnow(const uint16_t* frameData);
uint32_t convertColor(float r, float g, float b);
void updateSnowflakes();
void drawSnowflakes();
void cleanBunny();

TFT_eSPI tft = TFT_eSPI();  // Create TFT_eSPI object

#define BUNNY_WIDTH 55 
#define BUNNY_HEIGHT 74 
#define BUTTON_PIN1 35
#define BUTTON_PIN2 0

#define SNOW_WIDTH  240
#define SNOW_HEIGHT 19

struct Snowflake {
  int x;    
  int y;    
  float speed; 
};

const int numSnowflakes = 20;
const float minSpeed = 1;
const float maxSpeed = 16;
const int maxSnowflakes = 60; 
std::vector<Snowflake> snowflakes;
uint16_t buffer[BUNNY_WIDTH * BUNNY_HEIGHT];
uint16_t bufferSnow[SNOW_WIDTH * SNOW_HEIGHT];

unsigned long lastFrameTime = 0;
unsigned long lastFrameSnow = 0;

bool butPressed = false;
int colorIndex = 0;

int currentFrame = 0;
StateMachine bunny;
std::vector<int> currentSeq;
int offsetX = 0;

void setup() {
  Serial.begin(115200);
  setCpuFrequencyMhz(240);
  pinMode(BUTTON_PIN1, INPUT_PULLUP);  
  pinMode(BUTTON_PIN2, INPUT_PULLUP);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(background[colorIndex]);
  tft.setSwapBytes(true);

  currentSeq = bunny.getRandomSeq();

  // Create snowflakes at random positions on the top row
  for (int i = 0; i < numSnowflakes; ++i) {
    Snowflake snowflake;
    snowflake.x = random(tft.width());
    snowflake.y = 0;
    snowflake.speed = random(minSpeed, maxSpeed);
    snowflakes.push_back(snowflake);
  }
}

void loop() {
  for (int i = 0; i < currentSeq.size(); i++) {
    int numOfSprites = bunny.getNumOfSprites(currentSeq[i]);

    while (numOfSprites > 0) {
      unsigned long currentTime = millis();
      if (currentTime - lastFrameSnow >= 400) {
          updateSnowflakes();
          drawSnowflakes();

          lastFrameSnow = currentTime;
      }

      if (digitalRead(BUTTON_PIN1) == LOW && bunny._behaviour.state == Behaviour::States::sleepy) {
        currentSeq = bunny.getRandomSeq();
        currentFrame = 0;
        i = 0;
        break;
      }

      if (digitalRead(BUTTON_PIN2) == LOW && !butPressed) {
        colorIndex = (colorIndex + 1) % background.size();
        tft.fillScreen(background[colorIndex]);
        butPressed = true;
      }
      if (digitalRead(BUTTON_PIN2) == HIGH)
        butPressed = false;

      bunny._behaviour.state = currentSeq[i];
      if (currentTime - lastFrameTime >= frameDuration[bunny._behaviour.state]) {
        bunny.elapsedTime = (esp_timer_get_time() / 1000000.0) - bunny.prevTime;
      
      if (tft.width() - offsetX - BUNNY_WIDTH < 0 || tft.width() - offsetX - BUNNY_WIDTH > 195) {
        bunny._behaviour.state = Behaviour::States::jump;
      }      

        switch (bunny._behaviour.state) {
          case Behaviour::States::sit: {
            cleanBunny();
            if (bunny._behaviour.prevState == Behaviour::States::sleepy) {
              bunny._behaviour.frames = 9;
              drawFrame(sit[currentFrame], offsetX);
              bunny._behaviour.prevState = Behaviour::States::sit;
            }
            else {
              bunny._behaviour.frames = 2;
              drawFrame(sit[currentFrame + 7], offsetX);
            }
          } break;
          case Behaviour::States::sleepy: {
            cleanBunny();
            if (bunny._behaviour.prevState == Behaviour::States::sit) {
              bunny._behaviour.frames = 10;
              drawFrame(sleepy[currentFrame], offsetX);
              bunny._behaviour.prevState = Behaviour::States::sleepy;
            }
            else {
              bunny._behaviour.frames = 4;
              drawFrame(sleepy[currentFrame + 6], offsetX);
            }
          } break;
          case Behaviour::States::stand: {
            cleanBunny();
            bunny._behaviour.frames = 10;
            drawFrame(stand[currentFrame], offsetX);
          } break;
          case Behaviour::States::jump: {
            cleanBunny();
            bunny._behaviour.frames = 6;
            if (tft.width() - offsetX - BUNNY_WIDTH < -55) {
              offsetX = -55;
            } else {
              cleanBunny();
              if (currentFrame % bunny._behaviour.frames != 5)
                offsetX += 10;
            }
            drawFrame(jump[currentFrame], offsetX);
          } break;
          default:
            break;
        }

        lastFrameTime = currentTime;
        currentFrame = (currentFrame + 1) % bunny._behaviour.frames;
        bunny._behaviour.currFrame++;
        // bunny._behaviour.print();
        numOfSprites--;

        if (bunny.elapsedTime <= timers[bunny._behaviour.state][random(0, 2)] && numOfSprites == 0) {
          numOfSprites = bunny.getNumOfSprites(currentSeq[i]);
        }
      }

      auto scan = tft.read_gscan();
      // if (scan != 0)
        std::cout << scan << "\n";

      drawSnow(snow);
    } // while

    bunny.prevTime = (esp_timer_get_time() / 1000000.0);
    currentFrame = 0;
    bunny._behaviour.currFrame = 0;
  }
  currentSeq = bunny.getRandomSeq();
  bunny.prevTime = (esp_timer_get_time() / 1000000.0);
  currentFrame = 0;
  bunny._behaviour.currFrame = 0;

  // Pause to control the snowfall speed
  delay(100);
}

void drawFrame(const uint16_t* frameData, int offsetX) {
  memset(buffer, 0, BUNNY_WIDTH * BUNNY_HEIGHT * sizeof(uint16_t));
  memcpy(buffer, frameData, BUNNY_WIDTH * BUNNY_HEIGHT * sizeof(uint16_t));
  tft.pushImage((tft.width() - offsetX - BUNNY_WIDTH), (tft.height() - BUNNY_HEIGHT - 5), BUNNY_WIDTH, BUNNY_HEIGHT, buffer, 0x0000);
}

void drawSnow(const uint16_t* frameData) {
  memset(bufferSnow, 0, SNOW_WIDTH * SNOW_HEIGHT * sizeof(uint16_t));
  memcpy(bufferSnow, frameData, SNOW_WIDTH * SNOW_HEIGHT * sizeof(uint16_t));
  tft.pushImage((tft.width() - SNOW_WIDTH + 11), (tft.height() - SNOW_HEIGHT + 7), SNOW_WIDTH, SNOW_HEIGHT, bufferSnow, 0x0000);
}

void updateSnowflakes() {
  for (auto& snowflake : snowflakes) {
    tft.drawPixel(snowflake.x, snowflake.y, background[colorIndex]);
    // Update snowflake positions
    snowflake.y += snowflake.speed;
    snowflake.x += random(-5, 6);

    // Do not wrap around, just reset the position when reaching the bottom
    if (snowflake.y >= tft.height()) {
      snowflake.y = 0;
      snowflake.x = random(tft.width());
    }
  }

  // Introduce new snowflakes at the top if not reached the maximum limit
  if (snowflakes.size() < maxSnowflakes) {
    for (int i = 0; i < numSnowflakes; ++i) {
      Snowflake snowflake;
      snowflake.x = random(tft.width());
      snowflake.y = 0;
      snowflake.speed = random(minSpeed, maxSpeed);
      snowflakes.push_back(snowflake);
    }
  }
}

void cleanBunny() {
  tft.fillRect(tft.width() - offsetX - BUNNY_WIDTH, tft.height() - BUNNY_HEIGHT - 11, BUNNY_WIDTH, BUNNY_HEIGHT + 5, background[colorIndex]);
}

void drawSnowflakes() {
  for (const auto& snowflake : snowflakes) {
    // Draw snowflake at updated position
    if (tft.readPixel(snowflake.x, snowflake.y) == background[colorIndex])
      tft.drawPixel(snowflake.x, snowflake.y, convertColor(255,255,255), (background[colorIndex] == TFT_BLACK)?random(32,160):255, background[colorIndex]);
  }
}

uint32_t convertColor(float r, float g, float b) {
    uint32_t R = static_cast<uint32_t>(r) << 16;
    uint32_t G = static_cast<uint32_t>(g) << 8;
    uint32_t B = static_cast<uint32_t>(b);

    return 0xFF000000 | R | G | B;
}
