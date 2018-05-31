#include "Common.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <NTP.h>
#include "HwTimer1.h"
#include "SdWriter.h"
#include "Ambient.h"
extern "C" {
#include "user_interface.h"
}

const char* ssid = "XXXX";
const char* password = "XXXX";

ESP8266WebServer server(80);

const int led = 13;

#define ADV_TEMP10 (800)
#define ADV_TEMP30 (402)

unsigned int channelId = 0000;
const char* writeKey = "XXXX";
WiFiClient wifiAmbientClient;
Ambient ambient;
int ambientFlg = -1;
int ADC_Value = 0;
typedef enum {
      eTEMP_LESSTHAN_10 = 0
    , eTEMP_10_30
    , eTEMP_MORETHAN_30
} eTEMP_SECTION;
const float temp_slope[3] =
{
-25.54347826, // ...10℃
-16.44171779, // 10℃...30℃
-10.57553957 // 30℃...
};
const float temp_intercept[3] =
{
1007.173913, // ...10℃
887.0306748, // 10℃...30℃
713.9784173 // 30℃...
};

void ICACHE_RAM_ATTR TimerRoutine();
void display();

bool checkLightOn(int now_hour, int start_hour, int end_hour) {
  bool ret = false;  

  if(end_hour < start_hour) { //ex. 13:00(start) -> 2:00(end)
    now_hour = ((now_hour + 24) - start_hour) % 24; //ex. 12:00->23:00(off),13:00->0:00(on),23:00->10:00(on),1:00->12:00(on),3:00->14:00(off)
    end_hour = ((end_hour + 24) - start_hour) % 24; //ex. 2:00->13:00
    start_hour = 0; //ex. 13:00->0:00
  }

  const char* format = "%d";
  char s[20];
  sprintf(s, format, now_hour);
  DEBUG_PRINT("NowTime : ");
  DEBUG_PRINTLN(s);
  sprintf(s, format, start_hour);
  DEBUG_PRINT("StartTime : ");
  DEBUG_PRINTLN(s);
  sprintf(s, format, end_hour);
  DEBUG_PRINT("EndTime : ");
  DEBUG_PRINTLN(s);
  
  if(now_hour >= start_hour) {
    if(now_hour <= end_hour) {
      ret = true;
    }
  }

  return ret;
}

void AmbientSend()
{
    if(ambientFlg > 0) {
      DEBUG_PRINTLN("AmbientSend");
      int temp_section = eTEMP_10_30;
      if(ADC_Value > ADV_TEMP10) {
        temp_section = eTEMP_LESSTHAN_10;
      }
      else if(ADC_Value < ADV_TEMP30) {
        temp_section = eTEMP_MORETHAN_30;
      }
      float fADC_Value = (float)ADC_Value;
      float temp = (fADC_Value - temp_intercept[temp_section]) / temp_slope[temp_section];
      ambient.set(1, temp);// if data type is int or float, I can input raw value.
      ambient.send();
      ambientFlg = -1;
    }
}

void ICACHE_RAM_ATTR TimerRoutine()
{
    DEBUG_PRINTLN("RoutineTimerWork");

    //exe adc
    ADC_Value = 0;
    ADC_Value = system_adc_read();
    ambientFlg = 1;

    //debug out adc result(0-1024)
    DEBUG_PRINTLN("=======ANALOG " + String(ADC_Value) + "ANALOG ");

    String strJst = "";
    time_t t_jst;
    t_jst = getJst(&strJst);
    SD_writeLine(&strJst);

    bool needLightOn = checkLightOn(hour(t_jst), START_TIME_HOUR, END_TIME_HOUR);
    String strMessage = "";
    if(needLightOn) {      
      strMessage = "Light On";
      DEBUG_PRINTLN("Light On");
      SD_writeLine(&strMessage);
      digitalWrite(PIN_ACOUT, HIGH);   // AC OUT on
    }
    else {
      strMessage = "Light Off";
      DEBUG_PRINTLN("Light Off");
      SD_writeLine(&strMessage);
      digitalWrite(PIN_ACOUT, LOW);   // AC OUT on
    }

}

void handleRoot() {
  digitalWrite(led, 1);
  String strJst = "";
  time_t t_jst;
  t_jst = getJst(&strJst);
  server.send(200, "text/plain", strJst);
  //server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
  //display();

}

