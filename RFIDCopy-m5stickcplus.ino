#include <Wire.h>
#include "MFRC522_I2C.h"
#include <M5StickCPlus.h>

MFRC522 mfrc522(0x28); // Create MFRC522 instance.

enum state {
  read_mode,
  write_mode
} currentState;

bool readUID = false;

byte UID[20];
uint8_t UIDLength = 0;

void displayReadMode() {
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.println(F("RFID Cloner. V1.0"));
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.println(F("(Press 'A' to write after reading...)"));
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.println(F("Ready to READ..."));
}

void displayWriteMode() {
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.println(F("RFID Cloner. V1.0"));
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.println(F("(Press 'A' to read a new card...)"));
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.println(F("Ready to WRITE..."));
  displayUID();
}

void beep_attention() {
  M5.Beep.tone(882, 40);
  delay(100);
  M5.Beep.mute();
}

void beep_error() {
  M5.Beep.tone(495, 60);
  delay(60);
  M5.Beep.mute();
}

void beep_writeOK() {
  M5.Beep.tone(661, 60);
  delay(60);
  M5.Beep.tone(882, 100);
  delay(100);
  M5.Beep.mute();
}

void cls() {
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
}

void setup() {
  M5.begin();
  M5.Lcd.setRotation(1); // Rotate the display by 90 degrees
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.setCursor(0, 0);
  Serial.begin(115200);
  Wire.begin();
  mfrc522.PCD_Init();
  currentState = read_mode;
  displayReadMode();
}

void loop() {
  M5.update();

  if (M5.BtnA.wasReleased() && readUID) {
    cls();
    switch (currentState) {
      case read_mode:
        currentState = write_mode;
        displayWriteMode();
        break;
      case write_mode:
        currentState = read_mode;
        displayReadMode();
        readUID = false;
        break;
    }
  }

  if (!mfrc522.PICC_IsNewCardPresent())
    return;
  if (!mfrc522.PICC_ReadCardSerial())
    return;

  cls();

  switch (currentState) {
    case read_mode:
      displayReadMode();
      readCard();
      break;
    case write_mode:
      displayWriteMode();
      writeCard();
      break;
  }

  mfrc522.PICC_HaltA();
}

void readCard() {
  MFRC522::PICC_Type piccType = (MFRC522::PICC_Type)mfrc522.PICC_GetType(mfrc522.uid.sak);
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.print(F(""));
  M5.Lcd.print(mfrc522.PICC_GetTypeName(piccType));
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.print(F(" (SAK "));
  M5.Lcd.print(mfrc522.uid.sak);
  M5.Lcd.print(")\r\n");
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(1); // Reduce text size
    M5.Lcd.setCursor(0, 80); // Move the error message down
    M5.Lcd.println(F("Not MIFARE Classic"));
    M5.Lcd.setCursor(0, 0); // Reset cursor
    M5.Lcd.setTextColor(YELLOW);
    beep_error();
    delay(1000);
  } else {
    M5.Lcd.println("");
    readUID = true;
    UIDLength = mfrc522.uid.size;
    for (byte i = 0; i < UIDLength; i++) {
      UID[i] = mfrc522.uid.uidByte[i];
    }
    Serial.println();
    displayUID();
    beep_attention();
    delay(1000);
  }
}

void displayUID() {
  M5.Lcd.setTextSize(1); // Reduce text size
  M5.Lcd.println(F("User ID:"));
  M5.Lcd.setTextSize(1); // Reduce text size
  for (byte i = 0; i < UIDLength; i++) {
    M5.Lcd.print(UID[i] < 0x10 ? " 0" : " ");
    M5.Lcd.print(UID[i], HEX);
  }
}

void writeCard() {
  if (mfrc522.MIFARE_SetUid(UID, (byte)UIDLength, true)) {
    M5.Lcd.println();
    M5.Lcd.setTextSize(1); // Reduce text size
    M5.Lcd.println(F("Wrote UID."));
    M5.Lcd.setTextSize(1); // Reduce text size
    M5.Lcd.println();
    beep_writeOK();
  } else {
    M5.Lcd.setTextColor(RED);
    M5.Lcd.setTextSize(1); // Reduce text size
    M5.Lcd.println();
    M5.Lcd.println(F("Write failed."));
    M5.Lcd.setTextSize(1); // Reduce text size
    M5.Lcd.setTextColor(YELLOW);
    beep_error();
  }

  mfrc522.PICC_HaltA();
  delay(1000);
}