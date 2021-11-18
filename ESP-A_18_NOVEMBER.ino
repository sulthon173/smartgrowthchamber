#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
// Replace the next variables with your SSID/Password combination
const char* ssid     = "kuda makan sapi";
const char* password = "sapimakankuda";

// Add your MQTT Broker IP address, example:
const char* mqtt_server = "34.101.153.146";
unsigned long previousTimeSenddatamqtt =millis();  
long timeIntervalSenddatamqtt = 1000;

//Variables
bool mode_status ; //mode for actuator, AUTO = 1, MANUAL = 0

bool heater_status = 0;
bool compressor_status = 0;
bool humidifier_status = 0;
bool minipump_status = 0;
bool temp_or_humidity_turn = 1; //0 for humidity turn, 1 for temperature turn
bool heater_status_temp ;
bool compressor_status_temp ;
bool humidifier_status_temp  ;

float temp_sensor = 0;
float humidity_sensor = 0;
float led_sensor = 0;
float temp_sensor_ambient = 0;
int waterlevel_sensor = 0;

float temp_setpoint = 0;
float humidity_setpoint = 0;
float led_setpoint= 0;

int fan_evap_pwm_value = 73;
unsigned int led_pwm_value = 0;
int fan_evap_pwm_value_temp;
unsigned int led_pwm_value_temp;
 
float temp_hysterisis = 1.5;
float humidity_hyterisis = 5;
float led_hysterisis = 10;

//------------------------------------------------------------------------------------------------------------


//Temperature and Humidity Control
  #include <RBDdimmer.h>
  #define PIN_HEATER 18
  #define PIN_COMPRESSOR 22
  #define PIN_FAN_EVAP 21
  #define ZC 19
  #define PIN_HUMIDIFIER 14
  #define PIN_FAN_HUMIDIFIER 32
  dimmerLamp dimmer_Heater(PIN_HEATER, ZC);
  dimmerLamp dimmer_Fan_Evap(PIN_FAN_EVAP,ZC);
  dimmerLamp dimmer_Compressor(PIN_COMPRESSOR,ZC);
  unsigned long previousTimeTempHumidControl = millis();
  long timeIntervalTempHumidControl = 50;

  unsigned long previousTimeSwitching = millis();
  long timeIntervalSwitching = 180000;

