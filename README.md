This project uses Arduino boards connected to WiFi to implement a smart doorbell system. The system consists of a doorbell located at the exterior door that, when pressed, activates a light inside the company. Communication between the two devices is carried out using the MQTT protocol, with Mosquitto as the server.

System Operation
Button Press: When the doorbell button is pressed, the exterior Arduino board sends a message via MQTT.
Message Reception: The Arduino board located inside the company receives this message and activates a relay that turns on a light.
Notifications: In addition to turning on the light, the system sends notifications to a central system to log the event.

Project Components
Arduino Boards: Two boards, one on the exterior and another on the interior, connected to the WiFi network.
MQTT: Utilization of an MQTT server for communication between the boards.
Relay: A relay connected to the interior board that controls the light.
Control Code: The source code implements the logic for connecting to WiFi, subscribing to MQTT topics, and controlling the light through the relay.
