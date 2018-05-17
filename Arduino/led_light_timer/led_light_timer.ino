#include "Common.h"
#include <ESP8266WiFi.h>
#include <NTP.h>
#include "Ambient.h"

extern "C" {
#include "user_interface.h"
}
 
const char* ssid = "0024A5B34C9D";
const char* password = "k2ueeia979ys7";
long sleepTime = 1;

byte operatingMode = INIT_MODE;

unsigned int channelId = 3811;
const char* writeKey = "15ee6fea5150a859";
WiFiClient client;
Ambient ambient;

void setup() {
  Serial.begin(115200);
  operatingMode = INIT_MODE;
  WiFi.mode(WIFI_STA);
  pinMode(LIGHT_SW, OUTPUT);
  digitalWrite(LIGHT_SW, LOW);
}

void loop() {
  controllLight();
  goSleep();
  wakeUp();
}

void goSleep() {
  if(operatingMode == LIGHT_OFF_MODE) {
#ifdef USE_TEST_PRINTOUT
    Serial.println("DeepSleeping...");
#endif // USE_TEST_PRINTOUT
    if(sleepTime > MAX_DEEP_SLEEP_SEC) {
      sleepTime = MAX_DEEP_SLEEP_SEC;
    }    
    //DEEP SLEEPモード突入命令
    //1:μ秒での復帰までのタイマー時間設定  2:復帰するきっかけの設定（モード設定）
    ESP.deepSleep(sleepTime * 1000 * 1000 , WAKE_RF_DEFAULT);  
    //deepsleepモード移行までのダミー命令
    delay(1000);      
  }
  else { //LIGHT_ON_MODE
#ifdef USE_TEST_PRINTOUT
    Serial.println("LightSleeping...");
#endif // USE_TEST_PRINTOUT
    wifi_set_sleep_type(LIGHT_SLEEP_T);
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    int lsleepTime;
    while(sleepTime>0) {
      if(sleepTime > MAX_LIGHT_SLEEP_SEC) {
        lsleepTime = MAX_LIGHT_SLEEP_SEC;
        sleepTime -= MAX_LIGHT_SLEEP_SEC;
      }
      else {
        lsleepTime = sleepTime;
        sleepTime = 0;
      }
      WiFi.forceSleepBegin(lsleepTime * 1000000L); //In uS. Must be same length as your delay
      delay(lsleepTime * 1000); //Hang out at 15mA for sleepTime(seconds)    
    }
  }
}

void wakeUp() {
  if(operatingMode == LIGHT_OFF_MODE) {
    ; //nothing
  }
  else { //LIGHT_ON_MODE
    WiFi.mode(WIFI_STA);    
#ifdef USE_TEST_PRINTOUT
    Serial.println("Awake LightSleep!");
#endif // USE_TEST_PRINTOUT
  }
}

void controllLight() {
  bool LedIsOn;
  long RestTimeSec;

  connectWifi();
  time_t t_jst = getJst();
  LedIsOn = judgeLightOn(hour(t_jst), START_TIME_HOUR, END_TIME_HOUR);
  RestTimeSec = calcRestTimeSec(t_jst, START_TIME_HOUR, END_TIME_HOUR);

  ambient.begin(channelId, writeKey, &client);

  if(LedIsOn) {
#ifdef USE_TEST_PRINTOUT
    Serial.println("Light on");
#endif // USE_TEST_PRINTOUT

    ambient.set(1, 1);// データーがint型かfloat型であれば、直接セットすることができます。
    ambient.send();
    
    digitalWrite(LIGHT_SW, HIGH);   // Light on
    operatingMode = LIGHT_ON_MODE;
    sleepTime = RestTimeSec;
  }
  else {
#ifdef USE_TEST_PRINTOUT
    Serial.println("Light off");
#endif // USE_TEST_PRINTOUT

    ambient.set(1, 0);// データーがint型かfloat型であれば、直接セットすることができます。
    ambient.send();

    digitalWrite(LIGHT_SW, LOW);   // Light off
    operatingMode = LIGHT_OFF_MODE;
    sleepTime = RestTimeSec;
  }

}

