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
      /*
      Serial.println("High index");
      Serial.println(highIndex);
      Serial.println(dmxVals[highIndex]);
      Serial.println("Low Index");
      Serial.println(lowIndex);
      Serial.println(dmxVals[lowIndex]);
      */
    }
  }
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial){};
  Serial.println("serial initialized");
  currentVals[0][0] = 65534;
  currentVals[1][0] = 0;
  Serial.println("values set");
  setDMX();
  Serial.println("DMX Values set");
  renderDMX();
}

void loop() {
  // put your main code here, to run repeatedly:
  currentVals[0][0]--;
  currentVals[1][0] ++;
  setDMX();
  renderDMX();
  delay(11);
}
