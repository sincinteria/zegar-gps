#include <Arduino.h>
#include <wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>
#include <time.h>

// Konfiguracja LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Konfiguracja GPS
TinyGPSPlus gps;
#define RX_PIN 18
#define TX_PIN 17
HardwareSerial gpsSerial(1);

// Zmienne do synchronizacji czasu
uint32_t lastSyncTime = 0;
const uint32_t SYNC_INTERVAL = 3600000UL;
bool gpsTimeValid = false;

// Konfiguracja podświetlenia LCD
const int BACKLIGHT_PIN = 10;           // PWM capable pin
const int BRIGHT_BACKLIGHT = 250;       // Jasność w dzień
const int DIM_BACKLIGHT = 10;           // Jasność w nocy
const int NIGHT_HOUR_START = 21;        // Godzina rozpoczęcia przyciemnienia
const int NIGHT_HOUR_END = 6;           // Godzina zakończenia przyciemnienia
bool isBacklightDimmed = false;
int currentHour = 0;
byte customCharS[8] = 
  {B00010,B01111,B10000,B01110,B00001,B00001,B11110,B00000}; // Ś
byte customChara[8] =
  {B00000,B01110,B00001,B01111,B10001,B01111,B00010,B00001}; // ą 

// Tablica skrótów nazw dni tygodnia
const char* dniTygodnia[] = {"Nie", "Pon", "Wto", "\5ro", "Czw", "Pi\1", "Sob"};

// Minimalna liczba satelitów wymagana do uznania fiksa za dobry
const int MIN_SATELLITES = 3;

bool waitForGPSSync() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Czekam na GPS...");
  lcd.setCursor(0, 1);
  lcd.print("Sat: 0  Fix: NIE");

  uint32_t startTime = millis();
  int dotCount = 0;
  uint32_t lastDotUpdate = 0;
  uint32_t lastInfoUpdate = 0;
  bool hasFix = false;

  while (true) {
    // Animacja kropek
    if (millis() - lastDotUpdate > 500) {
      lastDotUpdate = millis();
      lcd.setCursor(14, 0);
      for (int i = 0; i < dotCount; i++) {
        lcd.print(".");
      }
      for (int i = dotCount; i < 2; i++) {
        lcd.print(" ");
      }
      dotCount = (dotCount + 1) % 3;
    }
    
    // Aktualizacja informacji o satelitach i fiksie
    if (millis() - lastInfoUpdate > 1000) {
      lastInfoUpdate = millis();
      
      // Aktualizacja liczby satelitów
      lcd.setCursor(5, 1);
      lcd.print("  ");
      lcd.setCursor(5, 1);
      if (gps.satellites.isValid()) {
        lcd.print(gps.satellites.value());
      } else {
        lcd.print("0");
      }
      
      // Aktualizacja statusu fiksa
      lcd.setCursor(13, 1);
      if (gps.location.isValid() && gps.satellites.isValid() && gps.satellites.value() >= MIN_SATELLITES) {
        lcd.print("TAK");
        hasFix = true;
      } else {
        lcd.print("NIE");
        hasFix = false;
      }
    }
    
    while (gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        if (gps.time.isValid() && gps.date.isValid() && gps.location.isValid() && 
            gps.satellites.isValid() && gps.satellites.value() >= MIN_SATELLITES) {
          
          // Mamy czas, datę i lokalizację oraz wystarczającą liczbę satelitów
          struct tm tm;
          tm.tm_year = gps.date.year() - 1900;
          tm.tm_mon = gps.date.month() - 1;
          tm.tm_mday = gps.date.day();
          currentHour = gps.time.hour() + 2;
          tm.tm_hour = currentHour;
          tm.tm_min = gps.time.minute();
          tm.tm_sec = gps.time.second();
          tm.tm_isdst = -1;

          time_t t = mktime(&tm);
          struct timeval tv = { t, 0 };
          settimeofday(&tv, NULL);

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("GPS Sync OK!");
          lcd.setCursor(0, 1);
          lcd.print("SAT: ");
          lcd.print(gps.satellites.value());
          lcd.print(" FIX: TAK");
          delay(2000);
          return true;
        }
      }
    }
    
    // Pokaż, że nadal czekamy, ale nie przerywaj
    delay(10);
  }
}

