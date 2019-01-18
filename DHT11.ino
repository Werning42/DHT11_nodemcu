#include <DHT.h>

#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <DHT_U.h>

#include "ThingSpeak.h"

#define DHTPIN 2

#define DHTTYPE DHT11
DHT dht(DHTPIN,DHTTYPE);

#include <LiquidCrystal_I2C.h>


#include <Wire.h>

#include <ESP8266WiFi.h>
LiquidCrystal_I2C lcd(0x27,16,2);

WiFiClient client;

ESP8266WiFiMulti WiFiMulti;

String apiKey = "K595ZYNHEG2L7J5T"; // Enter your Write API key from ThingSpeak

const char* ssid = "Pyramide"; // replace with your wifi ssid and wpa2 key
const char* pass = "friendzone";
const char* server = "api.thingspeak.com";
unsigned long myChannelNumber = 659353;
const char * myWriteAPIKey = "708OO7OBZSOGN8N6";

void setup() {

Serial.begin(115200);

WiFi.mode(WIFI_STA);
WiFiMulti.addAP("Pyramide", "friendzone");

pinMode(D5, OUTPUT);
pinMode(D6, OUTPUT);
pinMode(D7, OUTPUT);

lcd.init();
lcd.backlight();
dht.begin();


WiFi.begin(ssid, pass);

while (WiFi.status() != WL_CONNECTED)
{
delay(500);
Serial.print(".");
}
ThingSpeak.begin(client);
}

void loop(){

delay(5000);
float t = dht.readTemperature();
float h = dht.readHumidity();
if(isnan(h) || isnan(t)){
return;
}

int temp = (int) t;
int humi = (int) h;

lcd.setCursor(0,0);
lcd.print(" Temp: ");
lcd.print(temp);
lcd.print("C ");
lcd.setCursor(0,1);
lcd.print(" Humi: ");
lcd.print(humi);
lcd.println("% ");

ThingSpeak.setField(1,int(t));
ThingSpeak.setField(2,int(h));
ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

if (t <= 10) {
digitalWrite(D7, HIGH);
digitalWrite(D6, LOW);
digitalWrite(D5, LOW);
}
if (t > 10 && t <= 20) {
digitalWrite(D7, LOW);
digitalWrite(D6, HIGH);
digitalWrite(D5, LOW);
}
if (t > 20) {
digitalWrite(D7, LOW);
digitalWrite(D6, LOW);
digitalWrite(D5, HIGH);
}

String dataDHT = "temperature="+String(t)+"&humidite="+String(h);

if((WiFiMulti.run() == WL_CONNECTED)) { // Si le wifi est connecté

Serial.println("Connexion wifi initialisée !");

HTTPClient http; // préparation d'une requète HTTP au serveur

http.begin("http://vps596416.ovh.net/DHT11/store_temp.php"); // connexion au serveur

http.addHeader("Content-Type", "application/json"); // envoi d'un entête pour préciser notre format de données : json

String data = "{\"temperature\":\"" + String(temp) + "\",\"humidite\":\"" + String(humi) + "\"}"; // préparation du json {"temperature":"22","humidite":"55"}

int httpCode = http.POST(data); // envoi les données et récupère un code de retour

if(httpCode == 200 ) { // si le code de retour est bon (200)
Serial.println("Requête POST acceptée !");
} else { // si le code de retour n'est pas bon (différent de 200)
Serial.print("POST : Echec, erreur : ");
Serial.println(http.errorToString(httpCode).c_str());
}
http.end(); // on ferme la connexion au serveur
} else {
Serial.println("Echec connexion wifi");
}

if (client.connect(server,80)){
Serial.println("Requête Think acceptée !");
String postStr = apiKey;
postStr +="&field1=";
postStr += String(t);
postStr +="&field2=";
postStr += String(h);
postStr += "\r\n\r\n";

client.print("POST /update HTTP/1.1n");
client.print("Host: api.thingspeak.comn");
client.print("Connection: closen");
client.print("X-THINGSPEAKAPIKEY: "+apiKey+"n");
client.print("Content-Type: application/x-www-form-urlencodedn");
client.print("Content-Length: ");
client.print(postStr.length());
client.print(postStr);

}
client.stop();


// thingspeak needs minimum 15 sec delay between updates, i've set it to 30 seconds
delay(10000);
} 
