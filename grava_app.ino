
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

#include <MFRC522.h> //biblioteca responsável pela comunicação com o módulo RFID-RC522
#include <SPI.h> //biblioteca para comunicação do barramento SPI

#define SS_PIN    2
#define RST_PIN   15

#define SIZE_BUFFER     18
#define MAX_SIZE_BLOCK  16

#define pinVerde     12
#define pinVermelho  32
#define pinAzul  33
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

void setup() {
  
  //inicial serial
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  pinMode(pinVerde, OUTPUT);
  pinMode(pinVermelho, OUTPUT);
  pinMode(pinAzul, OUTPUT);
  
  // Inicia MFRC522
  mfrc522.PCD_Init(); 
  // Mensagens iniciais no serial monitor
  Serial.println();
  //lcd
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  
}

void loop() {
  
  String matricula = "";
  
  //recebe dados do bluetooth
  while (SerialBT.available())  
      {
         matricula += (char)SerialBT.read();
      }
      
   lcd.setCursor(0, 0);
   lcd.clear();
   lcd.print("AGUARDANDO");
  if(matricula != "")
     {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Matricula recebida:");
        lcd.setCursor(0, 1);
        lcd.print(matricula);
        delay(1500);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("posicione o cartao");
        Serial.println("posicione o cartao");
        digitalWrite(pinAzul, HIGH);
        delay(3000);
        digitalWrite(pinAzul, LOW);
              
        // Aguarda a aproximacao do cartao
         
        //digitalWrite(pinVermelho, HIGH);
        //delay(1000);
        //digitalWrite(pinVermelho, LOW);
        if ( ! mfrc522.PICC_IsNewCardPresent()) 
        {
          lcd.clear();
          digitalWrite(pinVermelho, HIGH);
          lcd.setCursor(0, 0);
          lcd.print("ERRO DE LEITURA: ");
          lcd.setCursor(0, 1);
          lcd.print("ENVIAR DADOS NOVAMENTE.");
          delay(1000);
          digitalWrite(pinVermelho, LOW);
          Serial.println("ERRO DE LEITURA: ENVIAR DADOS NOVAMENTE.");
          return ;
          
        }
        // Seleciona um dos cartoes
        if( ! mfrc522.PICC_ReadCardSerial()) 
        {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Erro ao selecionar o cartao");
          lcd.setCursor(0, 1);
          lcd.print("ENVIAR DADOS NOVAMENTE.");
          Serial.println("Erro ao selecionar o cartao");
          digitalWrite(pinVermelho, HIGH);
          delay(1000);
          digitalWrite(pinVermelho, LOW);
          return ;
        }
               
        gravarDados(matricula);
     }
  delay(20);
}

void gravarDados(String &matricula)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Entrou na funcao gravar");
  Serial.println("Entrou na funcao gravar");
  lcd.setCursor(0, 1);
  lcd.print("Matricula:");
  lcd.setCursor(0, 2);
  lcd.print(matricula);
  Serial.println(matricula);
  
  //imprime os detalhes tecnicos do cartão/tag
  mfrc522.PICC_DumpDetailsToSerial(&(mfrc522.uid)); 
  // aguarda 30 segundos para entrada de dados via Serial
  //Serial.setTimeout(30000L);     
  //Serial.println(F("Insira os dados a serem gravados com o caractere '#' ao final\n[máximo de 16 caracteres]:"));
  
  //Prepara a chave - todas as chaves estão configuradas para FFFFFFFFFFFFh (Padrão de fábrica).
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //buffer para armazenamento dos dados que iremos gravar
  while(matricula.length() < MAX_SIZE_BLOCK){
    matricula += ' ';
  }
  byte bloco; //bloco que desejamos realizar a operação
 
  bloco = 1; //bloco definido para operação
  //Authenticate é um comando para autenticação para habilitar uma comuinicação segura
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &key, &(mfrc522.uid));
  

  if (status != MFRC522::STATUS_OK) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Falha de autenticacao");
    lcd.setCursor(0, 1);
    lcd.print("Tente de novo!");
  
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    //identificacao de erro
    digitalWrite(pinVermelho, HIGH);
    delay(1000);
    digitalWrite(pinVermelho, LOW);
    delay(1000);
    Serial.println("Erro de autenicacao");
    
    //colocar aqui no lcd que deu errado 
    return;
  }
  //else Serial.println(F("PCD_Authenticate() success: "));
  
  //Grava no bloco
  status = mfrc522.MIFARE_Write(bloco, (byte *)matricula.c_str(), MAX_SIZE_BLOCK);
  if (status != MFRC522::STATUS_OK) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Erro de gravacao");
    lcd.setCursor(0, 1);
    lcd.print("Tente de novo!");
    
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    digitalWrite(pinVermelho, HIGH);
    delay(1000);
    digitalWrite(pinVermelho, LOW);
    Serial.println("Erro de Gravacao, tente de novo");
    //colocar aqui no lcd que deu errado 
    return;
  }
  else{
    Serial.println(F("MIFARE_Write() success: "));

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Gravado com sucesso");
    lcd.setCursor(0, 1);
    lcd.print(matricula);
    
    digitalWrite(pinVerde, HIGH);
    delay(1000);
    digitalWrite(pinVerde, LOW);
    Serial.println("Tag gravada com sucesso");
    return;
  }
 
}
