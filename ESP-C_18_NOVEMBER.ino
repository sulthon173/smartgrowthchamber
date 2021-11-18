#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "kuda makan sapi";
const char* password = "sapimakankuda";

char *strings[16]; // an array of pointers to the pieces of the above array after strtok()
char *ptr = NULL;
const char* mqtt_server = "34.101.153.146";

long timeIntervalUpdateDataFromServer = 1000;
unsigned long previousTimeUpdateDataFromServer = millis();

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
//Variables

bool mode_status = 1; //mode for actuator, AUTO = 1, MANUAL = 0

bool heater_status = 0;
bool compressor_status = 0;
bool humidifier_status = 0;
bool minipump_status = 0;

float temp_sensor = 0;
float humidity_sensor = 0;
float led_sensor = 0;
float temp_sensor_ambient = 0;
int waterlevel_sensor = 0;

float temp_setpoint = 0;
float humidity_setpoint = 0;
float led_setpoint = 0;

int fan_evap_pwm_value = 73;
unsigned int led_pwm_value = 0;

bool enable_ambient = 1;


//------------------------------------------------------------------------------------------------------------

//LCD
#include "Nextion.h"

#define TEMP 0
#define HUM 1
#define HUM_MAN 2
#define LIGHT 3
#define HOME_MAN 4
#define HOME_AUTO 5
#define COOL 6
#define HEAT 7
#define LIGHT_MAN 8
#define FAN 9

int i, j;
int currentDisplay;
int MODE;

NexButton bAuto_manual           = NexButton(0, 2, "bMode_man");
NexButton bAuto_setting_auto    = NexButton(0, 1, "bSet_auto");
NexButton bManual_auto          = NexButton(1, 1, "bMode_auto");
NexButton bManual_setting_manual  = NexButton(1, 2, "bSet_man");

NexButton bHum_auto_temp  = NexButton(2, 1, "bTemp");
NexButton bHum_auto_light = NexButton(2, 2, "bLight");
NexButton bHum_auto_set   = NexButton(2, 4, "bSet");
NexButton bHum_auto_home  = NexButton(2, 3, "bHome");

NexButton bTemp_auto_hum  = NexButton(3, 1, "bHum");
NexButton bTemp_auto_light = NexButton(3, 2, "bLight");
NexButton bTemp_auto_home = NexButton(3, 3, "bHome");
NexButton bTemp_auto_set  = NexButton(3, 4, "bSet");

NexButton bLight_auto_hum   = NexButton(4, 1, "bHum");
NexButton bLight_auto_temp  = NexButton(4, 2, "bTemp");
NexButton bLight_auto_home  = NexButton(4, 3, "bHome");
NexButton bLight_auto_set  = NexButton(4, 7, "bSet");

NexButton bHum_manual_cool   = NexButton(5, 1, "bCool");
NexButton bHum_manual_heat   = NexButton(5, 2, "bHeat");
NexButton bHum_manual_light  = NexButton(5, 3, "bLight");
NexButton bHum_manual_fan    = NexButton(5, 4, "bFan");
NexButton bHum_manual_home   = NexButton(5, 5, "bHome");

NexButton bCool_manual_hum   = NexButton(6, 1, "bHum");
NexButton bCool_manual_heat   = NexButton(6, 2, "bHeat");
NexButton bCool_manual_light  = NexButton(6, 3, "bLight");
NexButton bCool_manual_fan    = NexButton(6, 4, "bFan");
NexButton bCool_manual_home   = NexButton(6, 5, "bHome");

NexButton bHeat_manual_hum    = NexButton(7, 1, "bHum");
NexButton bHeat_manual_cool   = NexButton(7, 2, "bCool");
NexButton bHeat_manual_light  = NexButton(7, 3, "bLight");
NexButton bHeat_manual_fan    = NexButton(7, 4, "bFan");
NexButton bHeat_manual_home   = NexButton(7, 5, "bHome");

