#include "Common.h"
#include "SdWriter.h"

#include <SPI.h>
//#include <SD.h>

SdFat SD;

const char* logfile = LOGFILE_NAME;

// SD pin
#define SD_CS (4)

#define CSV_DELIM ','

static File s_myFile;
File writeFile;
File csvFile;

static bool hasSD = false;

int csvReadInt32(File* file, int32_t* num, char delim);
int csvReadInt16(File* file, int16_t* num, char delim);
int csvReadText(File* file, char* str, size_t size, char delim);
int csvReadDouble(File* file, double* num, char delim);
int csvReadFloat(File* file, float* num, char delim);


void SD_init()
{
    DEBUG_PRINT("Initializing SD card...");

    if (!SD.begin(SD_CS)) {
      DEBUG_PRINTLN("initialization failed!");
      return;
    }
    hasSD = true;
    DEBUG_PRINTLN("initialization done.");        
}

bool SD_existCard()
{
  return hasSD;
}

void SD_testWrite()
{
    s_myFile = SD.open("testtest.txt", FILE_WRITE);

    if (s_myFile) {
        DEBUG_PRINT("Writing to test.txt...");
        s_myFile.println("testing 1, 2, 3.");
        s_myFile.close();
        DEBUG_PRINT("done.");
    } else {
        DEBUG_PRINTLN("error opening test.txt");
    }
}

void SD_writeLine(String *pStrWriteLine)
{
    s_myFile = SD.open(logfile, FILE_WRITE);

    if (s_myFile) {
        DEBUG_PRINT("Writing to ");
        DEBUG_PRINT(logfile);
        DEBUG_PRINT("...");
        s_myFile.println(*pStrWriteLine);
        s_myFile.close();
        DEBUG_PRINTLN("done.");
    } else {
        DEBUG_PRINTLN("error opening file");
    }
}

File SD_openFile(const char *filepath)
{
    DEBUG_PRINTLN("SD_openFile");
    s_myFile = SD.open(filepath);
    return s_myFile;      
}

void SD_closeFile()
{
    DEBUG_PRINTLN("SD_closeFile");
    if (s_myFile) {
        s_myFile.close();
    }
}

bool SD_existFile(char *filename)
{
    DEBUG_PRINTLN("SD_existFile");
    return SD.exists(filename);
}

void SD_remove(char *filename)
{
    DEBUG_PRINTLN("SD_remove");
    SD.remove(filename);
}

File SD_openWriteFile(const char *filepath)
{
    DEBUG_PRINTLN("SD_openWriteFile");
    writeFile = SD.open(filepath, FILE_WRITE);
    return writeFile;      
}

void SD_closeWriteFile()
{
    DEBUG_PRINTLN("SD_closeWriteFile");
    writeFile.close();
}

void SD_writeFile(const uint8_t *buf, size_t size)
{
    writeFile.write(buf, size);
}

int16_t SD_countUpCsv()
{
  int16_t ret = -1;

  if(SD.exists(CSVFILE_NAME)) {
    // Open the file.
    csvFile = SD.open(CSVFILE_NAME, FILE_WRITE);
    if (!csvFile) {
      DEBUG_PRINTLN("csv write file open failed");
      return ret;
    }
    DEBUG_PRINTLN("csv write open");
    // Rewind the file for read.
    csvFile.seek(0);
  }
  else {
    // Create the file.
    csvFile = SD.open(CSVFILE_NAME, FILE_WRITE);
    if (!csvFile) {
      DEBUG_PRINTLN("csv write file open failed");
      return ret;
    }

    DEBUG_PRINTLN("csv write initial open");
    // Write initial data.
    csvFile.seek(0);
    csvFile.print(F(
      "1,23.20,20.70,57.60,79.50,01:08:14,23.06.16\r\n"
    ));
    // Rewind the file for read.
    csvFile.seek(0);
  }

 
 // Read the file and print fields.
  int16_t tcalc=-1; 
  float t1, t2, h1, h2;
  // Must be dim 9 to allow for zero byte.
  char timeS[9], dateS[9];
  while (csvFile.available()) {
    if (csvReadInt16(&csvFile, &tcalc, CSV_DELIM) != CSV_DELIM
      || csvReadFloat(&csvFile, &t1, CSV_DELIM) != CSV_DELIM
      || csvReadFloat(&csvFile, &t2, CSV_DELIM) != CSV_DELIM
      || csvReadFloat(&csvFile, &h1, CSV_DELIM) != CSV_DELIM
      || csvReadFloat(&csvFile, &h2, CSV_DELIM) != CSV_DELIM
      || csvReadText(&csvFile, timeS, sizeof(timeS), CSV_DELIM) != CSV_DELIM
      || csvReadText(&csvFile, dateS, sizeof(dateS), CSV_DELIM) != '\n') {
      DEBUG_PRINTLN("read end");
      break;            
    }
    DEBUG_PRINTLN(tcalc);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(t1);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(t2);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(h1);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(h2);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(timeS);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(dateS);
  }

  // Write next data.
  csvFile.seek(0);
  csvFile.print(String(tcalc+1)
  + F(
    ",23.20,20.70,57.60,79.50,01:08:14,23.06.16\r\n"
  ));

  csvFile.close();

  ret = tcalc;

  return ret;
}

