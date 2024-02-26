#include <Adafruit_DotStar.h>
#include <SPI.h>

#define DATAPIN 9
#define CLOCKPIN 19
#define NUMPIXELS 576
#define BRIGHTNESS 200


#define Input_2 2
#define Input_1 2

const int Length = 50;
const int StreakLength = 10;
unsigned long CurrentTime;
int StormLevel = 0;

const int UpperSewerStart = 70;
const int UpperSewerEnd = 143;

struct PixelControl {
  bool PooPath;
  bool run;
  bool start;
  int Stream;
  int Pixel;
  int Poo;
  const int End;
  const int Begin;
};

PixelControl PipeOne = {false, false, false,NUMPIXELS,NUMPIXELS + Length,NUMPIXELS + StreakLength,144,NUMPIXELS};
PixelControl PooOne = {false, false, false, 143, 143,143,70,143};
// PixelControl PooTwo = {false, false, false, 20, 20,20, 143,0};

struct Time {
  unsigned long LastTriggered;
  long Duration;
};

Time PipesFrame = {0,13};
Time ToiletWait = {0,50};
Time PipeOnePooWait = {0,60};
Time SewersFrame = {0,10};


int WaveHue[Length];
int StreakHue[StreakLength];
int ColorHue[Length];
int SewerWaveHue[Length];
int LastPixel;
int SewersLastPixel;


Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

void WaveUpdate(){
  for(int x=0; x < Length; x++){
    ColorHue[Length - 1- x]= (150/2)+((150/2) * cos(x * (3.14/Length)));
    WaveHue[x]= (150/2)-((150/2) * sin(x * (3.14/Length)));
    SewerWaveHue[x]= (150/2)-((150/2) * sin(x * (3.14/Length)));

  }

  for(int x=0; x < StreakLength; x++){
    StreakHue[StreakLength - x] = (150/2)+((150/2)*cos(x*(3.14/StreakLength)));
  }
  
  ColorHue[0] = 0;
}

void setup() {
  strip.begin();
  strip.show();
  strip.setBrightness(BRIGHTNESS);
  Serial.begin(9600);

  pinMode(Input_2,INPUT);
  WaveUpdate();
}

void MediumPipe(){
  if(CurrentTime >= PipesFrame.LastTriggered + PipesFrame.Duration){
    if(analogRead(Input_1) > 600 && PipeOne.run == false){

      if(PipeOne.start == false){
        ToiletWait.LastTriggered = CurrentTime;
        PipeOnePooWait.LastTriggered = CurrentTime;
        PipeOne.start = true;
      }
      else if(CurrentTime >= ToiletWait.LastTriggered + ToiletWait.Duration){
        for(int x = PipeOne.Begin; x > PipeOne.Stream; x--){strip.setPixelColor(x, WaveHue[x -(Length*(x/Length))], 0, 150);}
        if(PipeOne.Stream >= PipeOne.End){PipeOne.Stream--;}
      }
    }

    else if(PipeOne.start == true && CurrentTime >= ToiletWait.LastTriggered + ToiletWait.Duration){
      PipeOne.run = true;

      if(PipeOne.Stream < PipeOne.Begin){
        for(int x = PipeOne.Pixel-Length; x > PipeOne.Stream; x--){strip.setPixelColor(x, WaveHue[x -(Length*(x/Length))], 0, 150);}
        if(PipeOne.Stream > PipeOne.End){PipeOne.Stream--;}
      }

      for(int x = 0; x < Length; x++){
        if(PipeOne.Pixel - x < PipeOne.End){break;}
        else if(PipeOne.Pixel - x >= PipeOne.End){strip.setPixelColor(PipeOne.Pixel - x,((WaveHue[x -(Length*(x/Length))] * ColorHue[x])/150), 0, ColorHue[x]);}
      }
      if(PipeOne.Pixel >= PipeOne.End){PipeOne.Pixel--;}
      else if(PipeOne.Pixel < PipeOne.End){
        PipeOne.PooPath = false;
        PipeOne.run = false;
        PipeOne.start = false;
        PipeOne.Stream = PipeOne.Begin;
        PipeOne.Pixel = PipeOne.Begin + Length;
        PipeOne.Poo = PipeOne.Begin + StreakLength;
        //digitalWrite(Output_1,LOW);
      }
    }

    if(PipeOne.start == true && CurrentTime >= PipeOnePooWait.LastTriggered + PipeOnePooWait.Duration){
      for(int x = 0; x < StreakLength; x++){
        if(PipeOne.Poo - x < PipeOne.End){break;}
        else if(PipeOne.Poo - x >= PipeOne.End){strip.setPixelColor(PipeOne.Poo - x, 0,StreakHue[x],0);}
      }

      if(PipeOne.Poo >= PipeOne.End){PipeOne.Poo--;}
      //else{digitalWrite(Output_1,HIGH);}
    }
  }
}


