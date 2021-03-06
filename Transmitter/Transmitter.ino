//Servo rest ranges for each finger:
//1) rest 180-60 pinky
//2) rest 180-70
//3) rest 90-0
//4) rest 105-0

#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24_config.h>                    //NRRF24 pin layout: UNO:  MOSI(pin11)MISO(pin12)SCK(pin13)(CE,CSN any pin; defined below)
#define CE_PIN      7                       //MEGA: MOSI(pin51)MISO(pin50)SCK(pin52) any pin; defined below)               
#define CSN_PIN     8
RF24 radio(CE_PIN, CSN_PIN); 
const uint64_t pipe = 0xE8E8F0F0E1LL;  
int flexPin_1 = A0;                         //index
int flexPin_2 = A1;
int flexPin_3 = A2;
int flexPin_4 = A3;

bool test;
const int maxReadingNumber =  4;
int finger[6][maxReadingNumber] = {};       //4 rows by infinite columns. 5 rows: each 4 fingers (gana start with 1 so made it 5)
int total[6] = {};                          //all are zeros here, smoothing 
int i = 0;                                  //initial index
int flex[6] = {};                           //final flex
int xfinger[6][maxReadingNumber] = {};      //4 rows by infinite columns. 5 rows: each 4 fingers (gana start with 1 so made it 5)
int xtotal[6] = {};                         //all are zeros here, smoothing 
int xi = 0;                                 //initial index
int xflex[6] = {};                          //final flex
int flex1; 
int flex2; 
int flex3; 
int flex4;

struct flexData{
  byte flex1;
  byte flex2;
  byte flex3;
  byte flex4;
}; flexData data;
   
int fingerLow[6] = {};         
int fingerHigh[6] = {};
void setup(){
  Serial.begin(115200);    
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setRetries(15, 15);
  radio.openWritingPipe(pipe);
  for(int v =0; v<maxReadingNumber;v++)//setting all elements to 0
  {     
    finger[1][v]=0;
    finger[2][v]=0;
    finger[3][v]=0;
    finger[4][v]=0;
    fingerLow[v]=0;
    fingerHigh[v]=0;
  }
}

void loop() 
{
  test = 0  ;
  total[1] = total[1] - finger[1][i];
  total[2] = total[2] - finger[2][i];
  total[3] = total[3] - finger[3][i];
  total[4] = total[4] - finger[4][i];
  if (test == 1)
  {
    finger[1][i] = analogRead(flexPin_1); 
    finger[2][i] = analogRead(flexPin_2); 
    finger[3][i] = analogRead(flexPin_3); 
    finger[4][i] = analogRead(flexPin_4); 
  }
  else 
  {
    finger[1][i] = constrain_flex_hiLo(1,flexPin_1, 1,2, 105,0);      //index
    finger[2][i] = constrain_flex_hiLo(2,flexPin_2, 5,0, 90,0);       //middle
    finger[3][i] = constrain_flex_hiLo(3,flexPin_3, 0,1, 180,70);     //ring
    finger[4][i] = constrain_flex_hiLo(4,flexPin_4, 4,6, 180,60);     //pinky
  }
  flexCalib();                        //going to data, different servo angles
  flexNoCalib();                      //for me, 0 to 180
  print_flexNoCalib();
  Serial.print("  |     ");
  print_flexCalib();
  
  radio.write(&data, sizeof(unsigned long));
  if (!radio.write( &data, sizeof(flexData) )){            //if it fails to send, reset 
    Serial.print("       Failed!    ");
    resetFingers();
  }
  if (radio.write(&data, sizeof(flexData))) {              //if u sent it let me know
    Serial.print("       Sent!      ");
  }
  Serial.println();
}

//////////////////////////////////////////FUNCTIONS/////////////////////////////////////////
int constrain_flex_hiLo(int index, int flexPin, int lo,int hi, int servoLo, int servoHi){      //data one
 int finger = analogRead(flexPin);
// *fingerLo[index] = &lo;      //for pointer
// *fingerHi[index] = &hi;
 fingerLow[index] = lo;       //for old way
 fingerHigh[index] = hi;
 finger = constrain(finger, lo, hi);
 int fingerServo = map(finger, lo,hi, servoLo,servoHi);    
 return fingerServo;
}

int constrain_flex_hiLo(int index, int flexPin, int servoLo, int servoHi){     //non pointer old way
 int finger = constrain(finger, fingerLow[index], fingerHigh[index]);
 int fingerServo = map(finger, fingerLow[index],fingerHigh[index], servoLo,servoHi);    
 return fingerServo;
 
 Serial.print("index: ");
 Serial.print(index);
 Serial.print(" ");
 Serial.print(fingerLow[index]);
 Serial.print("    ");
}

void flexCalib(){
  total[1] = total[1] + finger[1][i];
  total[2] = total[2] + finger[2][i];
  total[3] = total[3] + finger[3][i];
  total[4] = total[4] + finger[4][i]; 
  i = i + 1;                                            //I CANCELED THIS
  if (i >= maxReadingNumber)
  { 
    i = 0;
  }
  data.flex1 = total[1] / maxReadingNumber;
  data.flex2 = total[2] / maxReadingNumber;
  data.flex3 = total[3] / maxReadingNumber;
  data.flex4 = total[4] / maxReadingNumber;
}

void print_flexCalib(){
  Serial.print("data:  ");
  Serial.print(data.flex1); Serial.print("   ");
  Serial.print(data.flex2); Serial.print("   "); 
  Serial.print(data.flex3); Serial.print("   ");
  Serial.print(data.flex4); Serial.print("   ");
}

void flexNoCalib(){
  xtotal[1] = xtotal[1] - xfinger[1][xi];
  xtotal[2] = xtotal[2] - xfinger[2][xi];
  xtotal[3] = xtotal[3] - xfinger[3][xi];
  xtotal[4] = xtotal[4] - xfinger[4][xi];
  xfinger[1][xi] = constrain_flex_hiLo(1,flexPin_1, 3,5, 0,180);//pinky
  xfinger[2][xi] = constrain_flex_hiLo(2,flexPin_2, 0,180);     //ring
  xfinger[3][xi] = constrain_flex_hiLo(3,flexPin_3, 0,180);     //middle
  xfinger[4][xi] = constrain_flex_hiLo(4,flexPin_4, 0,180);     //index

  xtotal[1] = xtotal[1] + xfinger[1][xi];
  xtotal[2] = xtotal[2] + xfinger[2][xi];
  xtotal[3] = xtotal[3] + xfinger[3][xi];
  xtotal[4] = xtotal[4] + xfinger[4][xi];
  xi = xi + 1;
  if (xi >= maxReadingNumber)
  { 
    xi = 0;
  }
  flex1 = xtotal[1] / maxReadingNumber;
  flex2 = xtotal[2] / maxReadingNumber;
  flex3 = xtotal[3] / maxReadingNumber;
  flex4 = xtotal[4] / maxReadingNumber;
}
 
void print_flexNoCalib(){
  Serial.print(flex1); Serial.print("   ");    //just print stuff to let me know the status
  Serial.print(flex2); Serial.print("   "); 
  Serial.print(flex3); Serial.print("   ");
  Serial.print(flex4); Serial.print("   ");
}

void resetFingers(){
  data.flex1 = 105;
  data.flex2 = 90;
  data.flex3 = 180;
  data.flex4 = 180;
}






