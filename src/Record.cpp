// #include <Arduino.h>
// #include <SPI.h>
// #include "SdFat.h"
// #include "FreeStack.h"
// #include "UserTypes.h"
// #include "main.h"

// //==============================================================================
// // Start of configuration constants.
// //==============================================================================
// // Abort run on an overrun.  Data before the overrun will be saved.
// #define ABORT_ON_OVERRUN 1
// //------------------------------------------------------------------------------
// //Interval between data records in microseconds.
// const uint32_t LOG_INTERVAL_USEC = 3600; //Number is in milliseconds
// //------------------------------------------------------------------------------
// // Set USE_SHARED_SPI non-zero for use of an SPI sensor.
// // May not work for some cards.
// #ifndef USE_SHARED_SPI
// #define USE_SHARED_SPI 0
// #endif  // USE_SHARED_SPI
// //------------------------------------------------------------------------------
// // Pin definitions.
// //
// // SD chip select pin.
// const uint8_t SD_CS_PIN = 5;

// //
// // Digital pin to indicate an error, set to -1 if not used.
// // The led blinks for fatal errors. The led goes on solid for
// // overrun errors and logging continues unless ABORT_ON_OVERRUN
// // is non-zero.
// #ifdef ERROR_LED_PIN
// #undef ERROR_LED_PIN
// #endif  // ERROR_LED_PIN
// const int8_t ERROR_LED_PIN = LED_BUILTIN;
// //------------------------------------------------------------------------------
// // File definitions.
// //
// // Maximum file size in blocks.
// // The program creates a contiguous file with FILE_BLOCK_COUNT 512 byte blocks.
// // This file is flash erased using special SD commands.  The file will be
// // truncated if logging is stopped early.
// const uint32_t FILE_BLOCK_COUNT = 256000;
// //
// // log file base name if not defined in UserTypes.h
// #ifndef FILE_BASE_NAME
// #define FILE_BASE_NAME "data"
// #endif  // FILE_BASE_NAME
// //------------------------------------------------------------------------------
// // Buffer definitions.
// //
// // The logger will use SdFat's buffer plus BUFFER_BLOCK_COUNT-1 additional
// // buffers.
// //
// const uint8_t BUFFER_BLOCK_COUNT = 10;

// //==============================================================================
// // End of configuration constants.
// //==============================================================================
// // Temporary log file.  Will be deleted if a reset or power failure occurs.
// #define TMP_FILE_NAME FILE_BASE_NAME "##.bin"

// // Size of file base name.
// const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
// const uint8_t FILE_NAME_DIM  = BASE_NAME_SIZE + 7;
// char binName[FILE_NAME_DIM] = FILE_BASE_NAME "00.bin";

// SdFat sd;
// SdBaseFile binFile;

// // Number of data records in a block.
// const uint16_t DATA_DIM = (512 - 4)/sizeof(data_t);

// //Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
// const uint16_t FILL_DIM = 512 - 4 - DATA_DIM*sizeof(data_t);

// struct block_t {
//   uint16_t count;
//   uint16_t overrun;
//   data_t data[DATA_DIM];
//   uint8_t fill[FILL_DIM];
// };
// //==============================================================================
// // Error messages stored in flash.
// #define error(msg) {sd.errorPrint(&Serial, F(msg));fatalBlink();}
// //------------------------------------------------------------------------------
// //
// void fatalBlink() {
//   while (true) {
//     SysCall::yield();
//     if (ERROR_LED_PIN >= 0) {
//       digitalWrite(ERROR_LED_PIN, HIGH);
//       delay(200);
//       digitalWrite(ERROR_LED_PIN, LOW);
//       delay(200);
//     }
//   }
// }
// //------------------------------------------------------------------------------
// void binaryToCsv() {
//   boolean moreFiles = true;
//   uint8_t fileCounter = 0;
//   char name[FILE_NAME_DIM];

//   while(moreFiles)
//   {
//     sprintf(name,FILE_BASE_NAME "%02d.bin",fileCounter);
//     Serial.println(name);
//     uint8_t lastPct = 0;
//     block_t block;
//     uint32_t t0 = millis();
//     uint32_t syncCluster = 0;
//     SdFile csvFile;
//     char csvName[FILE_NAME_DIM];