void UpperSewers(){
  if(CurrentTime >= SewersFrame.LastTriggered + SewersFrame.Duration){
    for(int x = 0; x < 144 - UpperSewerStart; x++){
      //strip.setPixelColor(x+UpperSewerStart,(StormLevel+1)*(SewerWaveHue[x-(Length*(x/Length))]/3)+15,0,20+(StormLevel*40));
      strip.setPixelColor(UpperSewerEnd-x,(StormLevel+1)*(SewerWaveHue[x-(Length*(x/Length))]/3)+15,0,20+(StormLevel*40));
    }

    if(PipeOne.Poo <= PipeOne.End && PipeOne.PooPath == false){
      PipeOne.PooPath = true;
      PooOne.start = true;
    }
    if(PooOne.start == true){
      for(int x = 0; x < StreakLength; x++){
        if(PooOne.Poo -x < PooOne.End){break;}
        else if(PooOne.Poo -x >= PooOne.End){strip.setPixelColor(PooOne.Poo-x, 0, StreakHue[x],0);}
      }

      if(PooOne.Poo > PooOne.End){PooOne.Poo--;}
      else {
        PooOne.Poo = PooOne.Begin;
        PooOne.start = false;
      }
    }

    // if(PooTwo.PooPath == true && PooTwo.run == false){
    //   PooTwo.start = true;
    //   PooTwo.run = true;
    // }
    // if(PooTwo.start == true){
    //   for(int x = 0; x < StreakLength; x++){
    //     if(PooTwo.Poo -x < PooTwo.start){break;}
    //     else if(PooTwo.Poo -x <= PooTwo.End){strip.setPixelColor(PooTwo.Poo-x, 0, StreakHue[StreakLength-1-x],0);}
    //   }

    //   if(PooTwo.Poo < PooTwo.End + StreakLength){PooTwo.Poo++;}
    //   else {
    //     PooTwo.Poo = PooTwo.Begin;
    //     PooTwo.start = false;
    //   }
    // }
  }
  

}

void loop() {
  CurrentTime = millis();
  MediumPipe();
  UpperSewers();

  // if(digitalRead(Input_2) == LOW){
  //   //PooTwo.PooPath = false;
  //   PooTwo.run = false;
  // }
  // else if(digitalRead(Input_2) == HIGH){PooTwo.PooPath = true;}

  //Serial.println (analogRead(2));
  if(CurrentTime  >= PipesFrame.Duration + PipesFrame.LastTriggered){
    LastPixel = WaveHue[0];
    for(int x = 0; x <= Length -1; x++){
      if(x!=Length -1){WaveHue[x]=WaveHue[x+1];} 
      else {WaveHue[x]=LastPixel;}
    }
    PipesFrame.LastTriggered = CurrentTime;
  }

  if(CurrentTime  >= SewersFrame.Duration + SewersFrame.LastTriggered){
    SewersLastPixel = SewerWaveHue[Length-1];
    for(int x = Length; x >= 0; x--){
      if(x!=0){SewerWaveHue[x]=SewerWaveHue[x-1];} 
      else {SewerWaveHue[x]=SewersLastPixel;}
    }

    SewersFrame.LastTriggered = CurrentTime;
  }

  strip.show();
}
