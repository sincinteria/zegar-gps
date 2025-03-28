
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>

// Stałe konfiguracyjne
const int SYNC_INTERVAL = 120000; // 2 minuty (w ms)
const int TIME_ZONE_OFFSET_WINTER = 1; // Przesunięcie czasu zimowego (CET)
const int TIME_ZONE_OFFSET_SUMMER = 2; // Przesunięcie czasu letniego (CEST)
const int SYNC_MESSAGE_DURATION = 2000; // Czas wyświetlania komunikatu (2 sekundy)

// Inicjalizacja wyświetlacza LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Inicjalizacja obiektu TinyGPS++
TinyGPSPlus gps;

// Definiowanie pinów dla UART (RX, TX)
#define RX_PIN 18
#define TX_PIN 17

// Użycie wbudowanego UART1 (Serial1) do komunikacji z modułem GPS
HardwareSerial gpsSerial(1);

// Zmienne do przechowywania czasu i daty
int localHour = 0;
int localMinute = 0;
int localSecond = 0;
int localDay = 0;
int localMonth = 0;
int localYear = 0;
unsigned long lastSyncTime = 0; // Ostatni czas synchronizacji z GPS
unsigned long lastUpdateTime = 0; // Ostatni czas aktualizacji wyświetlacza
bool isTimeSet = false; // Flaga wskazująca, czy czas został ustawiony z GPS

// Zmienne do wyświetlania komunikatu synchronizacji
bool showSyncMessage = false; // Flaga wskazująca, czy wyświetlić komunikat
unsigned long syncMessageDisplayTime = 0; // Czas wyświetlenia komunikatu

// Deklaracje funkcji (prototypy)
void setTimeAndDateFromGPS();
void updateLocalTime();
void displayTimeAndSat();
void displayDateOrSyncMessage();
bool isSummerTime(int year, int month, int day, int hour);

void setup() {
  // Inicjalizacja komunikacji szeregowej
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Inicjalizacja wyświetlacza LCD
  Wire.begin(8, 9);  // SDA = GPIO8, SCL = GPIO9
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Czekam na GPS...");
}

void loop() {
  // Sprawdzanie, czy dostępne są nowe dane z modułu GPS
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Jeśli czas i data nie zostały jeszcze ustawione z GPS, ustaw je
  if (!isTimeSet && gps.time.isValid() && gps.date.isValid()) {
    setTimeAndDateFromGPS();
    isTimeSet = true;
    lcd.clear(); // Wyczyść ekran po ustawieniu czasu i daty
  }

  // Synchronizacja czasu i daty z GPS co określony interwał
  if (isTimeSet && millis() - lastSyncTime >= SYNC_INTERVAL) {
    if (gps.time.isValid() && gps.date.isValid()) {
      setTimeAndDateFromGPS();
      showSyncMessage = true; // Ustaw flagę do wyświetlenia komunikatu
      syncMessageDisplayTime = millis(); // Zapisz czas wyświetlenia komunikatu
    }
  }

  // Aktualizacja czasu lokalnego co 1 sekundę
  if (isTimeSet && millis() - lastUpdateTime >= 1000) {
    updateLocalTime();
    displayTimeAndSat();
    lastUpdateTime = millis();
  }

  // Wyświetlanie daty lub komunikatu synchronizacji
  displayDateOrSyncMessage();
}

// Funkcja do ustawiania czasu i daty z GPS
void setTimeAndDateFromGPS() {
  localHour = gps.time.hour();
  localMinute = gps.time.minute();
  localSecond = gps.time.second();
  localDay = gps.date.day();
  localMonth = gps.date.month();
  localYear = gps.date.year();

  // Sprawdź, czy obowiązuje czas letni czy zimowy
  if (isSummerTime(localYear, localMonth, localDay, localHour)) {
    localHour += TIME_ZONE_OFFSET_SUMMER; // Czas letni (CEST)
  } else {
    localHour += TIME_ZONE_OFFSET_WINTER; // Czas zimowy (CET)
  }

  // Przetwarzaj godziny powyżej 23
  if (localHour >= 24) {
    localHour -= 24;
    localDay++; // Przesuń dzień, jeśli przekroczono północ
  }

  lastSyncTime = millis();
}

// Funkcja do aktualizacji czasu lokalnego
void updateLocalTime() {
  localSecond++;
  if (localSecond >= 60) {
    localSecond = 0;
    localMinute++;
    if (localMinute >= 60) {
      localMinute = 0;
      localHour++;
      if (localHour >= 24) {
        localHour = 0;
        localDay++; // Przesuń dzień, jeśli przekroczono północ
      }
    }
  }
}

// Funkcja do wyświetlania czasu i liczby satelitów
void displayTimeAndSat() {
  lcd.setCursor(0, 0);
  // Wyświetl czas w formacie HH:MM:SS
  if (localHour < 10) lcd.print("0");
  lcd.print(localHour);
  lcd.print(":");
  if (localMinute < 10) lcd.print("0");
  lcd.print(localMinute);
  lcd.print(":");
  if (localSecond < 10) lcd.print("0");
  lcd.print(localSecond);

  // Wyświetl liczbę satelitów
  lcd.print("  sat:");  
  if (gps.satellites.isValid()) {
    if (gps.satellites.value() < 10) lcd.print("0"); // Dodaj zero wiodące dla jednocyfrowej liczby
    lcd.print(gps.satellites.value());
  } else {
    lcd.print("--");
  }
}

// Funkcja do wyświetlania daty lub komunikatu synchronizacji
void displayDateOrSyncMessage() {
  lcd.setCursor(0, 1);

  if (showSyncMessage) {
    // Wyświetl komunikat "Pobrano czas GPS"
    lcd.print("Pobrano czas GPS");

    // Sprawdź, czy minęły 2 sekundy od wyświetlenia komunikatu
    if (millis() - syncMessageDisplayTime >= SYNC_MESSAGE_DURATION) {
      showSyncMessage = false; // Ukryj komunikat
      lcd.setCursor(0, 1);
      lcd.print("                "); // Wyczyść linię
    }
  } else {
    // Wyświetl datę w formacie DD.MM.YYYY
    if (localDay < 10) lcd.print("0");
    lcd.print(localDay);
    lcd.print(".");
    if (localMonth < 10) lcd.print("0");
    lcd.print(localMonth);
    lcd.print(".");
    lcd.print(localYear);
    if (isSummerTime(localYear, localMonth, localDay, localHour)) {
      lcd.print("  lato"); // Czas letni (CEST)
    } else {
      lcd.print("  zima"); // Czas zimowy (CET)
    }
  }
}

// Funkcja sprawdzająca, czy obowiązuje czas letni
bool isSummerTime(int year, int month, int day, int hour) {
  // Czas letni obowiązuje od ostatniej niedzieli marca do ostatniej niedzieli października
  if (month > 3 && month < 10) {
    return true; // Kwiecień–Wrzesień: czas letni
  } else if (month == 3) {
    // Marzec: czas letni zaczyna się ostatniej niedzieli
    int lastSunday = 31 - (5 * year / 4 + 4) % 7; // Oblicz ostatnią niedzielę marca
    if (day > lastSunday || (day == lastSunday && hour >= 2)) {
      return true;
    }
  } else if (month == 10) {
    // Październik: czas letni kończy się ostatniej niedzieli
    int lastSunday = 31 - (5 * year / 4 + 1) % 7; // Oblicz ostatnią niedzielę października
    if (day < lastSunday || (day == lastSunday && hour < 3)) {
      return true;
    }
  }
  return false; // Czas zimowy
}