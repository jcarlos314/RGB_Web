//Sensor RGB
#include <ColorConverterLib.h>
#include <Wire.h>
#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X);

//Sensor Ultrasonico
const int trigPin = 2; 
const int echoPin = 5;
long duration;
int distance;

//Led RGB
//canales para comunicarse via PMW. Eso es porque no tenemos salida Analogicas, el cual se hara en el metodo adelante
#define LEDC_CHANNEL_0_R     1
#define LEDC_CHANNEL_1_G     2
#define LEDC_CHANNEL_2_B     3

//Bits de precision para el timer
#define LEDC_TIMER_13_BIT  13

//Frecuencia base para el led. Es la frecuencia definida
#define LEDC_BASE_FREQ     5000

//pines usados para enviar el PWM. Estos si son los pines propios de ESP
#define LED_PIN_R            18
#define LED_PIN_G            19
#define LED_PIN_B            23

//Metodo PROPIO para crear el ANALOGWRITE
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) 
{
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);
  ledcWrite(channel, duty);
}


void setup() {
  //////////////////////PARTE PARA CONFIGURAR LA CONEXION DEL ESP A LA RED
  Serial.begin(9600);
  Serial.println("");

  //////////////////PARTE QUE PONE LA CONFIGRACION DE LOS LEDS, EL TCS Y EL ULTRASONICO
  //Se inicializan los leds indicando el canal, frecuencia y el timer, se ancla al pin que le indicamos un canal antes inicializado
  ledcAttachPin(LED_PIN_R, 1);
  ledcSetup(1, 12000, 8); 
  ledcAttachPin(LED_PIN_G, 2);
  ledcSetup(2, 12000, 8);
  ledcAttachPin(LED_PIN_B, 3);
  ledcSetup(3, 12000, 8);

  //se inicializa el sensor ultrasonico, indicando quien va a ser el output y el put
  pinMode(trigPin, OUTPUT); //Manda
  pinMode(echoPin, INPUT);  //Recibe
  
  //se verifica que el sensor RGB este funcionando
    if (!tcs.begin())
  {
    Serial.println("Error al iniciar TCS34725");
    while (1) delay(1000);
  }
}

int contador=0;

void loop() {
  
  
  //////////////////////////////PROCESO PARA DETECTAR OBJETO Y MOSTRAR COLOR
  
  //Aqui se hace el proceso de distancia
  digitalWrite(trigPin, LOW); //Inicializa el trigger
  delayMicroseconds(2);
  
  digitalWrite(trigPin, HIGH); //Dispara la onda
  delayMicroseconds(10);
  
  digitalWrite(trigPin, LOW); //Estado apagado
  
  //Se lee el pulso para saber la duracion
  duration = pulseIn(echoPin, HIGH);
  
  //Se calcula la distancia
  distance= duration*0.034/2;
  
  //Si al leer la distancia sale igual o menor a 5cm se hace el if
  if(distance <= 5)
  {
    contador++; //Se anade al contador un extra
    if (contador == 3) //Si detecta 3 veces la condicion/objeto, entonces activa 
    {
      Serial.println("+++OBJETO DETECTADO++++");
      uint16_t clear, red, green, blue; 
      delay(60); // Cuesta 50ms capturar el color
      
      tcs.getRawData(&red, &green, &blue, &clear); //Valores del sensor
      // Hacer rgb medición relativa
      uint32_t sum = clear;
      float r, g, b;
      r = red; 
      r /= sum; 
      g = green; 
      g /= sum;
      b = blue; 
      b /= sum;
      // Escalar rgb a bytes
      r *= 256; 
      g *= 256; 
      b *= 256;
      
      // Convertir a hue, saturation, value
      double hue, saturation, value;
      //Toda la transoformacion
      ColorConverter::RgbToHsv(static_cast<uint8_t>(r), static_cast<uint8_t>(g), static_cast<uint8_t>(b), hue, saturation, value);
      // Mostrar nombre de color
      printColorName(hue * 360, saturation*360, value*360);
      delay(1000);
      contador = 0;
     }
     else
     {
      delay(10);
     }
  }
  else
  {
    Serial.println("***OBJETO NO DETECTADO***");
    contador = 0;
    ColorMandar(0,0,0);
  }
}

void ColorMandar(int r , int g , int b ) //Metodo para mostrar el color
{
  ledcWrite(1, r);
  ledcWrite(2, g);
  ledcWrite(3, b);
}

void printColorName(double hue,double value,double saturation) //Este metodo hara que muestre los colores
{   
  if (hue < 15)
  {
    Serial.println("Rojo");
    ColorMandar(255,0,0);
  }
  else if (hue < 45)
  {
    Serial.println("Anaranjado");
    ColorMandar(255,125,0);
  }
  else if (hue < 90)
  {
    Serial.println("Amarillo");
    ColorMandar(255,255,0);
  }
  else if (hue < 150)
  {
    Serial.println("Verde");
    ColorMandar(0,255,0);
  }
  else if (hue < 210)
  {
    Serial.println("Cyan");
    ColorMandar(0,255,255);
  }
  else if (hue < 270)
  {
    Serial.println("Azul");
    ColorMandar(0,0,255);
  }
  else if (hue < 330)
  {
    Serial.println("Magenta");
    ColorMandar(255,0,255);
  }
  else
  {
    Serial.println("Rojo");
    ColorMandar(255,0,0);
  }
}
