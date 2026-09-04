#include <WiFi.h>
#include <time.h>
#include <Preferences.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>


// WiFi

const char* ssid = "NA Bro";
const char* password = "*Not Visible in this code*";


// Display pins

#define TFT_SCLK 9
#define TFT_MOSI 10
#define TFT_RST  8
#define TFT_DC   4
#define TFT_CS   5
#define TFT_BL   6

Adafruit_ST7789 tft(
  TFT_CS,
  TFT_DC,
  TFT_MOSI,
  TFT_SCLK,
  TFT_RST
);

// Buttons

#define BUTTON_PLUS   3
#define BUTTON_SET    2
#define BUTTON_MINUS  1
#define BUTTON_SCREEN 0

// Buzzer

#define BUZZER_PIN 7


// Alarm data

struct Alarm {
  int hour;
  int minute;
  bool enabled;
};

Alarm alarms[3];


// Other variables

Preferences preferences;
int currentScreen = 0;
bool editingAlarm = false;
int editingAlarmNumber = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastButtonTime = 0;
bool alarmRinging = false;
int ringingAlarm = -1;
unsigned long alarmStartTime = 0;


// Button helper

bool buttonPressed(int pin) {
  if (digitalRead(pin) == LOW) {

    if (millis() - lastButtonTime > 180) {
      lastButtonTime = millis();

      while (digitalRead(pin) == LOW) {
        delay(5);
      }

      return true;
    }
  }

  return false;
}

// Load alarms

void loadAlarms() {

  preferences.begin("blare", true);

  for (int i = 0; i < 3; i++) {

    String hourKey = "h" + String(i);
    String minKey  = "m" + String(i);
    String onKey   = "e" + String(i);

    alarms[i].hour =
      preferences.getInt(hourKey.c_str(), 7);

    alarms[i].minute =
      preferences.getInt(minKey.c_str(), 0);

    alarms[i].enabled =
      preferences.getBool(onKey.c_str(), false);
  }

  preferences.end();
}


// Save one alarm

void saveAlarm(int number) {

  preferences.begin("blare", false);

  String hourKey = "h" + String(number);
  String minKey  = "m" + String(number);
  String onKey   = "e" + String(number);

  preferences.putInt(
    hourKey.c_str(),
    alarms[number].hour
  );

  preferences.putInt(
    minKey.c_str(),
    alarms[number].minute
  );

  preferences.putBool(
    onKey.c_str(),
    alarms[number].enabled
  );

  preferences.end();
}


// Get current time

void getCurrentTime(int &hour, int &minute, int &second) {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    hour = 0;
    minute = 0;
    second = 0;
    return;
  }

  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
}

// Draw normal clock

void showClock() {

  int hour;
  int minute;
  int second;

  getCurrentTime(hour, minute, second);

  char timeText[10];

  sprintf(
    timeText,
    "%02d:%02d",
    hour,
    minute
  );

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(6);

  tft.setCursor(20, 5);

  tft.print(timeText);

  // Small seconds display
  tft.setTextSize(2);

  tft.setCursor(125, 58);

  char secText[5];

  sprintf(secText, "%02d", second);

  tft.print(secText);

  tft.setTextSize(1);

  tft.setCursor(5, 5);
  tft.print("BLARE");
}


// Show alarm screen

void showAlarm(int number) {

  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);

  // Alarm NO
  tft.setTextSize(2);

  tft.setCursor(5, 3);

  tft.print("ALARM ");
  tft.print(number + 1);


  // Alarm time
  tft.setTextSize(5);

  tft.setCursor(35, 20);

  char alarmText[10];

  sprintf(
    alarmText,
    "%02d:%02d",
    alarms[number].hour,
    alarms[number].minute
  );

  tft.print(alarmText);


  // Status
  tft.setTextSize(2);

  tft.setCursor(105, 62);

  if (alarms[number].enabled) {
    tft.setTextColor(ST77XX_GREEN);
    tft.print("ON");
  }
  else {
    tft.setTextColor(ST77XX_RED);
    tft.print("OFF");
  }


  // Editing message
  if (editingAlarm &&
      editingAlarmNumber == number) {

    tft.setTextColor(ST77XX_WHITE);

    tft.setTextSize(1);

    tft.setCursor(5, 65);

    tft.print("+/- CHANGE   SET SAVE");
  }
}


// Show ringing screen

void showAlarmRinging() {

  tft.fillScreen(ST77XX_RED);

  tft.setTextColor(ST77XX_WHITE);

  tft.setTextSize(4);

  tft.setCursor(35, 10);

  tft.print("ALARM!");

  tft.setTextSize(2);

  tft.setCursor(40, 55);

  tft.print("PRESS SET");

  tft.setCursor(40, 70);

  tft.print("TO STOP");
}

// Start buzzer

void startAlarm(int number) {

  alarmRinging = true;
  ringingAlarm = number;

  alarmStartTime = millis();

  tone(BUZZER_PIN, 2000);

  showAlarmRinging();
}


// Stop buzzer

void stopAlarm() {

  noTone(BUZZER_PIN);

  alarmRinging = false;

  if (ringingAlarm >= 0) {

    // Turn this alarm off after
    alarms[ringingAlarm].enabled = false;

    saveAlarm(ringingAlarm);
  }

  ringingAlarm = -1;

  currentScreen = 0;

  showClock();
}


// Check alarms

void checkAlarms() {

  if (alarmRinging) {
    return;
  }

  int hour;
  int minute;
  int second;

  getCurrentTime(hour, minute, second);


  // Only check at start of minute
  if (second != 0) {
    return;
  }


  for (int i = 0; i < 3; i++) {

    if (!alarms[i].enabled) {
      continue;
    }

    if (alarms[i].hour == hour &&
        alarms[i].minute == minute) {

      startAlarm(i);

      break;
    }
  }
}


