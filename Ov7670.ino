#include"config.h"

//I don't check pin right after i change the define
#define SCCB_SCL 21
#define SCCB_SDA 20
//#define VSYNC 2
#define HREF 24
#define WEN 25//25
//#define XCLK 26//PA4
#define RESET 27
#define OE 28//PWON //28
#define PCLK 29

#define D0 37//PC0
#define D1 36//PC1
#define D2 35//PC2
#define D3 34//PC3
#define D4 33//PC4
#define D5 32//PC5
#define D6 31//PC6
#define D7 30//PC7

volatile int OV_State=0;
int c_data;
char incomingByte;
void setup(){
  Serial.begin(115200);
  while (!Serial) {
    ;
  }
  pinMode(HREF,OUTPUT);
  pinMode(WEN,OUTPUT);
  pinMode(RESET,OUTPUT);
  pinMode(OE,OUTPUT);
  pinMode(PCLK,OUTPUT);
  DDRC = 0x00;
  digitalWrite(OE,LOW);
  digitalWrite(WEN,HIGH);
  Sensorinit();
  readReset();
  while(incomingByte!='0'){//key in 0 to serial monitor can run next state.....
    incomingByte=Serial.read();
  }
  Serial.println(incomingByte,HEX);
  digitalWrite(13,HIGH);
  delay(500);
  digitalWrite(13,LOW);
}

void loop(){
  attachInterrupt(0,watcher,RISING);
  while(OV_State != 0){//外部中斷0==>Vsync
  }
  readReset();
  digitalWrite(OE,LOW);
  Serial.println("start....");
  Serial.println(0xff,HEX);
  digitalWrite(13,HIGH);
  delay(500);
  digitalWrite(13,LOW);
  delay(10);
 
  for(int a=0;a<240;a++){ //read pixel
    Serial.print("row :");
     Serial.print(a);
     Serial.print("   ");
    for(int b=0;b<320;b++){
      digitalWrite(PCLK,LOW); 
      c_data=PINC;//<<8;//读高位

      digitalWrite(PCLK,HIGH); 
      digitalWrite(PCLK,LOW);
      //c_data|=PINC;//读低位
      digitalWrite(PCLK,HIGH);
      Serial.print(c_data);
      Serial.print("   ");
    }
    Serial.println(" ");
  }
  digitalWrite(OE,HIGH);
  delayMicroseconds(20);
  while(1);
  OV_State=0;
}

void watcher(){
  if(OV_State == 0)           //判断状态，第一次下降沿
  {
    OV_State = 1;             //状态变为1
    digitalWrite(HREF,LOW);
    digitalWrite(HREF,HIGH);
    digitalWrite(WEN,HIGH);                //使能WEN，写
    //Serial.println(1000000);
  }
  else if(OV_State == 1)      //状态1，
  {
    digitalWrite(WEN,LOW);                 //关闭写使能，禁止写
    OV_State = 2;             //状态变为2，准备读数据
    detachInterrupt(0);		//关闭外部中断 
    //Serial.println(2000000);
  }  
}

void readReset(){
  digitalWrite(RESET,LOW);
  digitalWrite(PCLK,LOW);
  digitalWrite(PCLK,HIGH);
  digitalWrite(PCLK,LOW);
  digitalWrite(RESET,HIGH);
  digitalWrite(PCLK,HIGH);
}

void Sensorinit(){
  pinMode(SCCB_SCL,OUTPUT);
  pinMode(SCCB_SDA,OUTPUT);
  digitalWrite(SCCB_SDA,HIGH);
  digitalWrite(SCCB_SCL,HIGH);
  delayMicroseconds(10);
  writeSensor(0x12,0x80);//reset
  if(readSensor(0x0A)==0x76){
    if(readSensor(0x0B)==0x73){
      //Serial.println("setting");
      for(int i=0;i< OV7670_REG_NUM;i++){
        writeSensor(OV7670_reg[i][0],OV7670_reg[i][1]);
      }
    }
  }
}