//     if(!binFile.open(name,O_RDONLY))
//     {
//       //Current file doens't exist so there are no more
//       Serial.println("All files converted");
//       moreFiles = false;
//       return;
//     }

//     // Create a new csvFile.
//     strcpy(csvName, name);
//     strcpy(&csvName[BASE_NAME_SIZE + 3], "csv");

//     if (!csvFile.open(csvName, O_WRONLY | O_CREAT | O_TRUNC)) {
//       error("open csvFile failed");
//     }
//     binFile.rewind();
//     Serial.print(F("Writing: "));
//     Serial.print(csvName);
//     Serial.println(F(" - type any character to stop"));
//     printHeader(&csvFile);
//     uint32_t tPct = millis();
//     while (!Serial.available() && binFile.read(&block, 512) == 512) {
//       uint16_t i;
//       if (block.count == 0 || block.count > DATA_DIM) {
//         break;
//       }
//       if (block.overrun) {
//         csvFile.print(F("OVERRUN,"));
//         csvFile.println(block.overrun);
//       }
//       for (i = 0; i < block.count; i++) {
//         printData(&csvFile, &block.data[i]);
//       }
//       if (csvFile.curCluster() != syncCluster) {
//         csvFile.sync();
//         syncCluster = csvFile.curCluster();
//       }
//       if ((millis() - tPct) > 1000) {
//         uint8_t pct = binFile.curPosition()/(binFile.fileSize()/100);
//         if (pct != lastPct) {
//           tPct = millis();
//           lastPct = pct;
//           Serial.print(pct, DEC);
//           Serial.println('%');
//         }
//       }
//       if (Serial.available()) {
//         break;
//       }
//     }
//     csvFile.close();
//     binFile.close();
//     Serial.print(F("Done: "));
//     Serial.print(0.001*(millis() - t0));
//     Serial.println(F(" Seconds"));

//     fileCounter++;
//   }
// }
// //-----------------------------------------------------------------------------
// void createBinFile() {
//   // max number of blocks to erase per erase call
//   const uint32_t ERASE_SIZE = 262144L;
//   uint32_t bgnBlock, endBlock;

//   // Delete old tmp file.
//   if (sd.exists(TMP_FILE_NAME)) {
//     Serial.println(F("Deleting tmp file " TMP_FILE_NAME));
//     if (!sd.remove(TMP_FILE_NAME)) {
//       error("Can't remove tmp file");
//     }
//   }
//   // Create new file.
//   Serial.println(F("\nCreating new file"));
//   binFile.close();
//   if (!binFile.createContiguous(TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT)) {
//     error("createContiguous failed");
//   }
//   // Get the address of the file on the SD.
//   if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
//     error("contiguousRange failed");
//   }
//   // Flash erase all data in the file.
//   Serial.println(F("Erasing all data"));
//   uint32_t bgnErase = bgnBlock;
//   uint32_t endErase;
//   while (bgnErase < endBlock) {
//     endErase = bgnErase + ERASE_SIZE;
//     if (endErase > endBlock) {
//       endErase = endBlock;
//     }
//     if (!sd.card()->erase(bgnErase, endErase)) {
//       error("erase failed");
//     }
//     bgnErase = endErase + 1;
//   }
// }
// //------------------------------------------------------------------------------
// void logData() {
//   createBinFile();
//   recordBinFile();
//   renameBinFile();
// }
// //------------------------------------------------------------------------------
// void recordBinFile() {
//   const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 1;
//   // Index of last queue location.
//   const uint8_t QUEUE_LAST = QUEUE_DIM - 1;

//   // Allocate extra buffer space.
//   block_t block[BUFFER_BLOCK_COUNT - 1];

//   block_t* curBlock = 0;

//   block_t* emptyStack[BUFFER_BLOCK_COUNT];
//   uint8_t emptyTop;
//   uint8_t minTop;

//   block_t* fullQueue[QUEUE_DIM];
//   uint8_t fullHead = 0;
//   uint8_t fullTail = 0;

