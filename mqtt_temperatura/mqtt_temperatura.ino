#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN D4     // Especifica el núnero de pin al que se conecta el sensor DHT

#define wifi_ssid "villaface"
#define wifi_password "*****"

#define mqtt_server "192.168.1.42"
#define mqtt_port 1883
#define mqtt_user "openhabian"
#define mqtt_password "openhabian"     

#define humidity_topic "topicName/humidity"         //Cambia el nombre del topic a tu gusto
#define temperature_topic "topicName/temperature"

// Selecciona el tipo de sensor conectado
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

WiFiClient espClient;
PubSubClient client;
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setClient(espClient);
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
    dht.begin();
}

void setup_wifi() {
    //WiFi.softAPdisconnect();
    //WiFi.disconnect();
    //WiFi.mode(WIFI_STA);
    //delay(100);
    delay(10);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(wifi_ssid);

    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
    // Bucle hasta conseguir la reconexión
    while (!client.connected()) {
        // Intento de conexión
        Serial.print(WiFi.status()+ " Attempting MQTT connection...");
        
        // Si utilizas usuario y contraseña, cambia la siguiente línea por esta
       if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
       // if (client.connect("ESP8266Client")) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            
            // Espera 5 segundos para un nuevo intento
            delay(5000);
        }
    }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
    return newValue < prevValue - maxDiff || newValue > prevValue + maxDiff;
}

long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;

// Rutina para la recepción de mensajes MQTT. No usada para el sensor de temperatura y humedad
void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        char receivedChar = (char)payload[i];
        Serial.print(receivedChar);
        //if (receivedChar == '0')
        // digitalWrite(in_led, LOW);
        //if (receivedChar == '1')
        // digitalWrite(in_led, HIGH);
    }
    Serial.println();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    long now = millis();
    
    // Nueva lectura de Temperatura y Humedad cada 10 segundos
    if (now - lastMsg > 10000) {
        lastMsg = now;
        float newHum = dht.readHumidity();
        float newTemp = dht.readTemperature(false);
        
        // Si la nueva lectura de temperatura es diferente de la anterior envía el nuevo valor
        if (checkBound(newTemp, temp, diff)) {
            temp = newTemp;
            Serial.print("New temperature:");
            Serial.println(String(temp).c_str());
            client.publish(temperature_topic, String(temp).c_str(), true);
        }
        
        // Si la nueva lectura de humedad es diferente de la anterior envía el nuevo valor
        if (checkBound(newHum, hum, diff)) {
            hum = newHum;
            Serial.print("New humidity:");
            Serial.println(String(hum).c_str());
            client.publish(humidity_topic, String(hum).c_str(), true);
        }
    }
}
