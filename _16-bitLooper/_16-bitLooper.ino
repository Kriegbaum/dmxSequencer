#include<DmxSimple.h>

const byte fixtureCount = 3;
const byte channelCount = 3;
const unsigned int totalDMX = (fixtureCount * channelCount * 2) + 1;
const byte frameRate = 44;
const unsigned int frameInterval = 1000 / frameRate;
byte dmxVals[totalDMX];
unsigned int currentVals[fixtureCount][channelCount];
unsigned int targetVals[fixtureCount][channelCount];
float differentials[fixtureCount][channelCount];

unsigned int clamp16bit(int value){
  if(value < 0) {
    value = 0;
  }
  if(value > 65535){
    value = 65535;
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
      int highIndex = ((channelSpace + channel) * 2) + 1;
      int lowIndex = ((channelSpace + channel) * 2) + 2;
      dmxVals[highIndex] = (currentVals[fixture][channel] & 0xFF00) / 256;
      dmxVals[lowIndex] = currentVals[fixture][channel] & 0x00FF;
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

void fadeColor(int (&color)[fixtureCount][channelCount], int fadeTime){
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

  now = millis();

  for(int fixture = 0; fixture < fixtureCount; fixture++) {
    for(int channel = 0; channel < channelCount; channel++) { 
      currentVals[fixture][channel] = targetVals[fixture][channel];   
    }
  }

  setDMX();
  renderDMX();
  while(millis() - now < frameInterval) {}

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

int color1[fixtureCount][channelCount] = {{65535, 0, 65535}, {0,0,0}, {0,65535,65535}};
int color2[][3] = {{0,0,0}, {0,0,0}, {0,0,0}};

void loop() {
  // put your main code here, to run repeatedly:

  fadeColor(color1, 10);
  Serial.println("HOLDING");
  Serial.print("Framerate: ");
  Serial.println(frameRate);
  delay(1500);
  fadeColor(color2, 10);
  Serial.println("HOLDING");
  Serial.print("Frame interval: ");
  Serial.println(frameInterval);
  delay(1500); 

}
