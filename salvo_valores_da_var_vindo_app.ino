//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void setup() {
  
 
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  
  char matricula[15];
  int i=0,j=0;
  int n = 0;
    for(j=0; j<15; j++)
    {
      matricula[j] = NULL;  
    }
    
    if (Serial.available()) 
      {
        SerialBT.write(Serial.read());
    
      }
  while (SerialBT.available())  
      {
         matricula[n] = SerialBT.read();
         n++;
         i = 1;
      }
      matricula[n+1] = '\0';
      
  if(i == 1)
     {
        Serial.println(matricula);
     }
        
        
 
  delay(20);
  
}