void syncTimeWithGPS() {
  // Funkcja przywrócona do oryginalnej formy
  lcd.setCursor(0, 0);
  lcd.print("Sync GPS...     ");

  uint32_t startTime = millis();
  while ((uint32_t)(millis() - startTime) < 10000UL) {
    while (gpsSerial.available() > 0) {
      if (gps.encode(gpsSerial.read())) {
        if (gps.time.isValid() && gps.date.isValid()) {
          struct tm tm;
          tm.tm_year = gps.date.year() - 1900;
          tm.tm_mon = gps.date.month() - 1;
          tm.tm_mday = gps.date.day();
          currentHour = gps.time.hour() + 2;
          tm.tm_hour = currentHour;
          tm.tm_min = gps.time.minute();
          tm.tm_sec = gps.time.second();
          tm.tm_isdst = -1;

          time_t t = mktime(&tm);
          struct timeval tv = { t, 0 };
          settimeofday(&tv, NULL);

          gpsTimeValid = true;
          lcd.setCursor(0, 0);
          lcd.print("GPS Sync OK!    ");
          delay(1000);
          return;
        }
      }
    }
    delay(10);
  }

  lcd.setCursor(0, 0);
  lcd.print("GPS Sync FAIL!  ");
  delay(1000);
}

void displayTimeOnLCD() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  currentHour = p_tm->tm_hour;
  
  // Sprawdzenie czy jest godzina 5:00:00 - restart
  if (p_tm->tm_hour == 5 && p_tm->tm_min == 0 && p_tm->tm_sec == 0) {
    lcd.setCursor(0, 0);  // Dodatkowa informacja na LCD (opcjonalnie)
    lcd.print("Restarting...   ");
    delay(1000);  // Krótkie opóźnienie na wyświetlenie komunikatu
    ESP.restart();  // Wykonaj restart ESP32
  }

  char timeStringBuff[9];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", p_tm);
  
  lcd.setCursor(0, 0);
  lcd.print(timeStringBuff);
  
  lcd.setCursor(9, 0);
  lcd.print(" SAT:");
  if (gps.satellites.isValid()) {
    lcd.print(gps.satellites.value() < 10 ? "0" : "");
    lcd.print(gps.satellites.value());
  } else {
    lcd.print("--");
  }
}

void displayDateOnLCD() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  
  char dateStringBuff[11];
  strftime(dateStringBuff, sizeof(dateStringBuff), "%d.%m.%Y", p_tm);
  
  lcd.setCursor(0, 1);
  lcd.print(" ");
  lcd.print(dniTygodnia[p_tm->tm_wday]);
  lcd.print(", ");
  lcd.print(dateStringBuff);
}

void updateBacklight() {
  bool isNightTime = (currentHour >= NIGHT_HOUR_START || currentHour < NIGHT_HOUR_END);

  if (isNightTime && !isBacklightDimmed) {
    analogWrite(BACKLIGHT_PIN, DIM_BACKLIGHT);
    isBacklightDimmed = true;
  } 
  else if (!isNightTime && isBacklightDimmed) {
    analogWrite(BACKLIGHT_PIN, BRIGHT_BACKLIGHT);
    isBacklightDimmed = false;
  }
}

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Inicjalizacja podświetlenia LCD
  pinMode(BACKLIGHT_PIN, OUTPUT);
  analogWrite(BACKLIGHT_PIN, BRIGHT_BACKLIGHT);

  // Inicjalizacja LCD
  Wire.begin(8, 9);
  Wire.setClock(400000);
  lcd.init();
  lcd.createChar(5, customCharS);
  lcd.createChar(1, customChara);
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RTC GPS Sync");
  delay(1000);

  // Czekamy na synchronizację z GPS przed kontynuowaniem
  gpsTimeValid = waitForGPSSync();
  lastSyncTime = millis();
  
  lcd.clear();
}

void loop() {
  if ((uint32_t)(millis() - lastSyncTime) >= SYNC_INTERVAL) {
    syncTimeWithGPS();
    lastSyncTime = millis();
  }

  displayTimeOnLCD();
  displayDateOnLCD();
  updateBacklight();

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  delay(20);
}