NexButton bLight_manual_hum  = NexButton(8, 1, "bHum");
NexButton bLight_manual_heat  = NexButton(8, 2, "bHeat");
NexButton bLight_manual_cool  = NexButton(8, 3, "bCool");
NexButton bLight_manual_fan   = NexButton(8, 4, "bFan");
NexButton bLight_manual_home  = NexButton(8, 5, "bHome");

NexButton bFan_manual_hum  = NexButton(9, 1, "bHum");
NexButton bFan_manual_heat  = NexButton(9, 2, "bHeat");
NexButton bFan_manual_cool  = NexButton(9, 3, "bCool");
NexButton bFan_manual_light = NexButton(9, 4, "bLight");
NexButton bFan_manual_home  = NexButton(9, 6, "bHome");

NexPage autohomePage  =  NexPage(0, 0, "pHome_Auto");
NexPage manualhomePage = NexPage(1, 0, "pHome_Man");
NexPage autohumPage   = NexPage(2, 0, "pAuto_hum");
NexPage autotempPage = NexPage(3, 0, "pAuto_temp");
NexPage autolightPage = NexPage(4, 0, "pAuto_light");
NexPage manhumPage = NexPage(5, 0, "pMan_hum");
NexPage mancoolPage = NexPage(6, 0, "pMan_cool");
NexPage manheatPage = NexPage(7, 0, "pMan_heat");
NexPage manlightPage = NexPage(8, 0, "pMan_light");
NexPage manfanPage  = NexPage(9, 0, "pMan_fan");

NexPicture pAuto_indhum = NexPicture(0, 8, "pIndhum");
NexPicture pAuto_indcool = NexPicture(0, 9, "pIndcool");
NexPicture pAuto_indheat = NexPicture(0, 10, "pIndheat");

NexPicture pManual_indhum = NexPicture(0, 9, "pIndhum");
NexPicture pManual_indcool = NexPicture(0, 10, "pIndcool");
NexPicture pManual_indheat = NexPicture(0, 11, "pIndheat");

NexPicture pHum_man = NexPicture(5, 6, "pHum");
NexPicture pCool_man = NexPicture(6, 6, "pCool");
NexPicture pHeat_man = NexPicture(7, 6, "pHeat");


NexSlider hLight_auto_slider = NexSlider(4, 4, "hLight");
NexSlider hLight_manual_slider = NexSlider(8, 6, "hLight");
NexSlider hFan_manual_slider = NexSlider(9, 5, "hFan");


NexText tAuto_temp        = NexText(0, 3, "tTemp");
NexText tAuto_hum         = NexText(0, 4, "tHum");
NexText tAuto_light       = NexText(0, 5, "tLight");
NexText tAuto_day         = NexText(0, 6, "tDay");
NexText tAuto_hour        = NexText(0, 7, "tHour");
NexText tAuto_indlight    = NexText(0, 11, "tIndlight");
NexText tAuto_indfan      = NexText(0, 12, "tIndfan");


NexText tManual_temp        = NexText(1, 3, "tTemp");
NexText tManual_hum         = NexText(1, 4, "tHum");
NexText tManual_light       = NexText(1, 5, "tLight");
NexText tManual_indlight    = NexText(1, 12, "tIndlight");
NexText tManual_indfan      = NexText(1, 6, "tIndfan");
NexText tManual_day         = NexText(1, 8, "tDay");
NexText tManual_hour        = NexText(1, 7, "tHour");

NexText tHum_auto_act   = NexText(2, 5, "tAct");
NexText tHum_auto_setpoint  = NexText(2, 6, "tSet");
NexText tNumhum       = NexText(2, 7, "tNumhum");

NexText tLight_auto_act   = NexText(4, 4, "tAct");
NexText tLight_auto_setpoint = NexText(4, 5, "tSet");
NexText tNumlight       = NexText(4, 7, "tNumlight");

