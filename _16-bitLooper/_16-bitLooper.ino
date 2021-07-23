#include <Arduino.h>
#include <DmxOutput.h>

#define FRAMERATE 44
#define UNIVERSE_LENGTH 512
DmxOutput dmx;
uint8_t universe[UNIVERSE_LENGTH + 1];

const int frameInterval = 1000 / FRAMERATE;
int stepStart = 0;
int lastFrame = 0;
int sequenceIterator = 0;

uint8_t splitHigh(uint16_t largeNumber){
  return largeNumber / 256;
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
          currentValues[i] += differentials[i];
        }
      }
    }

    void updateDMX(){
      for(int i = 0; i < 3; i ++){
        int high = splitHigh((uint16_t)currentValues[i]);
        int low  = splitLow((uint16_t)currentValues[i]);
        universe[((i * 2) + startAddress)] = high;
        universe[((i * 2) + startAddress) + 1] = low;
        Serial.print("Current: "); Serial.println(currentValues[i]);
        Serial.print("High:    "); Serial.println(high);
        Serial.print("Low:     "); Serial.println(low);
        Serial.print("DMX 1:   "); Serial.println(universe[1]);
        Serial.print("DMX 2:   "); Serial.println(universe[2]);
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

class SequenceStep{
  public:
    int wait;
    float fadeTime;
    uint16_t color[3];
    SequenceStep(uint16_t colorIn[3], float fadeTimeIn, int waitIn){
      wait = waitIn * 1000;
      fadeTime = fadeTimeIn * 1000;
      for(int i = 0; i < 3; i++){
        color[i] = colorIn[i];
      }
    }
};

RGBFixture fixture;

SequenceStep step1({65534, 65534, 65534}, 5, 5);
SequenceStep step2({0, 0, 65534}, 5, 5);
SequenceStep step1({65534, 0, 0}, 5, 5);

SequenceStep sequence[];
sequence[0] = step1;
sequence[1] = step2;
sequence[2] = step3;

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
  fixture.setColor16(color1, 15);
  Serial.println("Fixtures initialized...");
}

void loop() {
  if(not fixture.isBusy()){
    
  }
  if(millis() - lastFrame >= frameInterval) {
    // put your main code here, to run repeatedly:
    dmx.write(universe, UNIVERSE_LENGTH);
    lastFrame = millis();
    //got nuffin to do while DMX transmits
    while (dmx.busy()){}
  }
  delay(1);
}
