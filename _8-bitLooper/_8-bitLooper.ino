#include<DmxSimple.h>

const byte fixtureCount = 1;
const byte channelCount = 3;
const unsigned int totalDMX = (fixtureCount * channelCount) + 1;
const byte frameRate = 44;
const unsigned int frameInterval = 1000 / frameRate;
float dmxVals[totalDMX];
float currentVals[fixtureCount][channelCount];
byte targetVals[fixtureCount][channelCount];
float differentials[fixtureCount][channelCount];

unsigned int clamp8bit(int value){
  if(value < 0) {
    value = 0;
  }
  if(value > 255){
    value = 255;
  }
  return value;
}

void renderDMX(){
  for(int d = 1; d < totalDMX; d++) {
    DmxSimple.write(d, dmxVals[d]);
  }
}

void setDMX() {
  for(int fixture = 0; fixture < fixtureCount; fixture++) {
    for(int channel = 0; channel < channelCount; channel++) {
      int channelSpace = fixture * channelCount;
      int index = (channelSpace + channel) + 1;
      dmxVals[index] = currentVals[fixture][channel];
    }
  }
}

void setDifferentials(float frames) {
  for(int fixture = 0; fixture < fixtureCount; fixture++) {
    for(int channel = 0; channel < channelCount; channel++) { 
      float start = float(currentVals[fixture][channel]);
      float finish = float(targetVals[fixture][channel]);
      differentials[fixture][channel] = (finish - start) / frames;
    }
  }
}

void stepFrame() {
  for(int fixture = 0; fixture < fixtureCount; fixture++) {
    for(int channel = 0; channel < channelCount; channel++) { 
      currentVals[fixture][channel] += differentials[fixture][channel];       
    }
  }
  setDMX();
  renderDMX();
}

void fadeColor(byte (&color)[fixtureCount][channelCount], int fadeTime){
  Serial.println("fading color");
  for(int fixture = 0; fixture < fixtureCount; fixture++) {
    for(int channel = 0; channel < channelCount; channel++) { 
      targetVals[fixture][channel] = color[fixture][channel];   
    }
  }
  
  float frames = fadeTime * frameRate;
  setDifferentials(frames);
  int now;
  for(int frame = 0; frame < frames - 1; frame++){
    //now = millis();
    stepFrame();
    //while(millis() - now < frameInterval) {}
    delay(frameInterval);
  }

  for(int fixture = 0; fixture < fixtureCount; fixture++) {
    for(int channel = 0; channel < channelCount; channel++) { 
      currentVals[fixture][channel] = targetVals[fixture][channel];   
    }
  }

  setDMX();
  renderDMX();
  delay(frameInterval);


}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial){};
  Serial.println("serial initialized");
  /*
  currentVals[0][0] = 65534;
  currentVals[1][0] = 0;
  Serial.println("values set");
  setDMX();
  Serial.println("DMX Values set");
  renderDMX(); */
}

byte color1[][3] = {{255,255,255}};
byte color2[][3] = {{0,255,0}};

void loop() {
  // put your main code here, to run repeatedly:
  fadeColor(color1, 5);
  delay(1500);
  fadeColor(color2, 30);
  delay(1500); 

}