void tempandhumid(){

     heater_status_temp = heater_status;
     compressor_status_temp = compressor_status;
     humidifier_status_temp  = humidifier_status;
     fan_evap_pwm_value_temp = fan_evap_pwm_value;
    
    unsigned long currentTime2 = millis();
    
    dimmer_Fan_Evap.setPower(fan_evap_pwm_value);
    dimmer_Heater.setPower(40);   
    dimmer_Compressor.setPower(67);

    if(mode_status){
      if(temp_or_humidity_turn){
        if(temp_setpoint < temp_sensor_ambient){
            if(temp_sensor > temp_setpoint + temp_hysterisis){
            dimmer_Heater.setState(OFF);
            dimmer_Compressor.setState(ON);
            digitalWrite(PIN_HUMIDIFIER,OFF); //active low
            digitalWrite(PIN_FAN_HUMIDIFIER,OFF); //active low
            heater_status = 0;
            compressor_status = 1;
            humidifier_status = 1;
            fan_evap_pwm_value = 73;
          }else if(temp_sensor < temp_setpoint - temp_hysterisis){
            dimmer_Heater.setState(OFF);
            dimmer_Compressor.setState(OFF);
            digitalWrite(PIN_HUMIDIFIER,ON); //active low
            digitalWrite(PIN_FAN_HUMIDIFIER,ON); //active low
            heater_status = 0;
            compressor_status = 0;
            humidifier_status = 0;
            fan_evap_pwm_value = 73;
          }else{
            if((currentTime2 - previousTimeSwitching > timeIntervalSwitching) && (temp_sensor < temp_setpoint)){
               temp_or_humidity_turn = 0;
               previousTimeSwitching = currentTime2;
            }
          }
        } else{
            if(temp_sensor > temp_setpoint + temp_hysterisis){
            dimmer_Heater.setState(OFF);
            dimmer_Compressor.setState(OFF);
            digitalWrite(PIN_HUMIDIFIER,ON); //active low
            digitalWrite(PIN_FAN_HUMIDIFIER,ON); //active low
            heater_status = 0;
            compressor_status = 0;
            humidifier_status = 0;
            fan_evap_pwm_value = 73;
          }else if(temp_sensor < temp_setpoint - temp_hysterisis){
            dimmer_Heater.setState(ON);
            dimmer_Compressor.setState(OFF);
            digitalWrite(PIN_HUMIDIFIER,OFF); //active low
            digitalWrite(PIN_FAN_HUMIDIFIER,OFF); //active low
            heater_status = 1;
            compressor_status = 0;
            humidifier_status = 1;
            fan_evap_pwm_value = 73;
          }else{
            if((currentTime2 - previousTimeSwitching > timeIntervalSwitching) && (temp_sensor > temp_setpoint)){
               temp_or_humidity_turn = 0;
               previousTimeSwitching = currentTime2;
            }
          }
        }

     }else {
        if(humidity_sensor > humidity_setpoint + humidity_hyterisis){
          digitalWrite(PIN_HUMIDIFIER,ON); //active low
          digitalWrite(PIN_FAN_HUMIDIFIER,ON); //active low
          dimmer_Heater.setState(ON);
          dimmer_Compressor.setState(ON);
          humidifier_status = 0;
          fan_evap_pwm_value = 73;
          heater_status = 1;
          compressor_status = 1;
        }else if(humidity_sensor < humidity_setpoint - humidity_hyterisis ){
          digitalWrite(PIN_HUMIDIFIER,OFF); //active low
          digitalWrite(PIN_FAN_HUMIDIFIER,OFF); //active low
          dimmer_Heater.setState(OFF);
          dimmer_Compressor.setState(OFF);
          humidifier_status = 1;
          fan_evap_pwm_value = 73;
          heater_status = 0;
          compressor_status = 0;
        }else{
          if(currentTime2 - previousTimeSwitching > timeIntervalSwitching){
               temp_or_humidity_turn = 1;
               previousTimeSwitching = currentTime2;
            }
        }
          
        }  
  }else {
    
    if(heater_status){
      dimmer_Heater.setState(ON);
    }else{
      dimmer_Heater.setState(OFF);
    }

    if(compressor_status){
      dimmer_Compressor.setState(ON);
    }else {
      dimmer_Compressor.setState(OFF);
    }

    dimmer_Fan_Evap.setPower(fan_evap_pwm_value);

    if(humidifier_status){
      digitalWrite(PIN_HUMIDIFIER,OFF); //active low
      digitalWrite(PIN_FAN_HUMIDIFIER,OFF); //active low
    }else {
      digitalWrite(PIN_HUMIDIFIER,ON);//active low
      digitalWrite(PIN_FAN_HUMIDIFIER,ON);//active low
    }
  }      
}

//------------------------------------------------------------------------------------------------------------

//LED Control
  #define PIN_LED_1 5
  #define PIN_LED_2 16
  #define PIN_LED_3 17
  
  #define LED_PWM_RESOLUTION 8
  #define PWM_FREQ 5000
  #define LED_CHANNEL 0

  unsigned long previousTimeLedControl = millis();
  long timeIntervalLedControl = 100;

//LED Control
void ledcontrol(){
  if(mode_status){
    unsigned int ibuffer=led_pwm_value;
    ledcWrite(LED_CHANNEL,map(ibuffer,0,100,0,255));
    if(led_sensor < led_setpoint - led_hysterisis){
      ibuffer += 1;
      ibuffer = constrain(ibuffer,0,100);
    }else if(led_sensor > led_setpoint + led_hysterisis){
      ibuffer -= 1;
      ibuffer = constrain(ibuffer,0,100);
    }else{
      
    }
    led_pwm_value = ibuffer;
    
  }
   
  else{
    ledcWrite(LED_CHANNEL,map(led_pwm_value,0,100,0,255));
    }  
  }

//------------------------------------------------------------------------------------------------------------

//Minipump control
  #define PIN_MINIPUMP 12
  unsigned long previousTimeMinipumpControl = millis();
  long timeIntervalMinipumpControl = 50;

  //Minipump Control
void minipumpcontrol(){ 
    if (waterlevel_sensor>3900){
      digitalWrite(PIN_MINIPUMP,LOW); //active low
    }
    else if  (waterlevel_sensor<3500){      
      digitalWrite(PIN_MINIPUMP,HIGH); // active low
      }
}

//------------------------------------------------------------------------------------------------------------