NexText tTemp_auto_act    = NexText(3, 5, "tAct");
NexText tTemp_auto_setpoint = NexText(3, 6, "tSet");
NexText tNumtemp         = NexText(3, 7, "tNumtemp");
NexText tLight_manual_setpoint = NexText(8, 7 , "tSet");
NexText tFan_manual_setpoint = NexText(9, 7, "tFan");


NexTouch *nex_listen_list[] = {
  &bAuto_manual, &bAuto_setting_auto,
  &bManual_auto, &bManual_setting_manual,
  &bHum_auto_home, &bHum_auto_light, &bHum_auto_set, &bHum_auto_temp,
  &bTemp_auto_home, &bTemp_auto_hum, &bTemp_auto_light, &bTemp_auto_set,
  &bLight_auto_home, &bLight_auto_hum, &bLight_auto_temp, &bLight_auto_set,
  &bHum_manual_cool, &bHum_manual_fan, &bHum_manual_heat, &bHum_manual_home, &bHum_manual_light,
  &bCool_manual_fan, &bCool_manual_heat, &bCool_manual_home, &bCool_manual_light, &bCool_manual_hum,
  &bHeat_manual_cool, &bHeat_manual_fan, &bHeat_manual_home, &bHeat_manual_light, &bHeat_manual_hum,
  &bLight_manual_cool, &bLight_manual_fan, &bLight_manual_heat, &bLight_manual_home, &bLight_manual_hum,
  &bFan_manual_cool, &bFan_manual_heat, &bFan_manual_home, &bFan_manual_light, &bFan_manual_hum,
  &hLight_manual_slider, &hFan_manual_slider, &hLight_auto_slider,
  &pHum_man, &pCool_man, &pHeat_man,
  NULL
};

//------------------------------------------------------------------------------------------------------------

//Sensor
#include "max6675.h"
int thermoDO = 19;
int thermoCS = 23;
int thermoCLK = 5;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

void readtemp_ambient() {
  temp_sensor_ambient = thermocouple.readCelsius();
   char sString[8];
   dtostrf(led_setpoint, 1, 0, sString);
   client.publish(" esp32/tempambientsens", sString);
}

unsigned long previousTimeTempAmbient = millis();
long timeIntervalTempAmbient = 600000;

//------------------------------------------------------------------------------------------------------------
//Send and Receive Data
#include <esp_now.h>
#include <WiFi.h>
//Must match the receiver structure
typedef struct struct_message {
  bool mode_stat; //mode for actuator, AUTO = 1, MANUAL = 0 espc

  bool heater_stat; //esp
  bool compressor_stat;
  bool humidifier_stat;
  bool minipump_stat;

  float temp_sens;
  float humidity_sens;
  float led_sens;
  float temp_sens_ambient;
  int waterlevel_sens;

  float temp_set;
  float humidity_set;
  float led_set;

  int fan_evap_pwm_val;
  unsigned int led_pwm_val;

} struct_message;

// Create a struct_message to transmit data
struct_message transmitter;
// Create a struct_message to hold incoming data
struct_message receiver_espb;
//Destination Address
uint8_t broadcastAddress_espb[] = {0x84, 0xCC, 0xA8, 0x2C, 0xC4, 0xA0};
unsigned long previousTimeActCallback = millis();
long timeIntervalActCallback = 2000;

unsigned long previousTimeSendData = millis();
long timeIntervalSendData = 50;

unsigned long previousTimeUpdateData = millis();
long timeIntervalUpdateData = 50;

unsigned long previousTimeDisplaySerial = millis();
long timeIntervalDisplaySerial = 1000;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(250000);
  //MQTT
   setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  reconnect();
  
  currentDisplay = HOME_AUTO;
  if (mode_status) {
    autohomePage.show();
    currentDisplay = HOME_AUTO;
  } else {
    manualhomePage.show();
    currentDisplay = HOME_MAN;
  }
  nexInit();
  buttonCallback();
  ActCallback();
}


