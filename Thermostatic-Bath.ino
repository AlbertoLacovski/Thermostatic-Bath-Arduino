#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

const int oneWirePin = 13;
const int zeroCrossPin = 2;
const int cargaPin = 3;
volatile long carga = 0;  // 0 a 100
unsigned long time;
unsigned long time1 =0;
unsigned long time2 =0;
unsigned long timeCT =0;
int untilTurnOff = 10; //porcentagem antes de desligar
long tCT = 0;

float Tsp =0;
float T = 0;
float K[] = {2, 0.0002, 0};
float ISoma = 0;
float dt = 2; //em segundos
float ErroPssado = 0; //usado na derivativa
int contASD1 = 0;
int contASD2 = 0;
OneWire oneWire(oneWirePin);
DallasTemperature sensors(&oneWire);

void setup() {
  pinMode(zeroCrossPin, INPUT);
  pinMode(cargaPin, OUTPUT);
  pinMode(8, INPUT);
  pinMode(9, INPUT);
  attachInterrupt(digitalPinToInterrupt(zeroCrossPin), zeroCross, RISING); //Fala a regra que tem que seguir
  sensors.begin();
  lcd.begin(16, 2);
}

void loop() {
  time = micros();
  if((time-time1)>=100000){
      time1=time;
      verButton();
  }
  if((time-time2)>=(dt*1000000)){//atualiza o LCD a cada dt segundos
      if(carga<=50){ // enquanto o display mostra/pega temperatura e calcula o controle fica liga só se a carga for maior que 50%
        digitalWrite(cargaPin, LOW);
      }else{
        digitalWrite(cargaPin, HIGH);// só vai ficar low quando entrar no if de controla (proximo abaixo)
      }
      time2=time;
      sensors.requestTemperatures();
      T = sensors.getTempCByIndex(0);
      carga = controle(); //retorna a porcentagem da carga
      atualizarLCD(String(T)); //Mostra no LCD

  }
}

void zeroCross()  {
  if (carga>=untilTurnOff){
    tCT = 8333L*(100L-carga)/100L; //microsegundos que tem que esperar
    delayMicroseconds(tCT);
    digitalWrite (cargaPin,HIGH);
    delayMicroseconds(10);
    digitalWrite (cargaPin,LOW);

  }

}

void atualizarLCD(String show){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.setCursor(11, 0);
  lcd.print(show);
  lcd.setCursor(0, 1);
  lcd.print("Temp SP: ");
  lcd.setCursor(11, 1);
  lcd.print(Tsp);
}

float controle(){ //Retorna a porcentagem que a lampada deve ligar
  float Tcorr = Tsp + untilTurnOff/K[0]; //Temperatura corrigida
  float erro = Tcorr  - T;
  float Ppart = K[0]*erro;
  float Ipart = ISoma + K[1]*erro*dt;
  float Dpart = K[2]*(erro-ErroPssado)/dt;

  ErroPssado = erro;
  ISoma = Ipart;
  float saida = Ppart + Ipart + Dpart;
  if (saida>100){
    saida = 100;
  }
  else if(saida<0){
    saida =0;
  }
  return saida;
}

void verButton(){
  if (digitalRead(8)==1){
    Tsp = Tsp+1;
    atualizarLCD(String(T)); //Mostra no LCD
  }
  else if (digitalRead(9)==1){
    Tsp = Tsp-1;
    atualizarLCD(String(T)); //Mostra no LCD
  }
  if(Tsp<=0){
     Tsp=0;
  }
}
