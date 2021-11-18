#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char* ssid     = "kuda makan sapi";
const char* password = "sapimakankuda";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "34.101.153.146";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

 //Variables
bool mode_status = 1; //mode for actuator, AUTO = 1, MANUAL = 0

bool heater_status = 0;
bool compressor_status = 0 ;
bool humidifier_status= 0 ;
bool minipump_status= 0 ;

float temp_sensor=0 ;
float humidity_sensor=0;
float led_sensor =0;
float temp_sensor_ambient=0;
int waterlevel_sensor=0;

float temp_setpoint = 0;
float humidity_setpoint = 0;
float led_setpoint = 0;

int fan_evap_pwm_value=73;
unsigned int led_pwm_value=0;

//------------------------------------------------------------------------------------------------------------

//Sensor
#include <BH1750.h>
#include <Adafruit_SHT31.h>
const int waterlvl = 34;

Adafruit_SHT31 sht31 = Adafruit_SHT31(); //Create an Adafruit SHT31 object
BH1750 lightMeter(0x23);

//Main Function Sensor Reading
void readsensor() {

  float temporary_led;
  if (lightMeter.measurementReady()) {
    temporary_led = lightMeter.readLightLevel();
    if((int(temporary_led) == -1) || (int(temporary_led) == -2)){
    }else{
      led_sensor = temporary_led*1.9952 + 8.3766;
    }
  }

  String test = "nan";
  float temporary_temp = sht31.readTemperature();
  float temporary_humid = sht31.readHumidity();
  
  if(String(temporary_temp) == test){
  }else{
     temp_sensor = temporary_temp;
  }

  if(String(temporary_humid) == test){
  }else{
     humidity_sensor = temporary_humid;
  }
  
  int value   = analogRead(waterlvl);// check water level at humidifier tank
  waterlevel_sensor = value; // check water level at humidifier tank

   // MQTT
   char ledsensString[8];
   dtostrf(led_sensor, 1,0, ledsensString);
   client.publish("esp32/ledsens", ledsensString);

   char tempsensString[8];
   dtostrf(temp_sensor, 1,0, tempsensString);
   client.publish("esp32/tempsens", tempsensString);

   char humsensString[8];
   dtostrf(humidity_sensor, 1,0, humsensString);
   client.publish("esp32/humsens", humsensString);

   char watersensString[8];
   itoa(waterlevel_sensor,watersensString,10);
   client.publish("esp32/watersens", watersensString);
   
}


unsigned long previousTimeReadSensor = millis();
long timeIntervalReadSensor = 1000;

//------------------------------------------------------------------------------------------------------------


void displayserial() {

  Serial.print("Mode (Auto=1, Manual=0) : ");
  Serial.println(mode_status);
  Serial.println();
  
  Serial.print("heater_status : ");
  Serial.println(heater_status);
  Serial.print("compressor_status : ");
  Serial.println(compressor_status);
  Serial.print("humidifier_status : ");
  Serial.println(humidifier_status);
  Serial.print("minipump_status : ");
  Serial.println(minipump_status);
  Serial.println();
  
  Serial.print("temp_sensor : ");
  Serial.println(temp_sensor);
  Serial.print("humidity_sensor : ");
  Serial.println(humidity_sensor);
  Serial.print("led_sensor  : ");
  Serial.println(led_sensor);
  Serial.print("waterlevel_sensor : ");
  Serial.println(waterlevel_sensor);
  Serial.print("temp_sensor_ambient : ");
  Serial.println(temp_sensor_ambient);
  Serial.println();

  Serial.print("temp_setpoint : ");
  Serial.println(temp_setpoint);
  Serial.print("humidity_setpoint : ");
  Serial.println(humidity_setpoint);
  Serial.print("led_setpoint  : ");
  Serial.println(led_setpoint);
  Serial.println();

  Serial.print("fan_evap_pwm_value  : ");
  Serial.println(fan_evap_pwm_value);
  Serial.print("led_pwm_value : ");
  Serial.println(led_pwm_value);
  Serial.println();


  Serial.println("---------------------------------");
  Serial.println(); 
}

