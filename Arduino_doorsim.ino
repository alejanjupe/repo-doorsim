#include <WiFi.h>
#include <PubSubClient.h>

// Configuración de WiFi
const char* ssid = "Fablab_Torino";
const char* password = "Fablab.Torino!";

// Configuración de MQTT
const char* mqtt_server = "172.26.34.36";
const char* mqtt_topic_door1 = "Door1_topic";
const char* mqtt_username = "tu_usuario";
const char* mqtt_password = "tu_contraseña";

WiFiClient espClient;
PubSubClient client(espClient);

// Configuración del botón
const int buttonPin = 2; // Pin donde está conectado el botón
bool lastButtonState = HIGH; // Estado anterior del botón

void setup() {
  Serial.begin(9600);

  // Conexión a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a WiFi...");
  }
  Serial.println("Conectado a WiFi");

  // Conexión a MQTT con un cliente ID único
  client.setServer(mqtt_server, 1883);
  
  // Generar un ID único para el cliente
  String clientId = "ArduinoClient_" + String(random(0xffff), HEX);
  
  while (!client.connected()) {
    Serial.println("Conectando a MQTT...");
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Conectado a MQTT");
    } else {
      Serial.print("Error al conectar: ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  // Inicializar el pin del botón
  pinMode(buttonPin, INPUT_PULLUP); // Configurar el pin del botón como entrada con pull-up
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Leer el estado del botón
  bool buttonState = digitalRead(buttonPin);
  if (buttonState == LOW && lastButtonState == HIGH) {
    // El botón fue presionado
    if (client.connected()) {
      client.publish(mqtt_topic_door1, "abrir");
      Serial.println("Mensaje enviado: abrir");
    }
    delay(1000); // Esperar un segundo para evitar múltiples envíos
  }
  lastButtonState = buttonState; // Actualizar el estado anterior del botón
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Reconectando a MQTT...");
    
    // Generar un nuevo ID único para reconexión
    String clientId = "ArduinoClient_" + String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Conectado");
    } else {
      Serial.print("Error al reconectar: ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