void loop() {
   if((WiFi.status() != WL_CONNECTED)||(!client.connected())){
    setup_wifi();
    reconnect();
  }
  client.loop();
  
  unsigned long currentTime = millis();

  //LCD
  nexLoop(nex_listen_list);

  if (currentTime - previousTimeActCallback > timeIntervalActCallback) {
    previousTimeActCallback = currentTime;
    ActCallback();
  }
  //Sensor
  if ((currentTime - previousTimeTempAmbient > timeIntervalTempAmbient) || (enable_ambient)) {
    previousTimeTempAmbient = currentTime;
    readtemp_ambient();
    enable_ambient = 0;
  }
  
  if (currentTime - previousTimeDisplaySerial > timeIntervalDisplaySerial) {
    previousTimeDisplaySerial = currentTime;
    displayserial();
  }
}




//MQTT
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
    Serial.println("on");
    heater_status = 1;  
    }
    else if(messageTemp == "0"){
     Serial.println("off");
     heater_status = 0;
    }
  }
  if(String(topic) == "esp32/cooler"){
    Serial.print("Changing cooler to ");
    if(messageTemp == "1"){
      Serial.println("on");
      compressor_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("off");
      compressor_status= 0;
    }
  } 
  if(String(topic) == "esp32/humidifier"){
    Serial.print("Changing humidifier to ");
    if(messageTemp == "1"){
      Serial.println("on");
       humidifier_status = 1;
    }
    else if(messageTemp == "0"){
      Serial.println("off");
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
      autohomePage.show();
    }
    else if(messageTemp == "0"){
      Serial.println("manual");
      mode_status = 0;
      manualhomePage.show();
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
      client.subscribe("esp32/tempambientsens");
    } else {
      
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&receiver_espb, incomingData, sizeof(receiver_espb));
}

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


