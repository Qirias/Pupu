#include <iostream>
#include <vector>
#include <TFT_eSPI.h>
#include "more.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void drawFrame(const uint16_t* frameData, int xOffset);
uint32_t convertColor(float r, float g, float b);
void drawSnow(void* parameter);

TFT_eSPI tft = TFT_eSPI();  // Create TFT_eSPI object
// TFT_eSprite spr = TFT_eSprite(&tft);  // Declare Sprite object "spr" with pointer to "tft" object

// Define the dimensions of your image
#define IMAGE_WIDTH  55  // Width of the image in pixels
#define IMAGE_HEIGHT 74  // Height of the image in pixels

uint16_t buffer[IMAGE_WIDTH * IMAGE_HEIGHT];


unsigned long lastFrameTime = 0;
int currentFrame = 0;
int frameDuration = 300;
StateMachine bunny;
std::vector<int> currentSeq;
bool skipPrevState = false;
int offsetX = 0;
QueueHandle_t snowQueue;

void setup() {
  tft.init();
  tft.setRotation(1); 
  tft.fillScreen(TFT_BLACK);
  tft.setSwapBytes(true);
  currentSeq = bunny.getRandomSeq();

  // snowQueue = xQueueCreate(300, sizeof(int));
  //   xTaskCreatePinnedToCore(
  //   drawSnow,         // Function to run on the second core
  //   "SnowTask",       // Task name
  //   10000,            // Stack size (bytes)
  //   NULL,             // Task input parameter
  //   1,                // Task priority
  //   NULL,             // Task handle
  //   1                 // Core to run the task on (1 for the second core)
  // );
}

void loop() {
  for (int i = 0; i < currentSeq.size(); i++) {
    int numOfSprites = bunny.getNumOfSprites(currentSeq[i]);

    while (numOfSprites > 0) {
      // int snowX;
      // if (xQueueReceive(snowQueue, &snowX, 0)) {
      //   // Draw snowflake at snowX position
      //   tft.drawPixel(snowX, tft.height() - 1, TFT_WHITE);  // Draw at the bottom of the screen
      // }
      unsigned long currentTime = millis();
      if (currentTime - lastFrameTime >= frameDuration) {
        bunny.elapsedTime = (esp_timer_get_time() / 1000000.0) - bunny.prevTime;
        bunny._behaviour.state = currentSeq[i];

        if (tft.width() - offsetX - IMAGE_WIDTH < 0 || tft.width() - offsetX - IMAGE_WIDTH > 195) {
          bunny._behaviour.state = Behaviour::States::jump;
        }

        switch (bunny._behaviour.state)
        {
          case Behaviour::States::sit : {
              bunny._behaviour.frames = 2;
              drawFrame(sit[currentFrame], offsetX);
          }
          break;
          case Behaviour::States::sleepy : {
              bunny._behaviour.frames = 4;
              drawFrame(sleepy[currentFrame], offsetX);
          }
          break;
          case Behaviour::States::stand : {
            bunny._behaviour.frames = 10;
            drawFrame(stand[currentFrame], offsetX);
          }
          break;
          case Behaviour::States::jump : {
            bunny._behaviour.frames = 6;
            std::cout << "Offset: " << tft.width() - offsetX - IMAGE_WIDTH << "\n";
            if (tft.width() - offsetX - IMAGE_WIDTH < -55) {
              offsetX = -55;
            }
            else {
              tft.fillRect(tft.width() - offsetX - IMAGE_WIDTH, tft.height() - IMAGE_HEIGHT, IMAGE_WIDTH, IMAGE_HEIGHT, TFT_BLACK);
              offsetX += 10;
            }
            drawFrame(jump[currentFrame], offsetX);
          }
          break;
          default:
            break;
        }

        lastFrameTime = currentTime;
        currentFrame = (currentFrame + 1) % bunny._behaviour.frames;
        bunny._behaviour.currFrame++;
        bunny._behaviour.print();
        numOfSprites--;
        
        if (bunny.elapsedTime <= 6 && numOfSprites == 0){
          numOfSprites = bunny.getNumOfSprites(currentSeq[i]);
        }
      }
    }
    bunny.prevTime = (esp_timer_get_time() / 1000000.0);
    currentFrame = 0;
    bunny._behaviour.currFrame = 0;
  }
  currentSeq = bunny.getRandomSeq();
  bunny.prevTime = (esp_timer_get_time() / 1000000.0);
  currentFrame = 0;
  bunny._behaviour.currFrame = 0;
}


void drawFrame(const uint16_t* frameData, int offsetX) {
    // Clear the active buffer
    memset(buffer, 0, IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint16_t));
    
    memcpy(buffer, frameData, IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint16_t));

    tft.pushImage((tft.width() - offsetX - IMAGE_WIDTH), (tft.height() - IMAGE_HEIGHT), IMAGE_WIDTH, IMAGE_HEIGHT, buffer);
}

uint32_t convertColor(float r, float g, float b) {
    uint32_t R = static_cast<uint32_t>(r * 255) << 16;
    uint32_t G = static_cast<uint32_t>(g * 255) << 8;
    uint32_t B = static_cast<uint32_t>(b * 255);

    return 0xFF000000 | R | G | B;  // Assuming alpha is 255 (fully opaque)
}