unsigned long previousTimeDisplaySerial = millis();
long timeIntervalDisplaySerial = 1000;

//------------------------------------------------------------------------------------------------------------
void setup() {
  Serial.begin(250000);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  
  
  //Sensor
  Wire.begin();
  sht31.begin(0x44); //Aktivasi SHTx31 dengan address 0x44
  lightMeter.begin(BH1750::CONTINUOUS_LOW_RES_MODE);
}
//------------------------------------------------------------------------------------------------------------

void loop() {
if((WiFi.status() != WL_CONNECTED)||(!client.connected())){
    setup_wifi();
    reconnect();
  }
  client.loop();

  unsigned long currentTime = millis();
    if(currentTime - previousTimeReadSensor > timeIntervalReadSensor){
      previousTimeReadSensor = currentTime;
      readsensor();
    }

    if(currentTime - previousTimeDisplaySerial > timeIntervalDisplaySerial){
       previousTimeDisplaySerial = currentTime;
       displayserial();
    }
}


void setup_wifi() {
    delay(10);
// We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
  
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
    }
    
void callback(char* topic, byte* message, unsigned int length) {
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(". Message: ");
    String messageTemp;
    
    for (int i = 0; i < length; i++) {
     Serial.print((char)message[i]);
     messageTemp += (char)message[i];
    }
    Serial.println();
   
    if (String(topic) == "esp32/heater") {
     Serial.print("Changing heater to ");
     if(messageTemp == "1"){
      Serial.println(" heater on");
       heater_status = 1;
     }
    else if(messageTemp == "0"){
      Serial.println("heater off");
      heater_status = 0;
    }
  }
  if(String(topic) == "esp32/cooler"){
    Serial.print("Changing cooler to ");
    if(messageTemp == "1"){
      Serial.println("cooler on");
      compressor_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("cooler off");
      compressor_status= 0;
    }
  } 
  if(String(topic) == "esp32/humidifier"){
    Serial.print("Changing humidifier to ");
    if(messageTemp == "1"){
      Serial.println("humidifier on");
       humidifier_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("humidier off");
      humidifier_status= 0;
    }
  }
  if(String(topic) == "esp32/led"){
    Serial.print("Changing led to ");
    led_pwm_value= messageTemp.toInt();
    Serial.println(led_pwm_value);
  }
  if(String(topic) == "esp32/fan"){
    Serial.print("Changing fan to ");
    fan_evap_pwm_value  = messageTemp.toInt();
    Serial.println(fan_evap_pwm_value); 
  }
  if(String(topic) == "esp32/mode"){
    Serial.print("Changing mode to ");
    if(messageTemp == "1"){
      Serial.println("automatic");
      mode_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("manual");
      mode_status = 0;
    } 
  }
   if(String(topic) == "esp32/setpoint_humidity"){
    Serial.print("Changing setpoint humidity to ");
     humidity_setpoint= messageTemp.toInt();
  }
  if(String(topic) == "esp32/setpoint_temperature"){
    Serial.print("Changing temperature to ");
    temp_setpoint  =messageTemp.toInt();
  }
  if(String(topic) == "esp32/setpoint_led"){
    Serial.print("Changing led to ");
    led_setpoint =messageTemp.toInt();
  }
    if(String(topic) == "esp32/watersens"){
    Serial.print("Changing waterlevel to ");
    waterlevel_sensor =messageTemp.toInt();
  }
  
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected( )) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    WiFi.mode(WIFI_STA);
    if (client.connect("ESPsgc")) {

      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/heater");
      client.subscribe("esp32/cooler");
      client.subscribe("esp32/humidifier");
      client.subscribe("esp32/led");
      client.subscribe("esp32/fan");
      client.subscribe("esp32/mode");
      client.subscribe("esp32/setpoint_humidity");
      client.subscribe("esp32/setpoint_temperature");
      client.subscribe("esp32/setpoint_light");
      client.subscribe("esp32/ledsens");
      client.subscribe("esp32/tempsens");
      client.subscribe("esp32/humsens");
      client.subscribe("esp32/watersens");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