//LCD
void buttonCallback() {

  bAuto_manual.attachPop(bHomeManualPushCallback, &bAuto_manual);
  bAuto_setting_auto.attachPop(bHumReleaseCallback, &bAuto_setting_auto);

  bManual_auto.attachPop(bHomeAutoPushCallback, &bManual_auto);
  bManual_setting_manual.attachPop(bSetmanhumcallback, &bManual_setting_manual);

  bHum_auto_home.attachPop(bHomeAutoPushCallback, &bHum_auto_home);
  bHum_auto_light.attachPop(bLightReleaseCallback, &bHum_auto_light);
  bHum_auto_set.attachPop(Setcallback, &bHum_auto_set);
  bHum_auto_temp.attachPop(bTempReleaseCallback, &bHum_auto_temp);

  bTemp_auto_home.attachPop(bHomeAutoPushCallback, &bTemp_auto_home);
  bTemp_auto_hum.attachPop(bHumReleaseCallback, &bTemp_auto_hum);
  bTemp_auto_light.attachPop(bLightReleaseCallback, &bTemp_auto_light);
  bTemp_auto_set.attachPop(Setcallback, &bTemp_auto_set);

  bLight_auto_hum.attachPop(bHumReleaseCallback, &bLight_auto_hum);
  bLight_auto_temp.attachPop(bTempReleaseCallback, &bLight_auto_temp);
  bLight_auto_home.attachPop(bHomeAutoPushCallback, &bLight_auto_home);
  bLight_auto_set.attachPop(Setcallback, &bLight_auto_set);

  bHum_manual_cool.attachPop( bSetmancoolcallback, &bHum_manual_cool);
  bHum_manual_heat.attachPop(bSetmanheatcallback , &bHum_manual_heat);
  bHum_manual_light.attachPop(bSetmanlightcallback, &bHum_manual_light);
  bHum_manual_fan.attachPop( bSetmanfancallback , &bHum_manual_fan);
  bHum_manual_home.attachPop(bHomeManualPushCallback, &bHum_manual_home);

  bCool_manual_hum.attachPop( bSetmanhumcallback, &bCool_manual_hum);
  bCool_manual_heat.attachPop( bSetmanheatcallback, &bCool_manual_heat);
  bCool_manual_light.attachPop( bSetmanlightcallback, &bCool_manual_light);
  bCool_manual_fan.attachPop( bSetmanfancallback, &bCool_manual_fan);
  bCool_manual_home.attachPop( bHomeManualPushCallback, &bCool_manual_home);

  bHeat_manual_hum.attachPop( bSetmanhumcallback, &bHeat_manual_hum);
  bHeat_manual_cool.attachPop( bSetmancoolcallback, &bHeat_manual_cool);
  bHeat_manual_light.attachPop( bSetmanlightcallback, &bHeat_manual_light);
  bHeat_manual_fan.attachPop( bSetmanfancallback, &bHeat_manual_fan);
  bHeat_manual_home.attachPop( bHomeManualPushCallback, &bHeat_manual_home);

  bLight_manual_hum.attachPop( bSetmanhumcallback, &bLight_manual_hum);
  bLight_manual_heat.attachPop( bSetmanheatcallback, &bLight_manual_heat);
  bLight_manual_cool.attachPop( bSetmancoolcallback, &bLight_manual_cool);
  bLight_manual_fan.attachPop( bSetmanfancallback, &bLight_manual_fan);
  bLight_manual_home.attachPop( bHomeManualPushCallback, &bLight_manual_home);

  bFan_manual_hum.attachPop( bSetmanhumcallback, &bFan_manual_hum);
  bFan_manual_heat.attachPop( bSetmanheatcallback, &bFan_manual_heat);
  bFan_manual_cool.attachPop( bSetmancoolcallback, &bFan_manual_cool);
  bFan_manual_light.attachPop( bSetmanlightcallback, &bFan_manual_light);
  bFan_manual_home.attachPop( bHomeManualPushCallback, &bFan_manual_home);

  autohomePage.attachPop(bHomeAutoPushCallback, &autohomePage);
  autotempPage.attachPop( bTempReleaseCallback, &autotempPage);
  autohumPage.attachPop(bHumReleaseCallback, &autohumPage);
  autolightPage.attachPop(bLightReleaseCallback, &autolightPage);
  manualhomePage.attachPop( bHomeManualPushCallback , &manualhomePage);
  mancoolPage.attachPop(bSetmancoolcallback, &mancoolPage);
  manheatPage.attachPop(bSetmanheatcallback, &manheatPage);
  manlightPage.attachPop(bSetmanlightcallback, &manlightPage);
  manfanPage.attachPop(bSetmanfancallback, &manfanPage);

  hLight_manual_slider.attachPop(Setcallback);
  hFan_manual_slider.attachPop(Setcallback) ;
  hLight_auto_slider.attachPop(Setcallback) ;
  pHum_man.attachPop(Setcallback);
  pCool_man.attachPop(Setcallback);
  pHeat_man.attachPop(Setcallback);   
}

void bHomeAutoPushCallback(void *ptr) {

  autohomePage.show();
  currentDisplay = HOME_AUTO;
  mode_status = 1;
  ActCallback();
  client.publish("esp32/mode", "1");
}

void bHomeManualPushCallback(void *ptr) {
  manualhomePage.show();
  currentDisplay = HOME_MAN;
  mode_status = 0;
  ActCallback();
  client.publish("esp32/mode", "0");
}

void bTempReleaseCallback(void *ptr) {
  autotempPage.show();
  currentDisplay = TEMP;
  ActCallback();
}

void bHumReleaseCallback(void *ptr) {
  dbSerialPrintln("bHum Clicked");
  autohumPage.show();
  currentDisplay = HUM;
  ActCallback();
}

void bLightReleaseCallback(void *ptr) {
  autolightPage.show();
  currentDisplay = LIGHT;
  ActCallback();
}

