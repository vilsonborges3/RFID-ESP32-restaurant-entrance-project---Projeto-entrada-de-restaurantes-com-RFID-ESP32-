#include "BluetoothSerial.h"
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial SerialBT;
String MACadd = "AA:BB:CC:11:22:33";
uint8_t address[6]  = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};
//uint8_t address[6]  = {0x00, 0x1D, 0xA5, 0x02, 0xC3, 0x22};
String name = "OBDII";
char *pin = "1234"; //<- standard pin would be provided by default
bool connected;
#define pinConectado 14

//parte rfid
#include <MFRC522.h> //biblioteca responsável pela comunicação com o módulo RFID-RC522
#include <SPI.h> //biblioteca para comunicação do barramento SPI

#define SS_PIN    2 //sda
#define RST_PIN   15 //rst

#define SIZE_BUFFER     18 //sck
#define MAX_SIZE_BLOCK  16

#define pinVerde     12
#define pinVermelho  32
//for rfid - sck 18 - sda 2 - rst 15 - miso 19 - mosi 23
//for display - sda 21 - scl 22
//esse objeto 'chave' é utilizado para autenticação
MFRC522::MIFARE_Key key;
//código de status de retorno da autenticação
MFRC522::StatusCode status;

// Definicoes pino modulo RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 

//lcd
#include <LiquidCrystal_I2C.h>
#include <Wire.h> // responsável pela comunicação com a interface i2c

// set the LCD number of columns and rows
int lcdColumns = 20;
int lcdRows = 4;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
//MENSAGEM COMO ROLAGEM
void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}

void setup() {
  pinMode(pinConectado, OUTPUT);
  Serial.begin(115200);
  SerialBT.begin("UFU_RU"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  // Inicia a serial
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus

  pinMode(pinVerde, OUTPUT);
  pinMode(pinVermelho, OUTPUT);
  
  // Inicia MFRC522
  mfrc522.PCD_Init(); 
  // Mensagens iniciais no serial monitor
   connected = SerialBT.connect(name);
  //connected = SerialBT.connect(address);
  
  
  // this would reconnect to the name(will use address, if resolved) or address used with connect(name/address).
  SerialBT.connect();

  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();

  
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("       UFU-RU");
  
// Aguarda a aproximacao do cartao
   
    //digitalWrite(pinVermelho, HIGH);
    //delay(1000);
    //digitalWrite(pinVermelho, LOW);
  
  leituraDados();

  /* // bluetooth
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }
    */
  // instrui o PICC quando no estado ACTIVE a ir para um estado de "parada"
  mfrc522.PICC_HaltA(); 
  // "stop" a encriptação do PCD, deve ser chamado após a comunicação com autenticação, caso contrário novas comunicações não poderão ser iniciadas
  mfrc522.PCD_StopCrypto1();  
  delay(20);
}


//faz a leitura dos dados do cartão/tag
void leituraDados()
{
  status ;
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    digitalWrite(pinVermelho, HIGH);
    delay(500);
    digitalWrite(pinVermelho, LOW);
    delay(500);
    return;
  }
  // Seleciona um dos cartoes
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    
    return;
  }
  String mat_lida = "";
  //imprime os detalhes tecnicos do cartão/tag
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); 

  //Prepara a chave - todas as chaves estão configuradas para FFFFFFFFFFFFh (Padrão de fábrica).
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //buffer para colocar os dados ligos
  byte buffer[SIZE_BUFFER] = {0};

  //bloco que faremos a operação
  byte bloco = 1;
  byte tamanho = SIZE_BUFFER;


  //faz a autenticação do bloco que vamos operar
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Erro autenticacao"));
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(pinVermelho, HIGH);
    delay(100);
    digitalWrite(pinVermelho, LOW);
    return;
  }

  //faz a leitura dos dados do bloco
  status = mfrc522.MIFARE_Read(bloco, buffer, &tamanho);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Falha de leitura "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(pinVermelho, HIGH);
    delay(100);
    digitalWrite(pinVermelho, LOW);
    return;
  }
  else{
      digitalWrite(pinVerde, HIGH);
      delay(100);
      digitalWrite(pinVerde, LOW);
  }

  Serial.print(F("\nDados bloco ["));
  Serial.print(bloco);
  Serial.print(F("]: "));

  for (uint8_t i = 0; i < MAX_SIZE_BLOCK; i++)
  {
      Serial.write(buffer[i]);
      if(((char)buffer[i]) != ' ')
        mat_lida += (char)buffer[i];
      
      /*if (SerialBT.available()) {
        Serial.println("Enviando dados para app via buffer");
          SerialBT.write(buffer[i]);
        }*/
  }
  SerialBT.print(mat_lida);

  //imprime os dados lidos
  
  /*
  if (SerialBT.available()) {
      SerialBT.write(mat_lida);
    }*/
    
  Serial.println(" ");

  return;
}
