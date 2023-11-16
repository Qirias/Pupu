#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include "images.h"
#include <esp_timer.h> // Include the necessary library for ESP32 timer functions

#define IMAGE_WIDTH  55  // Width of the image in pixels
#define IMAGE_HEIGHT 74  // Height of the image in pixels


const uint16_t* sit[] = {sit7, sit6, sit5, sit4, sit3, sit2, sit1, sit0, sit1};
const uint16_t* sleepy[] = {sit2, sit3, sit4, sit5, sit6, sit7, sleep0, sleep1, sleep2, sleep1};
const uint16_t* stand[] = {stand0, stand1, stand2, stand3, stand4, stand5, stand4, stand3, stand2, stand1};
const uint16_t* jump[] = {jump0, jump1, jump2, jump3, jump4, jump5};


const uint16_t* sit_sleep[] = {sit0, sit1, sit2, sit3, sit4, sit5, sit6, sit7};
const uint16_t* sleep_sit[] = {sit7, sit6, sit5, sit4, sit3, sit2, sit1, sit0};

std::vector<std::vector<int>> sequence = {
    {0,2,3},
    {0,2},
    {0,3,2},
    {1,0,1},
    {1,0,3},
    {3,2,0},
    {3,0,2},
    {2,3,0},
    {2,0,3}
};


std::vector<std::vector<int>> allowedSeq = {
    {1,2,5,6},
    {0,2,3,5,6,7,8},
    {0,1,5,6},
    {0,1,2,5,6},
    {0,1,2,5,6},
    {0,1,2,3,4,6,7,8},
    {0,1,2,5,7,8},
    {0,1,2,5,6,8},
    {0,1,2,3,4}
};

std::vector<std::vector<int>> timers = {
    {20, 40, 60},
    {120, 240, 360},
    {4, 5, 6},
    {8, 10, 12}
};

std::vector<int> frameDuration = {600, 800, 500, 250};

std::vector<uint16_t> background = {TFT_BLACK, TFT_NAVY, TFT_DARKCYAN, TFT_MAROON,
                                    TFT_PURPLE, TFT_LIGHTGREY, TFT_DARKGREY,
                                    TFT_ORANGE, TFT_PINK, TFT_BROWN, TFT_SILVER, TFT_SKYBLUE, TFT_VIOLET};

class Behaviour {
public:
    enum States {sit, sleepy, stand, jump};

    int state;
    int prevState;

    int currFrame;
    int frames;

    Behaviour()
    : state(States::sit), prevState(States::sit), currFrame(0), frames(2) {}

    Behaviour(States newState, int newCurrFrame, int newFrames)
    : state(newState), currFrame(newCurrFrame), frames(newCurrFrame) {}

    void print() {
        std::string st;
        switch (state)
        {
        case States::sit : st = "sit";
            break;
        case States::sleepy : st = "sleepy";
            break;
        case States::stand : st = "stand";
            break;
        case States::jump : st = "jump";
            break;
        default:
            break;
        }
        std::cout <<"state: " << st << "\tcurrFrame: " << currFrame << "\tframes: " << frames <<'\n';
    }
};

class StateMachine {
public:
    Behaviour _behaviour;
    unsigned long long prevTime;
    unsigned long long elapsedTime;

    std::vector<int> getRandomSeq() {
        unsigned long long currentMicros = esp_timer_get_time(); // Get current time in microseconds
        unsigned int seed = static_cast<unsigned int>(currentMicros & 0xFFFFFFFF); // Use lower 32 bits as seed
        prevTime = currentMicros / 1000000.0;
        srand(seed);
        _behaviour.currFrame = 0;
        int index;
        while (true) {
            index = rand() % 9;
            if (std::find(allowedSeq[_behaviour.prevState].begin(), allowedSeq[_behaviour.prevState].end(), sequence[index][0]) != allowedSeq[_behaviour.prevState].end())
                break;
        }
        // std::cout << "Sequence index: " << index << "\n";
        return sequence[index];
    }

    int getNumOfSprites(int i) {
        switch (i)
        {
        case 0 : return 2;
            break;
        case 1 : return 4;
            break;
        case 2 : return 10;
            break;
        case 3 : return 6;
            break;
        default: return 2;
            break;
        }
    }

    StateMachine()
    : _behaviour(Behaviour(Behaviour::States::sit, 0, 8)), prevTime(0), elapsedTime(0) {}    
};