//   // Use SdFat's internal buffer.
//   emptyStack[0] = (block_t*)sd.vol()->cacheClear();
//   if (emptyStack[0] == 0) {
//     error("cacheClear failed");
//   }
//   // Put rest of buffers on the empty stack.
//   for (int i = 1; i < BUFFER_BLOCK_COUNT; i++) {
//     emptyStack[i] = &block[i - 1];
//   }
//   emptyTop = BUFFER_BLOCK_COUNT;
//   minTop = BUFFER_BLOCK_COUNT;

//   // Start a multiple block write.
//   if (!sd.card()->writeStart(binFile.firstBlock())) {
//     error("writeStart failed");
//   }
//   Serial.print(F("FreeStack: "));
//   Serial.println(FreeStack());
//   Serial.println(F("Logging - type any character to stop"));
//   bool closeFile = false;
//   uint32_t bn = 0;
//   uint32_t maxLatency = 0;
//   uint32_t overrun = 0;
//   uint32_t overrunTotal = 0;
//   uint32_t logTime = micros();
//   while(1) {
//      // Time for next data record.
//     logTime += LOG_INTERVAL_USEC;
//     if (Serial.available() || !digitalRead(recordSwitch)) {
//       closeFile = true;
//     }
//     if (closeFile) {
//       if (curBlock != 0) {
//         // Put buffer in full queue.
//         fullQueue[fullHead] = curBlock;
//         fullHead = fullHead < QUEUE_LAST ? fullHead + 1 : 0;
//         curBlock = 0;
//       }
//     } else {
//       if (curBlock == 0 && emptyTop != 0) {
//         curBlock = emptyStack[--emptyTop];
//         if (emptyTop < minTop) {
//           minTop = emptyTop;
//         }
//         curBlock->count = 0;
//         curBlock->overrun = overrun;
//         overrun = 0;
//       }
//       if ((int32_t)(logTime - micros()) < 0) {
//         error("Rate too fast");
//       }
//       int32_t delta;
//       do {
//         delta = micros() - logTime;
//       } while (delta < 0);
//       if (curBlock == 0) {
//         overrun++;
//         overrunTotal++;
//         if (ERROR_LED_PIN >= 0) {
//           digitalWrite(ERROR_LED_PIN, HIGH);
//         }
//   #if ABORT_ON_OVERRUN
//           Serial.println(F("Overrun abort"));
//           break;
//   #endif  // ABORT_ON_OVERRUN
//         } else {
//   #if USE_SHARED_SPI
//           sd.card()->spiStop();
//   #endif  // USE_SHARED_SPI
//           acquireData(&curBlock->data[curBlock->count++]);
//   #if USE_SHARED_SPI
//           sd.card()->spiStart();
//   #endif  // USE_SHARED_SPI
//           if (curBlock->count == DATA_DIM) {
//             fullQueue[fullHead] = curBlock;
//             fullHead = fullHead < QUEUE_LAST ? fullHead + 1 : 0;
//             curBlock = 0;
//           }
//         }
//       }
//       if (fullHead == fullTail) {
//         // Exit loop if done.
//         if (closeFile) {
//           break;
//         }
//       } else if (!sd.card()->isBusy()) {
//         // Get address of block to write.
//         block_t* pBlock = fullQueue[fullTail];
//         fullTail = fullTail < QUEUE_LAST ? fullTail + 1 : 0;
//         // Write block to SD.
//         uint32_t usec = micros();
//         if (!sd.card()->writeData((uint8_t*)pBlock)) {
//           error("write data failed");
//         }
//         usec = micros() - usec;
//         if (usec > maxLatency) {
//           maxLatency = usec;
//         }
//         // Move block to empty queue.
//         emptyStack[emptyTop++] = pBlock;
//         bn++;
//         if (bn == FILE_BLOCK_COUNT) {
//           // File full so stop
//           break;
//         }
//       }
//     }
//     if (!sd.card()->writeStop()) {
//       error("writeStop failed");
//     }
//     Serial.print(F("Min Free buffers: "));
//     Serial.println(minTop);
//     Serial.print(F("Max block write usec: "));
//     Serial.println(maxLatency);
//     Serial.print(F("Overruns: "));
//     Serial.println(overrunTotal);
//     // Truncate file if recording stopped early.
//     if (bn != FILE_BLOCK_COUNT) {
//       Serial.println(F("Truncating file"));
//       if (!binFile.truncate(512L * bn)) {
//         error("Can't truncate file");
//       }
//     }
// }
// //------------------------------------------------------------------------------
// void recoverTmpFile() {
//   uint16_t count;
//   if (!binFile.open(TMP_FILE_NAME, O_RDWR)) {
//     return;
//   }
//   if (binFile.read(&count, 2) != 2 || count != DATA_DIM) {
//     error("Please delete existing " TMP_FILE_NAME);
//   }
//   Serial.println(F("\nRecovering data in tmp file " TMP_FILE_NAME));
//   uint32_t bgnBlock = 0;
//   uint32_t endBlock = binFile.fileSize()/512 - 1;
//   // find last used block.
//   while (bgnBlock < endBlock) {
//     uint32_t midBlock = (bgnBlock + endBlock + 1)/2;
//     binFile.seekSet(512*midBlock);
//     if (binFile.read(&count, 2) != 2) error("read");
//     if (count == 0 || count > DATA_DIM) {
//       endBlock = midBlock - 1;
//     } else {
//       bgnBlock = midBlock;
//     }
//   }
//   // truncate after last used block.
//   if (!binFile.truncate(512*(bgnBlock + 1))) {
//     error("Truncate " TMP_FILE_NAME " failed");
//   }
//   renameBinFile();
// }
// //-----------------------------------------------------------------------------
// void renameBinFile() {
//   while (sd.exists(binName)) {
//     if (binName[BASE_NAME_SIZE + 1] != '9') {
//       binName[BASE_NAME_SIZE + 1]++;
//     } else {
//       binName[BASE_NAME_SIZE + 1] = '0';
//       if (binName[BASE_NAME_SIZE] == '9') {
//         error("Can't create file name");
//       }
//       binName[BASE_NAME_SIZE]++;
//     }
//   }
//   if (!binFile.rename(binName)) {
//     error("Can't rename file");
//     }
//   Serial.print(F("File renamed: "));
//   Serial.println(binName);
//   Serial.print(F("File size: "));
//   Serial.print(binFile.fileSize()/512);
//   Serial.println(F(" blocks"));
// }
// //------------------------------------------------------------------------------
// void testSensor() {
//   const uint32_t interval = 200000;
//   int32_t diff;
//   data_t data;
//   Serial.println(F("\nTesting - type any character to stop\n"));
//   // Wait for Serial Idle.
//   delay(1000);
//   printHeader(&Serial);
//   uint32_t m = micros();
//   while (!Serial.available()) {
//     m += interval;
//     do {
//       diff = m - micros();
//     } while (diff > 0);
//     acquireData(&data);
//     printData(&Serial, &data);
//   }
// }
// //------------------------------------------------------------------------------
// void setup(void) {
//   if (ERROR_LED_PIN >= 0) {
//     pinMode(ERROR_LED_PIN, OUTPUT);
//   }
//   Serial.begin(115200);

//   // Wait for USB Serial
//   while (!Serial) {
//     SysCall::yield();
//   }

//   // Allow userSetup access to SPI bus.
//   pinMode(SD_CS_PIN, OUTPUT);
//   digitalWrite(SD_CS_PIN, HIGH);

//   // Setup sensors.
//   userSetup();

//   // Initialize at the highest speed supported by the board that is
//   // not over 50 MHz. Try a lower speed if SPI errors occur.
//   if (!sd.begin(SD_CS_PIN, SD_SCK_MHZ(5))) {
//     sd.initErrorPrint(&Serial);
//     fatalBlink();
//   }

//   if(sd.exists(TMP_FILE_NAME))
//   {
//     //Always recover a file. It might be useful
//     recoverTmpFile();
//     delay(500);
//   }

//   pinMode(recordSwitch,INPUT_PULLDOWN);
//   attachInterrupt(recordSwitch,startRecording,HIGH);
// }