byte readSensor(byte regID){
  byte temp;
  SCCB_Start();
  if(SCCB_Write(0x42)==0){
    SCCB_Stop();
    //Serial.print(regID);
    //Serial.println(" Read ERROR1");
  }
  delayMicroseconds(10);
  if(SCCB_Write(regID)==0){
    SCCB_Stop();
    //Serial.print(regID);
    //Serial.println(" Read ERROR2");
  }
  SCCB_Stop();
  delayMicroseconds(10);
  SCCB_Start();
  if(SCCB_Write(0x43)==0){
    SCCB_Stop();
    //Serial.print(regID);
    //Serial.println(" Read ERROR3");
  }
  delayMicroseconds(10);
  temp=SCCB_Read();
  SCCB_NASK();
  SCCB_Stop();
  return temp;
}

void writeSensor(byte regID, byte regDat){
  SCCB_Start();
  if(SCCB_Write(0x42)==0){
    SCCB_Stop();
    //Serial.print(regID);
    //Serial.println(" Write ERROR1");
  }
  delayMicroseconds(10);
  if(SCCB_Write(regID)==0){
    SCCB_Stop();
    //Serial.print(regID);
    //Serial.println(" Write ERROR2");
  }
  delayMicroseconds(10);
  if(SCCB_Write(regDat)==0){
    SCCB_Stop();
    //Serial.print(regID);
    //Serial.println(" Write ERROR3");
  }
  delayMicroseconds(10);
  SCCB_Stop();
}

void SCCB_Start(){
  digitalWrite(SCCB_SDA,HIGH);
  delayMicroseconds(20);
  digitalWrite(SCCB_SCL,HIGH);
  delayMicroseconds(20);
  digitalWrite(SCCB_SDA,LOW);
  delayMicroseconds(20);
  digitalWrite(SCCB_SCL,LOW);
  delayMicroseconds(30);
}

void SCCB_Stop(){
  digitalWrite(SCCB_SDA,LOW);
  delayMicroseconds(20);
  digitalWrite(SCCB_SCL,HIGH);
  delayMicroseconds(20);
  digitalWrite(SCCB_SDA,HIGH);
  delayMicroseconds(20);
}

void SCCB_NASK(){
  digitalWrite(SCCB_SDA,HIGH);
  delayMicroseconds(20);
  digitalWrite(SCCB_SCL,HIGH);
  delayMicroseconds(20);
  digitalWrite(SCCB_SCL,LOW);
  delayMicroseconds(20);
  digitalWrite(SCCB_SDA,LOW);
  delayMicroseconds(20);
}

byte SCCB_Write(byte data){
  for(byte i=0x80;i>0;i>>=1){
    if(data&i){
      digitalWrite(SCCB_SDA,HIGH);
    }
    else{
      digitalWrite(SCCB_SDA,LOW);
    }
    delayMicroseconds(20);
    digitalWrite(SCCB_SCL,HIGH);
    delayMicroseconds(20);
    digitalWrite(SCCB_SCL,LOW);
    delayMicroseconds(20);
  }
  //digitalWrite(SCCB_SDA,HIGH);
  byte temp;
  pinMode(SCCB_SDA,INPUT);
  delayMicroseconds(20);
  digitalWrite(SCCB_SCL,HIGH);
  delayMicroseconds(20);
  if(digitalRead(SCCB_SDA)){
    temp=0;
  }   //SDA=1发送失败，返回0}
  else {
    temp=1;
  } 
  digitalWrite(SCCB_SCL,LOW);
  delayMicroseconds(20);
  pinMode(SCCB_SDA,OUTPUT); 
  return temp;
}

byte SCCB_Read(){
  byte data;
  pinMode(SCCB_SDA,INPUT);
  delayMicroseconds(50);
  for(int i=0;i<8;i++){
    digitalWrite(SCCB_SCL,HIGH);
    data <<= 1;
    delayMicroseconds(20);
    if(digitalRead(SCCB_SDA)){
      data++;
    }
    digitalWrite(SCCB_SCL,LOW);
    delayMicroseconds(20);
  }
  pinMode(SCCB_SDA,OUTPUT); 
  return data;
}