void SD_initCsv()
{
  if(SD.exists(CSVFILE_NAME)) {
    // Open the file.
    csvFile = SD.open(CSVFILE_NAME, FILE_WRITE);
    if (!csvFile) {
      DEBUG_PRINTLN("csv write file open failed");
      return;
    }
    DEBUG_PRINTLN("csv write open");
    // Rewind the file for read.
    csvFile.seek(0);
  } 

  // Over Write data
  // Write initial data.
  csvFile.seek(0);
  csvFile.print(F(
    "1,23.20,20.70,57.60,79.50,01:08:14,23.06.16\r\n"
  ));
  
  csvFile.close();

}


int16_t SD_writeCsv(bool clear)
{
  int16_t ret = -1;

  if(SD.exists(CSVFILE_NAME)) {
    // Open the file.
    csvFile = SD.open(CSVFILE_NAME, FILE_WRITE);
    if (!csvFile) {
      DEBUG_PRINTLN("csv write file open failed");
      return ret;
    }
    DEBUG_PRINTLN("csv write open");
    // Rewind the file for read.
    csvFile.seek(0);
  }
  else {
    // Create the file.
    csvFile = SD.open(CSVFILE_NAME, FILE_WRITE);
    if (!csvFile) {
      DEBUG_PRINTLN("csv write file open failed");
      return ret;
    }

    DEBUG_PRINTLN("csv write initial open");
    // Write initial data.
    csvFile.seek(0);
    csvFile.print(F(
      "1,23.20,20.70,57.60,79.50,01:08:14,23.06.16\r\n"
    ));
    // Rewind the file for read.
    csvFile.seek(0);
  }

 
 // Read the file and print fields.
  int16_t tcalc=-1; 
  float t1, t2, h1, h2;
  // Must be dim 9 to allow for zero byte.
  char timeS[9], dateS[9];
  while (csvFile.available()) {
    if (csvReadInt16(&csvFile, &tcalc, CSV_DELIM) != CSV_DELIM
      || csvReadFloat(&csvFile, &t1, CSV_DELIM) != CSV_DELIM
      || csvReadFloat(&csvFile, &t2, CSV_DELIM) != CSV_DELIM
      || csvReadFloat(&csvFile, &h1, CSV_DELIM) != CSV_DELIM
      || csvReadFloat(&csvFile, &h2, CSV_DELIM) != CSV_DELIM
      || csvReadText(&csvFile, timeS, sizeof(timeS), CSV_DELIM) != CSV_DELIM
      || csvReadText(&csvFile, dateS, sizeof(dateS), CSV_DELIM) != '\n') {
      DEBUG_PRINTLN("read error");
      int ch;
      int nr = 0;
      // print part of file after error.
      break;            
    }
    DEBUG_PRINTLN(tcalc);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(t1);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(t2);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(h1);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(h2);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(timeS);
    DEBUG_PRINTLN(CSV_DELIM);
    DEBUG_PRINTLN(dateS);
  }

  // Over Write data
  if(clear) {
    // Write initial data.
    csvFile.seek(0);
    csvFile.print(F(
      "1,23.20,20.70,57.60,79.50,01:08:14,23.06.16\r\n"
    ));
  }
  else {
    // Write next data.
    csvFile.seek(0);
    csvFile.print(String(tcalc+1)
    + F(
      ",23.20,20.70,57.60,79.50,01:08:14,23.06.16\r\n"
    ));
  }
  
  csvFile.close();

  ret = tcalc;

  return ret;
}

