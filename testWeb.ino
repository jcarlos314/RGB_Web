#include <WiFi.h>

WiFiServer server(80); //Puerto 80

const char* ssid = "benetts";
const char* password = "12345678";

int contconexion = 0; //Contador
String header; //para guardar el http request

//----------------CÓDIGO HTML-----------------------
String page = "<!DOCTYPE html>"
"<html>"
"<head>"
"<title> Sensor RGB </title>"
"</head>"
"<center>"
"<h1> Sensor RGB </h1>"
"</center>"
"</body>"
"</html>";

void setup() {
  Serial.begin(115200);
  Serial.println("");

  //Conexión del Wi-Fi
  WiFi.begin(ssid, password); //Se ingresan los credenciales
  //Cuenta 50 veces para que darle tiempo a conectarse
  while (WiFi.status() != WL_CONNECTED and contconexion < 50) 
  {
    ++contconexion;
    delay(500);
    Serial.println(".");
  }
  if (contconexion < 50) //Si se conecta en menos de los 50 veces que se trato, se manda mensaje y la ip a utilizar
  {
    Serial.println("");
    Serial.println("Conexion completa");
    Serial.println(WiFi.localIP());
    server.begin();
  } 
  else //De lo contrario marcara un error
  {
    Serial.println("");
    Serial.println("Error de conexion");
  }
}

void loop(){
  //Proceso para hacer mostrar la pagina
  WiFiClient client = server.available(); //Se revisa si hay un cliente
  if (client) //Si hay un cleinte se imprime
  {                             
    Serial.println("New Client.");          
    String currentLine = "";                
    while (client.connected()) //Ciclo para revisar si hay un nuevo cliente
    {            
      if (client.available()) //Si esta el cliente disponible
      {            
        char c = client.read(); //Asignamos en una variable el cliente y la mostramos           
        Serial.write(c); //Al escribirlo agregamos el header                
        header += c;
        if (c == '\n') 
        {                 
          if (currentLine.length() == 0) //Esto es para escribir cosas de conexiones extras, cerrar la conexion y mostrar la pagina.
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println(page);
            client.println();
            break;
          } 
          else 
          {
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        { 
          currentLine += c;      
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
