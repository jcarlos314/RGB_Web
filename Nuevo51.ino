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

String MensajeDetectado;
int ColorEstado;
String ColorMostrar;


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

//wifi
int contadorcolor[]={0,0,0,0,0,0,0};

#include <WiFi.h>
WiFiServer server(80);
const char* ssid     = "benetts";
const char* password = "123456789";
int contconexion = 0;
String header; // Variable para guardar el HTTP request


String pagina = "<!DOCTYPE html>"
"<html>"
"<head>"
"<meta http-equiv='refresh' content='0.5'>"
"<meta charset='utf-8'>"
"<title>Servidor Web ESP32</title>"
"</head>"
"<body style='background-color:######'>"
"<center>"
"<h1>Colores del sensor</h1>"
;

String Mostrar =
"<div id='fecha'></div>"
"<div id='hora'></div>"
"<script>"
"function PonerColor() {"
  "var d = new Date();"
  "var dia = d.getDate();"
  "var mes = d.getMonth();"
  "var ano = d.getFullYear();"
  "document.getElementById('fecha').innerHTML = dia+'/'+(mes+1)+'/'+ano;"
  "var hora = d.getHours();"
  "var minuto = d.getMinutes();"
  "document.getElementById('hora').innerHTML = hora+':'+minuto;"
"}"
"</script>"
"<script>"
"PonerColor();"
"</script>"
;

String piedepagina = 
"<p><a href='/limpiar'><button style='height:50px;width:100px;background-color: #4CAF50;display: inline-block;font-size: 16px;'> RESET </button></a></p>"
"<p><a href='/'><button style='height:50px;width:100px;background-color: #4CAF50;display: inline-block;font-size: 16px;'> FP </button></a></p>"
"</center>"
"</body>"
"</html>";

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


    //wifi
  Serial.begin(115200);
  //se verifica que el sensor RGB este funcionando
    if (!tcs.begin())
  {
    Serial.println("Error al iniciar TCS34725");
    while (1) delay(1000);
  }

  WiFi.begin(ssid, password);
  //Cuenta hasta 50 si no se puede conectar lo cancela
  while (WiFi.status() != WL_CONNECTED and contconexion <50) { 
    ++contconexion;
    delay(500);
    Serial.print(".");
  }
  if (contconexion <50) {
      
      Serial.println("");
      Serial.println("WiFi conectado");
      Serial.println(WiFi.localIP());
      server.begin();
  }
  else { 
      Serial.println("");
      Serial.println("Error de conexion");
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
      MensajeDetectado = "SE HA DETECTADO UN OBJETO DE COLOR: ";
      ColorEstado = 1;
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
    MensajeDetectado = "--*NO SE HA DETECTADO UN OBJETO*--";
    ColorMostrar = "";
    ColorEstado = 0;
    contador = 0;
    ColorMandar(0,0,0);
  }

  ///////////////////////////INICIALIZACION DE WIFI

  WiFiClient client = server.available();   // Escucha a los clientes entrantes

  if (client) {                             // Si se conecta un nuevo cliente
    Serial.println("New Client.");          // 
    String currentLine = "";                //
    while (client.connected()) {            // loop mientras el cliente está conectado
      if (client.available()) {             // si hay bytes para leer desde el cliente
        char c = client.read();             // lee un byte
        Serial.write(c);                    // imprime ese byte en el monitor serial
        header += c;
        if (c == '\n') {                    // si el byte es un caracter de salto de linea
          // si la nueva linea está en blanco significa que es el fin del 
          // HTTP request del cliente, entonces respondemos:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // enciende y apaga el GPIO
            if (header.indexOf("GET /limpiar") >= 0) {
              Serial.println("Borrando registros por orden de pagina web");
              contadorcolor[0] = 0;
              contadorcolor[1] = 0;
              contadorcolor[2] = 0;
              contadorcolor[3] = 0;
              contadorcolor[4] = 0;
              contadorcolor[5] = 0;
              contadorcolor[6] = 0;
            }
            // Muestra la página web
            client.println(pagina);
            client.println("Rojo: ");
            client.println(contadorcolor[0]);
            client.println("<br>");
            client.println("Anaranjado: ");
            client.println(contadorcolor[1]);
            client.println("<br>");
            client.println("Amarillo: ");
            client.println(contadorcolor[2]);
            client.println("<br>");
            client.println("Verde: ");
            client.println(contadorcolor[3]);
            client.println("<br>");
            client.println("Cyan: ");
            client.println(contadorcolor[4]);
            client.println("<br>");
            client.println("Azul: ");
            client.println(contadorcolor[5]);
            client.println("<br>");
            client.println("Magenta: ");
            client.println(contadorcolor[6]);
            client.println("<br>");
            if (ColorEstado = 1)
            {
              client.println("<br>");
              client.println(MensajeDetectado);
              client.println(ColorMostrar);
              client.println(Mostrar);     
            }
            else if (ColorEstado = 0)
            {
              client.println("<br>");
              client.println(MensajeDetectado);
              }
              
              client.println(piedepagina);
            // la respuesta HTTP temina con una linea en blanco
            client.println();
            break;
          } 
          else 
          { // si tenemos una nueva linea limpiamos currentLine
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {  // si C es distinto al caracter de retorno de carro
          currentLine += c;      // lo agrega al final de currentLine
        }
      }
    }
    // Limpiamos la variable header
    header = "";
    // Cerramos la conexión
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
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
    contadorcolor[0]++;
    ColorMostrar = "Rojo";
  }
  else if (hue < 45)
  {
    Serial.println("Anaranjado");
    contadorcolor[1]++;
    ColorMandar(255,125,0);
    ColorMostrar = "Anaranjado";
  }
  else if (hue < 90)
  {
    Serial.println("Amarillo");
    contadorcolor[2]++;
    ColorMandar(255,255,0);
    ColorMostrar = "Amarillo";
  }
  else if (hue < 150)
  {
    Serial.println("Verde");
    contadorcolor[3]++;
    ColorMandar(0,255,0);
    ColorMostrar = "Verde";
  }
  else if (hue < 210)
  {
    Serial.println("Cyan");
    contadorcolor[4]++;
    ColorMandar(0,255,255);
    ColorMostrar = "Cyan";
  }
  else if (hue < 270)
  {
    Serial.println("Azul");
    contadorcolor[5]++;
    ColorMandar(0,0,255);
    ColorMostrar = "Azul";
  }
  else if (hue < 330)
  {
    Serial.println("Magenta");
    contadorcolor[6]++;
    ColorMandar(255,0,255);
    ColorMostrar = "Magenta";
  }
  else
  {
    Serial.println("Rojo");
  }
}