int csvReadText(File* file, char* str, size_t size, char delim) {
  char ch;
  int rtn;
  size_t n = 0;
  while (true) {
    // check for EOF
    if (!file->available()) {
      rtn = 0;
      break;
    }
    if (file->read(&ch, 1) != 1) {
      // read error
      rtn = -1;
      break;
    }
    // Delete CR.
    if (ch == '\r') {
      continue;
    }
    if (ch == delim || ch == '\n') {
      rtn = ch;
      break;
    }
    if ((n + 1) >= size) {
      // string too long
      rtn = -2;
      n--;
      break;
    }
    str[n++] = ch;
  }
  str[n] = '\0';
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadInt32(File* file, int32_t* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtol(buf, &ptr, 10);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadInt16(File* file, int16_t* num, char delim) {
  int32_t tmp;
  int rtn = csvReadInt32(file, &tmp, delim);
  if (rtn < 0) return rtn;
  if (tmp < INT_MIN || tmp > INT_MAX) return -5;
  *num = tmp;
  return rtn;
}
//------------------------------------------------------------------------------
int csvReadDouble(File* file, double* num, char delim) {
  char buf[20];
  char* ptr;
  int rtn = csvReadText(file, buf, sizeof(buf), delim);
  if (rtn < 0) return rtn;
  *num = strtod(buf, &ptr);
  if (buf == ptr) return -3;
  while(isspace(*ptr)) ptr++;
  return *ptr == 0 ? rtn : -4;
}
//------------------------------------------------------------------------------
int csvReadFloat(File* file, float* num, char delim) {
  double tmp;
  int rtn = csvReadDouble(file, &tmp, delim);
  if (rtn < 0)return rtn;
  // could test for too large.
  *num = tmp;
  return rtn;
}

/*
static bool hasSD = false;
File uploadFile;


void returnOK() {
  server.send(200, "text/plain", "");
}

void returnFail(String msg) {
  server.send(500, "text/plain", msg + "\r\n");
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

  File dataFile = SD.open(path.c_str());
  if(dataFile.isDirectory()){
    path += "/index.htm";
    dataType = "text/html";
    dataFile = SD.open(path.c_str());
  }

  if (!dataFile)
    return false;

  if (server.hasArg("download")) dataType = "application/octet-stream";

  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
    DBG_OUTPUT_PORT.println("Sent less data than expected!");
  }

  dataFile.close();
  return true;
}

void SD_handleFileUpload(){
  if(server.uri() != "/edit") return;
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    if(SD.exists((char *)upload.filename.c_str())) SD.remove((char *)upload.filename.c_str());
    uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
    DBG_OUTPUT_PORT.print("Upload: START, filename: "); DBG_OUTPUT_PORT.println(upload.filename);
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: "); DBG_OUTPUT_PORT.println(upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(uploadFile) uploadFile.close();
    DBG_OUTPUT_PORT.print("Upload: END, Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void deleteRecursive(String path){
  File file = SD.open((char *)path.c_str());
  if(!file.isDirectory()){
    file.close();
    SD.remove((char *)path.c_str());
    return;
  }

  file.rewindDirectory();
  while(true) {
    File entry = file.openNextFile();
    if (!entry) break;
    String entryPath = path + "/" +entry.name();
    if(entry.isDirectory()){
      entry.close();
      deleteRecursive(entryPath);
    } else {
      entry.close();
      SD.remove((char *)entryPath.c_str());
    }
    yield();
  }

  SD.rmdir((char *)path.c_str());
  file.close();
}

void SD_handleDelete(){
  if(server.args() == 0) return returnFail("BAD ARGS");
  String path = server.arg(0);
  if(path == "/" || !SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }
  deleteRecursive(path);
  returnOK();
}

void SD_handleCreate(){
  if(server.args() == 0) return returnFail("BAD ARGS");
  String path = server.arg(0);
  if(path == "/" || SD.exists((char *)path.c_str())) {
    returnFail("BAD PATH");
    return;
  }

  if(path.indexOf('.') > 0){
    File file = SD.open((char *)path.c_str(), FILE_WRITE);
    if(file){
      file.write((const char *)0);
      file.close();
    }
  } else {
    SD.mkdir((char *)path.c_str());
  }
  returnOK();
}

void SD_printDirectory() {
  if(!server.hasArg("dir")) return returnFail("BAD ARGS");
  String path = server.arg("dir");
  if(path != "/" && !SD.exists((char *)path.c_str())) return returnFail("BAD PATH");
  File dir = SD.open((char *)path.c_str());
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
 dir.close();
}

void SD_handleNotFound(){
  if(hasSD && loadFromSdCard(server.uri())) return;
  String message = "SDCARD Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  DBG_OUTPUT_PORT.print(message);
}
*/