void bSetmanhumcallback(void *ptr) {
  manhumPage.show();
  currentDisplay = HUM_MAN;
  ActCallback();
}

void bSetmanheatcallback(void *ptr) {
  manheatPage.show();
  currentDisplay = HEAT;
  ActCallback();
}

void bSetmancoolcallback(void *ptr) {
  mancoolPage.show();
  currentDisplay = COOL;
  ActCallback();
}

void bSetmanlightcallback(void *ptr) {
  manlightPage.show();
  currentDisplay = LIGHT_MAN;
  ActCallback();
}

void bSetmanfancallback(void *ptr) {
  manfanPage.show();
  currentDisplay = FAN;
  ActCallback();
}
void Setcallback(void *ptr) {
  char *buffer2;
  char buffer[5];
  float fBuffer2;
  memset(buffer, 0, 5);
  char sString[8];
  memset(sString, 0, 8);
  if (currentDisplay == HUM) {
    tNumhum.getText(buffer, 5);
    i = 0;
    while (buffer[i] != NULL) {
      i++;
    }
    buffer2 = (char*)malloc ((i + 1) * sizeof (char));
    j = 0;
    while (j < i)
    {
      buffer2[j] = buffer[j];
      j++;
    }
    fBuffer2 = atof(buffer2);


    if ((fBuffer2 > 0.0000001) || (fBuffer2 = 0)) {
      humidity_setpoint = fBuffer2;
      Serial.print("Humidity_setPoint =   ");
      Serial.println(humidity_setpoint);
      tHum_auto_setpoint.setText(buffer);
      tNumhum.setText("");
      dtostrf(humidity_setpoint, 1, 0, sString);
      client.publish("esp32/setpoint_humidity", sString);
    }

  }
  else if (currentDisplay == TEMP) {
    tNumtemp.getText(buffer, 5);
    i = 0;
    while (buffer[i] != NULL) {
      i++;
    }
    buffer2 = (char*)malloc ((i + 1) * sizeof (char));

    j = 0;
    while (j < i)
    {
      buffer2[j] = buffer[j];
      j++;
    }
    fBuffer2 = atof(buffer2);
    temp_setpoint = fBuffer2;
    if ((fBuffer2 > 0.0000001) || (fBuffer2 == 0)) {
      Serial.print("temp_setPoint =   ");
      Serial.println(temp_setpoint);
      tTemp_auto_setpoint.setText(buffer);
      tNumtemp.setText("");
      dtostrf(temp_setpoint, 1, 0, sString);
      client.publish("esp32/setpoint_temperature", sString);
    }
  }
  else if (currentDisplay == LIGHT ) {
    tNumlight.getText(buffer, 5);
    i = 0;
    while (buffer[i] != NULL) {
      i++;
    }
    buffer2 = (char*)malloc ((i + 1) * sizeof (char));
    j = 0;
    while (j < i)
    {
      buffer2[j] = buffer[j];
      j++;
    }
    fBuffer2 = atof(buffer2);


    if ((fBuffer2 > 0.0000001) || (fBuffer2 = 0)) {
      led_setpoint = fBuffer2;
      Serial.print("led_setPoint =   ");
      Serial.println(led_setpoint);
      tLight_auto_setpoint.setText(buffer);
      tNumlight.setText("");
      dtostrf(led_setpoint, 1, 0, sString);
      client.publish("esp32/setpoint_led", sString);
    }
  }
  else if (currentDisplay == HUM_MAN) {
    uint32_t number = 0;
    pHum_man.getPic(&number);
    if (number == 15)
    {
      humidifier_status = 1;
      number = 16;
    }
    else
    {
      humidifier_status = 0;
      number = 15;
    }
    pHum_man.setPic(number);
    if (humidifier_status) {
      client.publish("esp32/humidifier", "1");
    } else {
      client.publish("esp32/humidifier", "0");
    }
  }
  else if (currentDisplay == COOL) {
    uint32_t number = 0;
    dbSerialPrintln("p0PopCallback");
    pCool_man.getPic(&number);
    if (number == 13)
    {
      compressor_status = 1;
      number = 14;
    }
    else
    {
      number = 13;
      compressor_status = 0;
    }
    pCool_man.setPic(number);
    if (compressor_status) {
      client.publish("esp32/cooler", "1");
    } else {
      client.publish("esp32/cooler", "0");
    }

  }
  else if (currentDisplay == HEAT) {
    uint32_t number = 0;
    pHeat_man.getPic(&number);
    if (number == 11)
    {
      heater_status = 1;
      number = 12;
    }
    else
    {
      number = 11;
      heater_status = 0;
    }
    pHeat_man.setPic(number);
    if (heater_status) {
      client.publish("esp32/heater", "1");
    } else {
      client.publish("esp32/heater", "0");
    }
  }
  else if (currentDisplay == LIGHT_MAN) {
    uint32_t number = 0;
    char temp[10] = {0};
    hLight_manual_slider.getValue(&number);
    utoa(number, temp, 10);
    tLight_manual_setpoint.setText(temp);
    led_pwm_value  = number;
    char ledString[8];
    dtostrf(led_pwm_value, 1, 0, ledString);
    client.publish("esp32/led", ledString);
  }
  else if (currentDisplay == FAN) {
    uint32_t number = 0;
    char temp[10] = {0};
    hFan_manual_slider.getValue(&number);
    utoa(number, temp, 10);
    tFan_manual_setpoint.setText(temp);
    fan_evap_pwm_value = number;
    char fanString[8];
    dtostrf(fan_evap_pwm_value, 1, 0, fanString);
    client.publish("esp32/fan", fanString);
  }
}

