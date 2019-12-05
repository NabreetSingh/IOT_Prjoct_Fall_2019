#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <Wire.h>
#include <SPI.h>
#include <PubSubClient.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager



//Thingsboard device access token
#define TOKEN "up3eEWqVtuv4sPge51kn"

char thingsboardServer[] = "192.168.0.26"; //Server's ip address

WiFiClient wifiClient; //get cardentials of new wifi connection

#define DS18B20_PIN   12       // DS18B20 data pin is connected to ESP8266 GPIO12 (NodeMCU D6)
 
int   raw_temp;
char *temp = "000.0000 C";

PubSubClient client(wifiClient); //Liberary for MQTT connection
 
int status = WL_IDLE_STATUS;
unsigned long lastSend;
 
void setup(void)
{
 
  Serial.begin(9600);
  delay(1000);
  
  Wire.begin(4, 0);           // set I2C pins [SDA = GPIO4 (D2), SCL = GPIO0 (D3)], default clock is 100kHz

   WiFiManager wifiManager;
   wifiManager.autoConnect(" Team -9 AutoConnectAP");
  delay(1000);

  delay(1000);
  client.setServer( thingsboardServer, 1883 );
  lastSend = 0;

  void send_data();

  delay(2000);
  ESP.deepSleep(10e6);
  delay(10000);
}

void loop()
{
  
}
void send_data()
{
  if ( !client.connected() ) {
    reconnect();
    }
    
  if(ds18b20_read(&raw_temp))
  {
    float acc_temp = float(raw_temp)/16;
    Serial.printf("Temperature = %8.4f°C\r\n", (float)raw_temp / 16);
    String payload = "{";
  payload += "\"Temperature\":"; payload += acc_temp; 
  payload += "}";
  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  client.publish( "v1/devices/me/telemetry", attributes );
  Serial.println( attributes );   
  }
  else 
  {
    Serial.println("Communication Error!");
  }

}


void reconnect() {
    Serial.print("Connecting to ThingsBoard node ...");
    // Attempt to connect (clientId, username, password)
    if ( client.connect("ESP8266 Device", TOKEN, NULL) ) {
      Serial.println( "[DONE]" );
    } else {
      Serial.print( "[FAILED] [ rc = " );
      Serial.print( client.state() );
      Serial.println( " : retrying in 5 seconds]" );
      // Wait 5 seconds before retrying
      delay( 5000 );
    }
  }


 
bool ds18b20_start()
{
  bool ret = 0;
  digitalWrite(DS18B20_PIN, LOW);     // Send reset pulse to the DS18B20 sensor
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(500);             // Wait 500 us
  pinMode(DS18B20_PIN, INPUT_PULLUP);
  delayMicroseconds(100);             //wait to read the DS18B20 sensor response
  if (!digitalRead(DS18B20_PIN)) {
    ret = 1;                          // DS18B20 sensor is present
    delayMicroseconds(400);           // Wait 400 us
  }
  return(ret);
}
 
void ds18b20_write_bit(bool value)
{
  digitalWrite(DS18B20_PIN, LOW);
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(2);
  digitalWrite(DS18B20_PIN, value);
  delayMicroseconds(80);
  pinMode(DS18B20_PIN, INPUT_PULLUP);
  delayMicroseconds(2);
}
 
void ds18b20_write_byte(byte value)
{
  byte i;
  for(i = 0; i < 8; i++)
    ds18b20_write_bit(bitRead(value, i));
}
 
bool ds18b20_read_bit(void)
{
  bool value;
  digitalWrite(DS18B20_PIN, LOW);
  pinMode(DS18B20_PIN, OUTPUT);
  delayMicroseconds(2);
  pinMode(DS18B20_PIN, INPUT_PULLUP);
  delayMicroseconds(5);
  value = digitalRead(DS18B20_PIN);
  delayMicroseconds(100);
  return value;
}
 
byte ds18b20_read_byte(void)
{
  byte i, value;
  for(i = 0; i  <8; i++)
    bitWrite(value, i, ds18b20_read_bit());
  return value;
}
 
bool ds18b20_read(int *raw_temp_value)
{
  if (!ds18b20_start())                     // Send start pulse
    return(0);
  ds18b20_write_byte(0xCC);                 // Send skip ROM command
  ds18b20_write_byte(0x44);                 // Send start conversion command
  while(ds18b20_read_byte() == 0);          // Wait for conversion complete
  if (!ds18b20_start())                     // Send start pulse
    return(0);                              // Return 0 if error
  ds18b20_write_byte(0xCC);                 // Send skip ROM command
  ds18b20_write_byte(0xBE);                 // Send read command
  *raw_temp_value = ds18b20_read_byte();    // Read temperature LSB byte and store it on raw_temp_value LSB byte
  *raw_temp_value |= (unsigned int)(ds18b20_read_byte() << 8);     // Read temperature MSB byte and store it on raw_temp_value MSB byte
  return(1);                                // OK --> return 1
}