void displayserial(){
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

  Serial.print("temp_humidity_turns : ");
  Serial.println(temp_or_humidity_turn);
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
  

////------------------------------------------------------------------------------------------------------------

  //Temperature and Humidity Control
  pinMode(PIN_HUMIDIFIER, OUTPUT);
  pinMode(PIN_FAN_HUMIDIFIER, OUTPUT);
  dimmer_Heater.begin(NORMAL_MODE, OFF);
  dimmer_Heater.setPower(60);
  dimmer_Fan_Evap.begin(NORMAL_MODE, ON);
  dimmer_Fan_Evap.setPower(74);
  dimmer_Compressor.begin(NORMAL_MODE, OFF);
  dimmer_Compressor.setPower(67);

  //LED Control
  // configure LED PWM functionalitites
  ledcSetup(LED_CHANNEL, PWM_FREQ, LED_PWM_RESOLUTION);
  
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(PIN_LED_1, LED_CHANNEL);
  ledcAttachPin(PIN_LED_2, LED_CHANNEL);
  ledcAttachPin(PIN_LED_3, LED_CHANNEL);

  //Minipump Control
  pinMode(PIN_MINIPUMP, OUTPUT);
  digitalWrite(PIN_MINIPUMP,HIGH);// turn off minipump  

}

//------------------------------------------------------------------------------------------------------------

void loop() {
    if((WiFi.status() != WL_CONNECTED)||(!client.connected())){
    setup_wifi();
    reconnect();
  }
  client.loop();
  
     
  unsigned long currentTime = millis();

  if(currentTime - previousTimeTempHumidControl > timeIntervalTempHumidControl){
    previousTimeTempHumidControl = currentTime;
    tempandhumid();    
  }

  if(currentTime - previousTimeLedControl > timeIntervalLedControl){
    previousTimeLedControl = currentTime;
    ledcontrol();
  }

  if(currentTime - previousTimeMinipumpControl > timeIntervalMinipumpControl){
    previousTimeMinipumpControl = currentTime;
    minipumpcontrol();
  }

  if(currentTime - previousTimeSenddatamqtt > timeIntervalSenddatamqtt){
    previousTimeSenddatamqtt = currentTime;
    Senddatamqtt();
  }

    if(currentTime - previousTimeDisplaySerial > timeIntervalDisplaySerial){
    previousTimeDisplaySerial = currentTime;
    displayserial();
  }
}

//------------------------------------------------------------------------------------------------------------
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

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  if (String(topic) == "esp32/heater") {
    Serial.print("Changing heater to ");
    if(messageTemp == "1"){
      Serial.println("on");
        heater_status  = 1;
      
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
      compressor_status = 0;
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
      humidifier_status = 0;
    }

  }
  if(String(topic) == "esp32/led"){
    Serial.print("Changing led to ");
    led_pwm_value = messageTemp.toInt();
    Serial.println(led_pwm_value);
  }
  if(String(topic) == "esp32/fan"){
    Serial.print("Changing fan to ");
    fan_evap_pwm_value = messageTemp.toInt();
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
       mode_status= 0;
    } 

  }
   if(String(topic) == "esp32/setpoint_humidity"){
    Serial.print("Changing setpoint humidity to ");
     humidity_setpoint =  messageTemp.toInt();
  }
  if(String(topic) == "esp32/setpoint_temperature"){
    Serial.print("Changing temperature to ");
    temp_setpoint  = messageTemp.toInt();
  }
  if(String(topic) == "esp32/setpoint_led"){
    Serial.print("Changing led to ");
    led_setpoint =messageTemp.toInt();
  }
    if(String(topic) == "esp32/watersens"){
    Serial.print("Changing led to ");
    waterlevel_sensor =messageTemp.toInt();
  }
   if(String(topic) == "esp32/watersens"){
    Serial.print("Changing led to ");
    waterlevel_sensor =messageTemp.toInt();
  }
  if(String(topic) == "tempambientsens"){
    Serial.print("Changing led to ");
    temp_sensor_ambient =messageTemp.toInt();
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



void Senddatamqtt(){
    
    if (heater_status_temp != heater_status ) {
      char heaterString[2];
      itoa(heater_status,heaterString,10);
      client.publish("esp32/heater", heaterString);
    }
    if (compressor_status_temp != compressor_status ) {
      char compressorString[2];
      itoa(compressor_status,compressorString,10);
      client.publish("esp32/cooler", compressorString);
    }
    if (humidifier_status_temp != humidifier_status ) {
      char humidifierString[2];
      itoa(humidifier_status,humidifierString,10);
      client.publish("esp32/humidifier", humidifierString);
    }
    if (fan_evap_pwm_value != fan_evap_pwm_value_temp ) {
      char fan_evap_pwm_valueString[2];
      dtostrf(led_pwm_value, 1, 0, fan_evap_pwm_valueString);
       client.publish("esp32/fan", fan_evap_pwm_valueString); 
    }
    if (led_pwm_value != led_pwm_value_temp ) {
      char ledString[8];
      dtostrf(led_pwm_value, 1, 0, ledString);
      client.publish("esp32/led", ledString); 
    }
 }
