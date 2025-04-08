#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TinyGPS++.h>

// Stałe konfiguracyjne
const int SYNC_INTERVAL = 120000; // 2 minuty (w ms)
const int TIME_ZONE_OFFSET_WINTER = 1; // Przesunięcie czasu zimowego (CET)
const int TIME_ZONE_OFFSET_SUMMER = 2; // Przesunięcie czasu letniego (CEST)
const int SYNC_MESSAGE_DURATION = 2000; // Czas wyświetlania komunikatu (2 sekundy)

// Konfiguracja podświetlenia LCD
const int BACKLIGHT_PIN = 10;           // Zmień na właściwy pin sterujący podświetleniem
const int BRIGHT_BACKLIGHT = 250;       // Jasność w ciągu dnia (max 255)
const int DIM_BACKLIGHT = 10;           // Przyciemniona jasność w nocy (dostosuj wg potrzeb)
const int NIGHT_HOUR_START = 21;        // Godzina rozpoczęcia przyciemnienia
const int NIGHT_HOUR_END = 6;           // Godzina zakończenia przyciemnienia
// Zmienna do śledzenia stanu podświetlenia
bool isBacklightDimmed = false;

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

// Tablica skrótów dni tygodnia
const char* dayNames[] = {"Pon", "Wto", "Sro", "Czw", "Pia", "Sob", "Nie"};

// Deklaracje funkcji (prototypy)
void setTimeAndDateFromGPS();
void updateLocalTime();
void displayTimeAndSat();
void displayDateOrSyncMessage();
bool isSummerTime(int year, int month, int day, int hour);
int calculateDayOfWeek(int y, int m, int d);
void updateBacklight();

void setup() {
  // Inicjalizacja komunikacji szeregowej
  // Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Inicjalizacja wyświetlacza LCD
  Wire.begin(8, 9);  // SDA = GPIO8, SCL = GPIO9

  // Konfiguracja pinu podświetlenia jako wyjście PWM
  pinMode(BACKLIGHT_PIN, OUTPUT);
  
  // Konfiguracja PWM dla ESP32
  ledcSetup(0, 5000, 8);  // Kanał 0, częstotliwość 5kHz, rozdzielczość 8-bit
  ledcAttachPin(BACKLIGHT_PIN, 0);  // Przypisanie pinu do kanału PWM
  ledcWrite(0, BRIGHT_BACKLIGHT);  // Ustawienie początkowe na pełną jasność

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.print("Czekam na GPS...");
  updateBacklight();
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
      updateBacklight();  // Sprawdź i dostosuj jasność podświetlenia
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
    // Oblicz dzień tygodnia
    int dayOfWeek = calculateDayOfWeek(localYear, localMonth, localDay);
    
    // Wyświetl skróconą nazwę dnia tygodnia i datę
    lcd.print(dayNames[dayOfWeek]);
    lcd.print(",  ");
    if (localDay < 10) lcd.print("0");
    lcd.print(localDay);
    lcd.print(".");
    if (localMonth < 10) lcd.print("0");
    lcd.print(localMonth);
    lcd.print(".");
    lcd.print(localYear);
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

// Funkcja obliczająca dzień tygodnia (algorytm Zellera)
// Zwraca 0=Poniedziałek, ..., 6=Sobota
int calculateDayOfWeek(int y, int m, int d) {
  if (m < 3) {
    m += 12;
    y--;
  }
  int k = y % 100;
  int j = y / 100;
  int dayOfWeek = (d + 13*(m+1)/5 + k + k/4 + j/4 + 5*j) % 7;
  return (dayOfWeek + 5) % 7; // Dostosowanie do formatu 0=Poniedziałek
}

// Nowa funkcja do aktualizacji jasności podświetlenia na podstawie godziny
void updateBacklight() {
  // Sprawdź, czy jest noc (między NIGHT_HOUR_START a NIGHT_HOUR_END)

  int currentHour = localHour;
  bool isNightTime = (currentHour >= NIGHT_HOUR_START || currentHour < NIGHT_HOUR_END);

  // Zmień jasność tylko gdy zmienia się stan (dzień/noc)
  if (isNightTime && !isBacklightDimmed) {
    // Przyciemnij podświetlenie na noc
    ledcWrite(0, DIM_BACKLIGHT);
    isBacklightDimmed = true;
  } 
  else if (!isNightTime && isBacklightDimmed) {
    // Rozjaśnij podświetlenie na dzień
    ledcWrite(0, BRIGHT_BACKLIGHT);
    isBacklightDimmed = false;
  }
}