void handleNotFound(){
  if(SD_existCard() && loadFromSdCard(server.uri())) return;
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
File uploadFile;

void handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    if(SD_existFile((char *)upload.filename.c_str())) SD_remove((char *)upload.filename.c_str());
    uploadFile = SD_openWriteFile((char *)upload.filename.c_str());
    DEBUG_PRINT("Upload: START, filename: "); DEBUG_PRINTLN(upload.filename);
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //if(uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    SD_writeFile(upload.buf, upload.currentSize);
    DEBUG_PRINT("Upload: WRITE, Bytes: "); DEBUG_PRINTLN(upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(uploadFile) SD_closeWriteFile();
    DEBUG_PRINT("Upload: END, Size: "); DEBUG_PRINTLN(upload.totalSize);

    server.sendHeader("Location","/uploadok.htm");      // Redirect the client to the success page
    server.send(303);  
  }
}

bool loadFromSdCard(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";

  //File dataFile = SD.open(path.c_str());
  File dataFile = SD_openFile(path.c_str());
  if(dataFile.isDirectory()){
    path += "/index.htm";
    dataType = "text/html";
    //dataFile = SD.open(path.c_str());
    dataFile = SD_openFile(path.c_str());
  }

  if (!dataFile) {
    return false;
  }

  if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    DEBUG_PRINTLN("Sent less data than expected!");
  }

  //dataFile.close();
  SD_closeFile();

  return true;
}

void returnOK() {
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
}

void printDirectory() {
  if(!server.hasArg("dir")) return returnFail("BAD ARGS");
  String path = server.arg("dir");
  if(path != "/" && !SD_existFile((char *)path.c_str())) return returnFail("BAD PATH");
  File dir = SD_openFile((char *)path.c_str());
  path = String();
  if(!dir.isDirectory()){
    dir.close();
    return returnFail("NOT DIR");
  }
  dir.rewindDirectory();
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/json", "");
  WiFiClient client = server.client();

  server.sendContent("[");
  for (int cnt = 0; true; ++cnt) {
    File entry = dir.openNextFile();
    if (!entry)
    break;

    String output;
    if (cnt > 0)
      output = ',';

    output += "{\"type\":\"";
    output += (entry.isDirectory()) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += entry.name();
    output += "\"";
    output += "}";
    server.sendContent(output);
    entry.close();
 }
 server.sendContent("]");
 SD_closeFile();
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  //server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/edit", HTTP_POST, [](){ returnOK(); }, handleFileUpload);
  server.on("/list", HTTP_GET, printDirectory);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  // 2390 はローカルのUDPポート。空いている番号なら何番でもいいです。
  ntp_begin(2390);

  // NTPサーバを変更 (デフォルト: ntp.nict.jp)
  //setTimeServer("s2csntp.miz.nao.ac.jp");

  // NTP同期間隔を変更 (デフォルト: 300秒)
  //setSyncInterval(10);

  pinMode(PIN_ACOUT, OUTPUT);     // Initialize the AC OUT pin as an output
  digitalWrite(PIN_ACOUT, LOW);   // AC OUT on

  SD_init();

  ambientFlg = -1;
  //WiFiClient client = server.client();
  ambient.begin(channelId, writeKey, &wifiAmbientClient);

  TimerRoutine();
  HwTimer1_Start(UPDATE_INTERVAL_SEC, TimerRoutine);

}

time_t getJst(String *pStrJst){

  time_t n = now();
  time_t t;

  char s[20];
  const char* format = "%04d-%02d-%02d %02d:%02d:%02d";

  // JST
  t = localtime(n, 9);
  sprintf(s, format, year(t), month(t), day(t), hour(t), minute(t), second(t));
  DEBUG_PRINTLN("JST : ");
  DEBUG_PRINTLN(s);

  *(pStrJst) = String(s);

  return t;
}

void display(){

  time_t n = now();
  time_t t;

  char s[20];
  const char* format = "%04d-%02d-%02d %02d:%02d:%02d";

  // JST
  t = localtime(n, 9);
  sprintf(s, format, year(t), month(t), day(t), hour(t), minute(t), second(t));
  DEBUG_PRINT("JST : ");
  DEBUG_PRINTLN(s);

  // UTC
  t = localtime(n, 0);
  sprintf(s, format, year(t), month(t), day(t), hour(t), minute(t), second(t));
  DEBUG_PRINT("UTC : ");
  DEBUG_PRINTLN(s);
}

void loop(void){
  server.handleClient();
  AmbientSend();

}
