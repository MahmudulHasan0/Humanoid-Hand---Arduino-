#include <SPI.h>
#include <RF24.h>
#include <nRF24L01.h>
#include <printf.h>
#include <RF24_config.h>
#define CE_PIN   7
#define CSN_PIN  8
RF24 radio(CE_PIN, CSN_PIN);  
const uint64_t pipe = 0xE8E8F0F0E1LL;   
 
#include <Servo.h>  
Servo finger1;                    //index  
Servo finger2;
Servo finger3;
Servo finger4;
int goLed = 4;
int lostLed = 2;
int reset = A0;                   //reset pin if lost
bool lost = 0;
struct flexData{
  byte flex1;
  byte flex2;
  byte flex3;
  byte flex4;
}; flexData data;

void setup(){    
  Serial.begin(115200);
  Serial.println("Starting");
  pinMode(goLed, OUTPUT);
  pinMode(lostLed, OUTPUT);
  finger1.attach(10);             //pinky           
  finger2.attach(9);
  finger3.attach(6);
  finger4.attach(5);
  radio.begin();            
  //radio.setAutoAck(false);
  radio.setPALevel(RF24_PA_LOW); 
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1,pipe); 
  radio.setRetries(15,15);       
  radio.startListening(); 
}

unsigned long lastRecvTime = 0;
void loop() 
{ 
  while (radio.available()){                                 //If i get data, let me know 
    radio.read(&data, sizeof(flexData));
    lastRecvTime = millis(); 
  }  
  unsigned long now = millis();  
  Serial.print(data.flex1); Serial.print("     ");            //index
  Serial.print(data.flex2); Serial.print("     "); 
  Serial.print(data.flex3); Serial.print("     ");
  Serial.print(data.flex4); Serial.print("     ");
  finger1.write(data.flex1);                                  //index
  finger2.write(data.flex2);
  finger3.write(data.flex3);
  finger4.write(data.flex4); 

  Serial.print("     Lost: ");                               //print status of lost variable for me
  Serial.print(lost);
  
  if(radio.available()) { 
    Serial.print("                 Got!");
    digitalWrite(goLed,HIGH);
    digitalWrite(lostLed,LOW);
    lost = 0;
  }

  if(now-lastRecvTime > 2000 && !radio.available()) {         //If lost, reset fingers and board
    lost = 1;
    resetFingers();
    Serial.print("                 Lost!");
    digitalWrite(lostLed,HIGH);
    digitalWrite(goLed,LOW);
    Serial.print("       RESET !!" );
   // analogWrite(reset, 1023);
  }
  radio.startListening();  
  Serial.println();  
} //end of program

void resetFingers(){
  data.flex1 = 105;
  data.flex2 = 90;
  data.flex3 = 180;
  data.flex4 = 180;
}
