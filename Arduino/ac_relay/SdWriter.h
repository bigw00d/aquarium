#include "Arduino.h"

#include <BlockDriver.h>
#include <FreeStack.h>
#include <MinimumSerial.h>
#include <SdFat.h>
#include <SdFatConfig.h>
#include <SysCall.h>

#ifndef INCLUDED_SD_WRITER

#define INCLUDED_SD_WRITER

void SD_init();

void SD_testWrite();

void SD_writeLine(String *pStrWriteLine);

bool SD_existCard();

File SD_openFile(const char *filepath);

void SD_closeFile();

bool SD_existFile(char *filename);

void SD_remove(char *filename);

File SD_openWriteFile(const char *filepath);

void SD_closeWriteFile();

void SD_writeFile(const uint8_t *buf, size_t size);

int16_t SD_writeCsv(bool clear);

void SD_initCsv();

int16_t SD_countUpCsv();

/*
void SD_handleNotFound();

void SD_printDirectory();

void SD_handleDelete();

void SD_handleCreate();

void SD_handleFileUpload();
*/

#endif // INCLUDED_TIMER1
