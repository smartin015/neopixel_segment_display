#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define HOSTNAME "countdown2"

const char* ssid = "robotoverlords";
const char* password = "oakdale43";
const char* mqtt_server = "192.168.0.8";

WiFiClient espClient;
PubSubClient client(espClient);

#define LED_PIN    D6


#define NSEG 13
#define NCHAR 10

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(NSEG*NCHAR, LED_PIN, NEO_GRB + NEO_KHZ800);

uint32_t con = strip.Color(0,0,128);
uint32_t coff = strip.Color(0,0,0);

const uint16_t nummap[10] = {
  0b1111110111111,
  0b1001010010100,
  0b1110111110111,
  0b1111011110111,
  0b1001011111101,
  0b1111011101111,
  0b1111111101111,
  0b1001010010111,
  0b1111111111111,
  0b1111011111111,
};

const uint16_t amap[26] = {
  0b1011111111111,
  0b0111101111011,
  0b1110100101111,
  0b0111110111011,
  0b1110111101111,
  0b0010101101111,
  0b1111110101111,
  0b1011111111101,
  0b1110001000111,
  0b1101110010111,
  0b1011101111101,
  0b1110100101001,
  0b1011111111000,
  0b1011101100000,
  0b1111110111111,
  0b0010101111011,
  0b1001011111111,
  0b0010111101000,
  0b1111001001111,
  0b1110101101000,
  0b1111110111101,
  0b0101110111101,
  0b0101110100000,
  0b0001101011000,
  0b0100001011000,
  0b1110101010111,
};

const uint16_t puncmap[3] = {
  0b0100000000000, // .
  0b0010000101001, // !
  0b0100001010110, // ?
};


void callback(char* topic, byte* payload, unsigned int length) {
  for (int i=0; i< NCHAR; i++) {
	  char c = (i >= length) ? ' ' : payload[i];
    uint16_t m = getMask(c);
    writeSegment(i, m);
  }
  strip.show();
}
 
 
void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("segdisplay")) {
  Serial.println("connected");
  // ... and subscribe to topic
  String topic = "/" HOSTNAME;
  client.subscribe(topic.c_str());
  Serial.println("Subscribed to " + topic);
 } else {
  Serial.print("failed, rc=");
  Serial.print(client.state());
  Serial.println(" try again in 5 seconds");
  // Wait 5 seconds before retrying
  delay(5000);
  }
 }
}
 
void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  Serial.begin(9600);

 	Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.hostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
	Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

uint16_t getMask(char c) {
  if (isalpha(c)) {
    return amap[tolower(c) - 'a'];
  } else if (isdigit(c)) {
    return nummap[c - '0'];
  } else if (c == '.') {
    return puncmap[0];
  } else if (c == '!') {
    return puncmap[1];
  } else if (c == '?') {
    return puncmap[2];
  }
  return 0;
}

void writeSegment(int offs, uint16_t mask) {
  for (int i = 0; i < NSEG; i++) {
    if ((mask >> i) & 0x01) {
      strip.setPixelColor(i+(offs*NSEG), con);
    } else {
      strip.setPixelColor(i+(offs*NSEG), coff);
    }
  }
}

char* test = "0123456789abcdefghijklmnopqrstuvwxyz.?!$";

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
