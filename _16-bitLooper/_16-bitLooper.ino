#include <Arduino.h>
#include <DmxOutput.h>

#define FRAMERATE 44
#define UNIVERSE_LENGTH 512
#define DIMMER_MINIMUM 30

DmxOutput dmx;
//Universe length +1 because we are going to 1 index this array. First value is dummy
uint8_t universe[UNIVERSE_LENGTH + 1];

//Number of milliseconds between frames
const int frameInterval = 1000 / FRAMERATE;
//Remove?
int stepStart = 0;
//Timestamp of the last frame's beginning, used to set frame cadence
int lastFrame = 0;
//Where are we in the sequence?
int sequenceIterator = 0;
// Where is the dimmer at right now?
float dimmerValue;
int dimmerPin = A2;

uint8_t splitHigh(uint16_t largeNumber){
  //Return high order byte of a 16 bit value
  return largeNumber / 256;
}

uint8_t splitLow(uint16_t largeNumber){
  //Return low order byte of a 16 bit value
  return largeNumber & 0x00FF;
}

float mapFloat(int inp, int fromLow, int fromHigh){
  // Takes an input and an upper and lower bound for that input, returns the number in a range 0 to 1
  int mappedInt = map(inp, fromLow, fromHigh, DIMMER_MINIMUM, 100);
  return mappedInt * 0.01;
}

class RGBFixture{
  private:
    int startAddress;
    float currentValues[3] = {0,0,0};
    float differentials[3] = {0,0,0};
    uint16_t targetValues[3] = {0,0,0};
    int remainingFrames = 0;

    int getFrameCount(float fadeTime) {
      int frameCount = (fadeTime * FRAMERATE) / 1000;
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
      else if(remainingFrames <= 0){
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
        float dimmedValue = currentValues[i] * dimmerValue;
        int high = splitHigh((uint16_t)dimmedValue);
        int low  = splitLow((uint16_t)dimmedValue);
        universe[((i * 2) + startAddress)] = high;
        universe[((i * 2) + startAddress) + 1] = low;
      }
    }
    
  public:
    void setAddress(int address){
      startAddress = address;
    }
    
    void setColor16(uint16_t color[3], float fadeTime){
      //Used to initate a colorfade, accepts a 16 bit value
      remainingFrames = getFrameCount(fadeTime);
      for(int i = 0; i < 3; i++){
        targetValues[i] = color[i];
        differentials[i] = (targetValues[i] - currentValues[i]) / remainingFrames;
      }
    }

     void setColor8(uint16_t color[3], float fadeTime){
      //Used to initate a colorfade, accepts an 8 bit value and steps it up to 16 bits
      remainingFrames = getFrameCount(fadeTime);
      for(int i = 0; i < 3; i++){
        //We multiply this value in the 8 bit setter to step up to what it should be in a 16 bit scheme
        targetValues[i] = color[i] * 255;
        differentials[i] = (targetValues[i] - currentValues[i]) / remainingFrames;
     }
    }

   bool isBusy(){
    if(remainingFrames >= 0) {
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
    /*
    if(remainingFrames < 0) {
      return;
    }
    */
    updateSelf();
    updateDMX();
    if(remainingFrames < 0){
      return;
    }
    else{
      remainingFrames -= 1;
    }
   }
};

class SequenceStep{
  public:
    int wait;
    float fadeTime;
    uint16_t color[3];
    SequenceStep(uint16_t red, uint16_t green, uint16_t blue, float fadeTimeIn, int waitIn){
      wait = waitIn * 1000;
      fadeTime = fadeTimeIn * 1000;
      color[0] = red;
      color[1] = green;
      color[2] = blue;
    }
};

RGBFixture fixture;

SequenceStep step1(65534, 65534, 65534, 5, 5);
SequenceStep step2(0, 0, 65534, 5, 5);
SequenceStep step3(65534, 0, 0, 5, 5);

#define SEQUENCE_STEPS 6
SequenceStep *sequence[SEQUENCE_STEPS];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(100);
  //while(!Serial){};
  Serial.println("Serial Online..");
  
  dmx.begin(0);
  for(int i = 1; i < UNIVERSE_LENGTH + 1; i++){
    universe[i] = 0;
  }
  Serial.println("DMX Online...");
  
  fixture.setAddress(1);
  Serial.println("Fixtures initialized...");
  //SEQUENCE FOR CONE 4, PURPLE-YELLOW ALTERNATE NOW NOT ALTERNATE
  sequence[0] = new SequenceStep(255, 255, 255,  10, 0);
  sequence[1] = new SequenceStep( 10,  10, 255,   8, 3);
  sequence[2] = new SequenceStep(  0,   0, 255,   2, 3);
  sequence[3] = new SequenceStep( 94, 255,  97,  10, 0);
  sequence[4] = new SequenceStep(255, 150,   0,  10, 3);
  sequence[5] = new SequenceStep(255,   0, 255,  10, 3);
  fixture.setColor8(sequence[0]->color, sequence[0]->fadeTime);
  Serial.println("Sequence Initialized...");
  stepStart = millis();

  pinMode(dimmerPin, INPUT);
  dimmerValue = mapFloat(analogRead(dimmerPin), 0, 1023);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
  dimmerValue = mapFloat(analogRead(dimmerPin), 0, 1023);
  if(not fixture.isBusy()){
    if (millis() - stepStart > (sequence[sequenceIterator]->wait + sequence[sequenceIterator]->fadeTime)){
      Serial.print("Total time "); Serial.println(millis() - stepStart); Serial.println();
      sequenceIterator += 1;
      if(sequenceIterator > SEQUENCE_STEPS - 1) {
        sequenceIterator = 0;
      }
      Serial.print("Initiating step "); Serial.println(sequenceIterator);
      Serial.print("Fading for "); Serial.print(sequence[sequenceIterator]->fadeTime); Serial.println(" milliseconds");
      Serial.print("Waiting for "); Serial.print(sequence[sequenceIterator]->wait); Serial.println(" milliseconds");
      fixture.setColor8(sequence[sequenceIterator]->color, sequence[sequenceIterator]->fadeTime);
      stepStart = millis();
    }
  }
  
  if(millis() - lastFrame >= frameInterval) {
    fixture.tick();
    dmx.write(universe, UNIVERSE_LENGTH);
    lastFrame = millis();
  }
  //got nuffin to do while DMX transmits
  while (dmx.busy()){}
  //delay(1);
}
