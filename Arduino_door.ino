#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>
#include <ArduinoJson.h> 

//hola

// Configuración de WiFi
const char* ssid = "ssid";
const char* password = "password";

// Configuración de MQTT
const char* mqtt_server = "IP";
const char* mqtt_topic_door1 = "Door1_topic";
const char* mqtt_topic_door2 = "Door2_topic";
const char* mqtt_topic_door3 = "Door3_topic";
const char* mqtt_username = "tu_usuario";
const char* mqtt_password = "tu_contraseña";

WiFiClient espClient;
PubSubClient client(espClient);

// Variables para almacenar el estado de los LED
bool ledStateDoor1 = false;
bool ledStateDoor2 = false;
bool ledStateDoor3 = false;
unsigned long ledOnTimeDoor1 = 0;
unsigned long ledOnTimeDoor2 = 0;
unsigned long ledOnTimeDoor3 = 0;

// Configuración de la pantalla LCD
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

// Variables para el temporizador de mensaje de puerta y LED
unsigned long startTime = 0;
const unsigned long displayDuration = 5000; // 5 segundos
bool doorOpen = false;

// Datos del timbre
const char* hostname = "your esp-rfid hostname"; // Cambia esto por tu hostname

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2); // Inicializar la pantalla LCD (16 columnas, 2 filas)

  // Conexión a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
    lcd.print("Conectando a WiFi");
  }
  Serial.println("Conectado a WiFi");
  lcd.clear();
  lcd.print("Conectado a WiFi");

  // Conexión a MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Conectando a MQTT...");
    lcd.print("Conectando a MQTT");
    if (client.connect("ArduinoClient", mqtt_username, mqtt_password)) {
      Serial.println("Conectado a MQTT");
      lcd.clear();
      lcd.print("Conectado a MQTT");
      client.subscribe(mqtt_topic_door1);
      client.subscribe(mqtt_topic_door2);
      client.subscribe(mqtt_topic_door3);
    } else {
      Serial.print("Error al conectar: ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // Inicializar los pines de los LED
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Controlar los LED según su estado
  controlLED(13, ledStateDoor1, ledOnTimeDoor1);
  controlLED(12, ledStateDoor2, ledOnTimeDoor2);
  controlLED(11, ledStateDoor3, ledOnTimeDoor3);

  // Mostrar mensaje de puerta abierta en la LCD
  if (doorOpen && (millis() - startTime >= displayDuration)) {
    lcd.clear();
    lcd.print("Door control");
    doorOpen = false;
    ledStateDoor1 = ledStateDoor2 = ledStateDoor3 = false; // Apagar los LEDs
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Mostrar el mensaje recibido en la pantalla LCD
  lcd.clear();
  lcd.print("Mensaje recibido:");
  lcd.setCursor(0, 1);
  for (int i = 0; i < length; i++) {
    lcd.print((char)payload[i]);
  }

  // Control de los LED basado en el topic del mensaje recibido
  if (strcmp(topic, mqtt_topic_door1) == 0) {
    ledStateDoor1 = true;
    ledOnTimeDoor1 = millis();
    lcd.clear();
    lcd.print("Door 1 open");
    startTime = millis();
    doorOpen = true;
    sendDoorbellMessage("Door 1"); // Enviar mensaje del timbre
    sendNotificationToPublisher("Door 1 opened"); // Enviar notificación al publicador
  } else if (strcmp(topic, mqtt_topic_door2) == 0) {
    ledStateDoor2 = true;
    ledOnTimeDoor2 = millis();
    lcd.clear();
    lcd.print("Door 2 open");
    startTime = millis();
    doorOpen = true;
    sendDoorbellMessage("Door 2"); // Enviar mensaje del timbre
    sendNotificationToPublisher("Door 2 opened"); // Enviar notificación al publicador
  } else if (strcmp(topic, mqtt_topic_door3) == 0) {
    ledStateDoor3 = true;
    ledOnTimeDoor3 = millis();
    lcd.clear();
    lcd.print("Door 3 open");
    startTime = millis();
    doorOpen = true;
    sendDoorbellMessage("Door 3"); // Enviar mensaje del timbre
    sendNotificationToPublisher("Door 3 opened"); // Enviar notificación al publicador
  } else {
    ledStateDoor1 = ledStateDoor2 = ledStateDoor3 = false;
  }
}

void sendDoorbellMessage(const char* doorName) {
  unsigned long currentTime = millis() / 1000; // Tiempo en segundos
  StaticJsonDocument<200> jsonDoc; // Crear un documento JSON
  
  jsonDoc["type"] = "INFO"; // Tipo de mensaje
  jsonDoc["src"] = doorName; // Nombre de la puerta
  jsonDoc["desc"] = "Doorbell ringing"; // Descripción del evento
  jsonDoc["data"] = ""; // Datos adicionales (vacío en este caso)
  jsonDoc["time"] = currentTime; // Marca de tiempo
  jsonDoc["cmd"] = "event"; // Comando relacionado
  
  // Asignar el nombre del host basado en el nombre de la puerta
  String topicName = String(doorName) + "_topic"; 
  jsonDoc["hostname"] = topicName; // Nombre del host

  String jsonString;
  serializeJson(jsonDoc, jsonString); // Serializar el documento JSON a una cadena

  Serial.println(jsonString); // Imprimir mensaje en el monitor serial
  client.publish("door/events", jsonString.c_str()); // Publicar el mensaje en el tópico MQTT
}

void sendNotificationToPublisher(const char* message) {
  StaticJsonDocument<200> jsonDoc; // Crear un documento JSON para la notificación
  
  jsonDoc["type"] = "INFO"; // Tipo de mensaje
  jsonDoc["message"] = message; // Mensaje de notificación

  String jsonString;
  serializeJson(jsonDoc, jsonString); // Serializar el documento JSON a una cadena

  // Publicar el mensaje en el tópico de notificaciones
  client.publish("door/notifications", jsonString.c_str());
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Reconectando a MQTT...");
    lcd.print("Reconectando a MQTT");
    if (client.connect("ArduinoClient", mqtt_username, mqtt_password)) {
      Serial.println("Conectado");
      lcd.clear();
      lcd.print("Conectado");
      client.subscribe(mqtt_topic_door1);
      client.subscribe(mqtt_topic_door2);
      client.subscribe(mqtt_topic_door3);
    } else {
      Serial.print("Error al reconectar: ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void controlLED(int ledPin, bool& ledState, unsigned long& ledOnTime) {
  if (ledState && (millis() - ledOnTime >= displayDuration)) { // Si el LED está encendido y han pasado 5 segundos
    ledState = false; // Apagar el LED
  }
  digitalWrite(ledPin, ledState);
}
