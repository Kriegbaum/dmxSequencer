#include <Arduino.h>
#include <DmxOutput.h>

#define FRAMERATE 44
#define UNIVERSE_LENGTH 512
DmxOutput dmx;
uint8_t universe[UNIVERSE_LENGTH + 1];

uint8_t splitHigh(uint16_t largeNumber){
  return largeNumber & 0xFF00 / 256;
}

uint8_t splitLow(uint16_t largeNumber){
  return largeNumber & 0x00FF;
}

class RGBFixture{
  private:
    int startAddress;
    float currentValues[3] = {0,0,0};
    float differentials[3] = {0,0,0};
    uint16_t targetValues[3] = {0,0,0};
    int remainingFrames = 0;

    int getFrameCount(float fadeTime) {
      int frameCount = fadeTime * FRAMERATE;
      if(frameCount == 0) {
        return 1;
      }
      else {
        return frameCount;
      }
    }

    void updateSelf(){
      if(remainingFrames == 1){
        for(int i = 0; i < 3; i++){
          currentValues[i] = targetValues[i];
        }
      }
      else if(remainingFrames == 0){
        return;
      }
      else{
        for(int i = 0; i < 3; i++){
          currentValues[i] -= differentials[i];
        }
      }
    }

    void updateDMX(){
      for(int i = 0; i < 3; i ++){
        universe[i + startAddress] = splitHigh(currentValues[i]);
        universe[i + startAddress + 1] = splitLow(currentValues[i]);
      }
    }
    
  public:
    void setAddress(int address){
      startAddress = address;
    }
    
    void setColor16(uint16_t color[3], float fadeTime){
      remainingFrames = getFrameCount(fadeTime);
      for(int i = 0; i < 3; i++){
        targetValues[i] = color[i];
        differentials[i] = (targetValues[i] - currentValues[i]) / remainingFrames;
      }
    }

   bool isBusy(){
    if(remainingFrames > 0) {
      return true;
    }
    else{
      return false;
    }
   }

   int getRed(){
    return currentValues[0];
   }
   int getGreen(){
    return currentValues[1];
   }
   int getBlue(){
    return currentValues[2];
   }

   void tick(){
    if(remainingFrames < 1) {
      return;
    }
    else{
      updateSelf();
      updateDMX();
      remainingFrames -= 1;
    }
   }
};

RGBFixture fixture;

uint16_t color1[3] = {65534, 65534, 65534};
uint16_t color2[3] = {65534, 32767, 0};
uint16_t color3[3] = {65534, 0, 0};


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial){};
  Serial.println("Serial Online..");
  dmx.begin(0);
  for(int i = 1; i < UNIVERSE_LENGTH + 1; i++){
    universe[i] = 0;
  }
  Serial.println("DMX Online...");
  fixture.setAddress(1);
  fixture.setColor16(color3, 1.5);
  Serial.println("Fixtures initialized...");
}

void loop() {
  // put your main code here, to run repeatedly:
  dmx.write(universe, UNIVERSE_LENGTH);
  //got nuffin to do while DMX transmits
  while (dmx.busy()){}
  delay(1);
}