time_t getJst(){
  ntp_begin(2390);  

  time_t n = now();
  time_t t;
  t = localtime(n, 9); // JST(+9)

#ifdef USE_TEST_PRINTOUT
  char s[20];
  const char* format = "%04d-%02d-%02d %02d:%02d:%02d";
  sprintf(s, format, year(t), month(t), day(t), hour(t), minute(t), second(t));
  Serial.println("JST : ");
  Serial.println(s);
#endif //USE_TEST_PRINTOUT

  return t;
}

void connectWifi() {
  wifi_set_sleep_type(NONE_SLEEP_T);
  WiFi.begin(ssid, password); //Connect to local Wifi
  while(WiFi.status() != WL_CONNECTED)
  {
#ifdef USE_TEST_PRINTOUT
    Serial.print(".");
#endif //USE_TEST_PRINTOUT
    delay(500);
  }
#ifdef USE_TEST_PRINTOUT
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
#endif //USE_TEST_PRINTOUT
}


bool judgeLightOn(int now_hour, int start_hour, int end_hour) {
  bool ret = false;  

  if(end_hour < start_hour) { //ex. 13:00(start) -> 2:00(end)
    now_hour = ((now_hour + 24) - start_hour) % 24; //ex. 12:00->23:00(off),13:00->0:00(on),23:00->10:00(on),1:00->12:00(on),3:00->14:00(off)
    end_hour = ((end_hour + 24) - start_hour) % 24; //ex. 2:00->13:00
    start_hour = 0; //ex. 13:00->0:00
  }

  if(now_hour >= start_hour) {
    if(now_hour <= end_hour) {
      ret = true;
    }
  }

#ifdef USE_TEST_PRINTOUT
  const char* format = "%d";
  char s[20];
  sprintf(s, format, now_hour);
  Serial.print("NowTime : ");
  Serial.println(s);
  sprintf(s, format, start_hour);
  Serial.print("StartTime : ");
  Serial.println(s);
  sprintf(s, format, end_hour);
  Serial.print("EndTime : ");
  Serial.println(s);
#endif //USE_TEST_PRINTOUT

  return ret;
}

long calcRestTimeSec(time_t t_jst, int start_hour, int end_hour) {
  bool lightOn = false;  
  int now_hour = hour(t_jst);
  long ret;

  //calculation for day over light on time (ex. 13:00(start) -> 2:00(end))
  now_hour = ((now_hour + 24) - start_hour) % 24; //ex. 12:00->23:00(off),13:00->0:00(on),23:00->10:00(on),1:00->12:00(on),3:00->14:00(off)
  end_hour = ((end_hour + 24) - start_hour) % 24; //ex. 2:00->13:00
  start_hour = 0; //ex. 13:00->0:00

  if(now_hour >= start_hour) {
    if(now_hour <= end_hour) {
      lightOn = true;
    }
  }

  long nowSec = (now_hour*60 + minute(t_jst)) * 60 + second(t_jst);
  long endSec;
  if(lightOn) {
    endSec = end_hour*60*60;
  }
  else {
    endSec = 24*60*60;
  }  
  ret = endSec - nowSec;
  if(ret==0)ret = 1; //proof of severe timing

#ifdef USE_TEST_PRINTOUT
  const char* format = "%ld";
  char s[20];
  sprintf(s, format, nowSec);
  Serial.print("nowSec : ");
  Serial.println(s);
  sprintf(s, format, endSec);
  Serial.print("endSec : ");
  Serial.println(s);
  sprintf(s, format, ret);
  Serial.print("ret : ");
  Serial.println(s);
#endif //USE_TEST_PRINTOUT

  return ret;

}