// Change alarm time

void increaseAlarm() {

  alarms[editingAlarmNumber].minute += 1;

  if (alarms[editingAlarmNumber].minute >= 60) {

    alarms[editingAlarmNumber].minute = 0;

    alarms[editingAlarmNumber].hour += 1;

    if (alarms[editingAlarmNumber].hour >= 24) {
      alarms[editingAlarmNumber].hour = 0;
    }
  }
}


void decreaseAlarm() {

  alarms[editingAlarmNumber].minute -= 1;

  if (alarms[editingAlarmNumber].minute < 0) {

    alarms[editingAlarmNumber].minute = 59;

    alarms[editingAlarmNumber].hour -= 1;

    if (alarms[editingAlarmNumber].hour < 0) {
      alarms[editingAlarmNumber].hour = 23;
    }
  }
}


// Handle buttons

void handleButtons() {


  // =======================
  // If alarm is ringing
  // =======================

  if (alarmRinging) {

    if (buttonPressed(BUTTON_SET)) {
      stopAlarm();
    }

    return;
  }


  // Change screen

  if (buttonPressed(BUTTON_SCREEN)) {

    // If currently editing, don't change screen
    if (!editingAlarm) {

      currentScreen++;

      if (currentScreen > 3) {
        currentScreen = 0;
      }

      if (currentScreen == 0) {
        showClock();
      }
      else {
        showAlarm(currentScreen - 1);
      }
    }

    return;
  }

  // Normal clock screen

  if (currentScreen == 0) {

    // Buttons don't change clock
    // on the main screen

    return;
  }

  // Alarm screen

  int alarmNumber = currentScreen - 1;


  // SET button

  if (buttonPressed(BUTTON_SET)) {

    if (!editingAlarm) {

      // Start editing
      editingAlarm = true;
      editingAlarmNumber = alarmNumber;

      showAlarm(alarmNumber);

    }
    else {

      // Save alarm
      alarms[alarmNumber].enabled = true;

      saveAlarm(alarmNumber);

      editingAlarm = false;

      showAlarm(alarmNumber);
    }

    return;
  }


  // + button

  if (editingAlarm &&
      buttonPressed(BUTTON_PLUS)) {

    increaseAlarm();

    showAlarm(alarmNumber);

    return;
  }


  // - button

  if (editingAlarm &&
      buttonPressed(BUTTON_MINUS)) {

    decreaseAlarm();

    showAlarm(alarmNumber);

    return;
  }
}


// Setup

void setup() {

  Serial.begin(115200);


  // Buttons
  pinMode(
    BUTTON_PLUS,
    INPUT_PULLUP
  );

  pinMode(
    BUTTON_SET,
    INPUT_PULLUP
  );

  pinMode(
    BUTTON_MINUS,
    INPUT_PULLUP
  );

  pinMode(
    BUTTON_SCREEN,
    INPUT_PULLUP
  );


  // Buzzer
  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  noTone(BUZZER_PIN);


  // Display backlight
  pinMode(
    TFT_BL,
    OUTPUT
  );

  digitalWrite(
    TFT_BL,
    LOW
  );


  // Display
  tft.init(76, 284);

  tft.setOffsets(82, 18);

  tft.invertDisplay(false);

  tft.setRotation(1);

  tft.fillScreen(ST77XX_BLACK);


  // Starting message
  tft.setTextColor(ST77XX_WHITE);

  tft.setTextSize(2);

  tft.setCursor(35, 25);

  tft.print("BLARE");

  delay(1000);


  // Connect WiFi

  tft.fillScreen(ST77XX_BLACK);

  tft.setCursor(20, 25);

  tft.print("WiFi...");

  WiFi.begin(
    ssid,
    password
  );

  int tries = 0;

  while (
    WiFi.status() != WL_CONNECTED &&
    tries < 30
  ) {

    delay(500);

    Serial.print(".");

    tries++;
  }


  // NTP time

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println();

    Serial.println(
      "WiFi connected"
    );


    // India timezone
    configTime(
      19800,
      0,
      "pool.ntp.org",
      "time.nist.gov"
    );

    tft.fillScreen(ST77XX_BLACK);

    tft.setCursor(20, 25);

    tft.print("SYNC TIME");

    delay(1500);
  }
  else {

    Serial.println(
      "WiFi failed"
    );

    tft.fillScreen(ST77XX_BLACK);

    tft.setCursor(20, 25);

    tft.print("NO WIFI");

    delay(1500);
  }


  // Load saved alarm
  loadAlarms();


  // Start clk
  currentScreen = 0;

  showClock();
}

// Main loop

void loop() {

  handleButtons();

  checkAlarms();


  // Updt clk a sec
  if (
    currentScreen == 0 &&
    !editingAlarm &&
    !alarmRinging
  ) {

    if (
      millis() - lastDisplayUpdate >= 1000
    ) {

      lastDisplayUpdate = millis();

      showClock();
    }
  }


  // Keep Screen Updt
  if (
    currentScreen > 0 &&
    !alarmRinging
  ) {

    if (
      millis() - lastDisplayUpdate >= 1000
    ) {

      lastDisplayUpdate = millis();

      showAlarm(
        currentScreen - 1
      );
    }
  }


  // Stop 2 mins
  if (
    alarmRinging &&
    millis() - alarmStartTime >= 120000
  ) {

    stopAlarm();
  }


  delay(10);
}