void ActCallback() {
  char buffer[5];
  char buffer1[5];
  char buffer2[5];
  char buffer3[5];
  char buffer4[5];
  char buffer5[10];
  char buffer6[10];
  char buffer7[10];

  String sBuffer;


  float fBuffer2;
  int   iBuffer2;
  memset(buffer, 0, 5);
  memset(buffer1, 0, 5);
  memset(buffer2, 0, 5);
  memset(buffer3, 0, 5);
  memset(buffer4, 0, 5);
  memset(buffer5, 0, 5);
  memset(buffer6, 0, 5);
  memset(buffer7, 0, 5);


  if (currentDisplay == HUM) {
    fBuffer2 = humidity_sensor;
    sprintf(buffer, "%.1f", fBuffer2);
    tHum_auto_act.setText(buffer);
    itoa(humidity_setpoint, buffer2, 10);
    tHum_auto_setpoint.setText(buffer2);

  }
  else if (currentDisplay == TEMP) {
    fBuffer2 = temp_sensor;
    sprintf(buffer, "%.2f", fBuffer2);
    tTemp_auto_act.setText(buffer);
    itoa(temp_setpoint, buffer2, 10);
    tTemp_auto_setpoint.setText(buffer2);

  }
  else if (currentDisplay == LIGHT) {
    fBuffer2 = led_sensor;
    sprintf(buffer, "%.2f", fBuffer2);
    tLight_auto_act.setText(buffer);
    itoa(led_setpoint, buffer2, 10);
    tLight_auto_setpoint.setText(buffer2);
  }
  else if (currentDisplay == HOME_AUTO) {
    fBuffer2 = humidity_sensor;
    sprintf(buffer, "%.2f", fBuffer2);
    tAuto_hum.setText(buffer);
    fBuffer2 = led_sensor;
    sprintf(buffer1, "%.2f", fBuffer2);
    tAuto_light.setText(buffer1);
    fBuffer2 = temp_sensor;
    sprintf(buffer2, "%.2f", fBuffer2);
    tAuto_temp.setText(buffer2);
    iBuffer2 = led_pwm_value;
    itoa(iBuffer2, buffer3, 10);
    tAuto_indlight.setText(buffer3);
    if (iBuffer2 == 0) {
      tAuto_indlight.Set_background_image_pic(26);
    }
    else {
      tAuto_indlight.Set_background_image_pic(25);
    }
    iBuffer2 = fan_evap_pwm_value;
    itoa(iBuffer2, buffer4, 10);
    tAuto_indfan.setText(buffer4);
    if (iBuffer2 == 0) {
      tAuto_indfan.Set_background_image_pic(24);
    }
    else {
      tAuto_indfan.Set_background_image_pic(23);
    }
    if (humidifier_status == 0) {
      pAuto_indhum.setPic(21);
    } else {
      pAuto_indhum.setPic(22);
    }
    if (compressor_status == 0 ) {
      pAuto_indcool.setPic(17);
    } else {
      pAuto_indcool.setPic(18);
    }
    if (heater_status == 0) {
      pAuto_indheat.setPic(19);
    } else  {
      pAuto_indheat.setPic(20);
    }
  }

  else if (currentDisplay == HOME_MAN) {

    fBuffer2 = humidity_sensor;
    sprintf(buffer, "%.2f", fBuffer2);
    tManual_hum.setText(buffer);
    fBuffer2 = led_sensor;
    sprintf(buffer1, "%.2f", fBuffer2);
    tManual_light.setText(buffer1);
    fBuffer2 = temp_sensor;
    sprintf(buffer2, "%.2f", fBuffer2);
    tManual_temp.setText(buffer2);
    iBuffer2 = led_pwm_value;
    itoa(iBuffer2, buffer3, 10);
    if (iBuffer2 == 0) {
      tManual_indlight.Set_background_image_pic(26);
    }
    else {
      tManual_indlight.Set_background_image_pic(25);
    }
    tManual_indlight.setText(buffer3);
    iBuffer2 = fan_evap_pwm_value;
    itoa(iBuffer2, buffer4, 10);
    if (iBuffer2 == 0) {
      tManual_indfan.Set_background_image_pic(24);
    }
    else {
      tManual_indfan.Set_background_image_pic(23);
    }

    tManual_indfan.setText(buffer4);

    if (humidifier_status == 0) {
      pManual_indhum.setPic(21);
    } else {
      pManual_indhum.setPic(22);
    }
    if (compressor_status == 0 ) {
      pManual_indcool.setPic(17);
    } else {
      pManual_indcool.setPic(18);
    }
    if (heater_status == 0) {
      pManual_indheat.setPic(19);
    } else  {
      pManual_indheat.setPic(20);
    }
  }
  else if (currentDisplay == HUM_MAN) {
    int buffer = humidifier_status;
    if (buffer == 1) {
      pHum_man.setPic(16);
    }
    else {
      pHum_man.setPic(15);
    }
  }
  else if (currentDisplay == COOL) {
    int   buffer = compressor_status ;
    if (buffer == 1) {
      pCool_man.setPic(14);
    }
    else {
      pCool_man.setPic(13);
    }
  }
  else if (currentDisplay == HEAT) {
    int buffer = heater_status;
    if ( buffer == 1) {
      pHeat_man.setPic(12);
    }
    else {
      pHeat_man.setPic(11);
    }
  }
  else if (currentDisplay == LIGHT_MAN) {
    int iBuffer = led_pwm_value;
    hLight_manual_slider.setValue(iBuffer);
    itoa(led_pwm_value, buffer, 10);
    tLight_manual_setpoint.setText(buffer);
  }
  else if (currentDisplay == FAN) {
    int iBuffer = fan_evap_pwm_value;
    hFan_manual_slider.setValue(iBuffer);
    itoa(fan_evap_pwm_value, buffer, 10);
    tFan_manual_setpoint.setText(buffer);
  }
}
