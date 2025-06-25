#include <Wire.h>          // Required for I2C communication (for OLED)
#include <Adafruit_GFX.h>  // Core graphics library (for OLED)
#include <Adafruit_SSD1306.h> // SSD1306 OLED driver (for OLED)
#include "DHT.h"           // DHT sensor library

// --- DHT Sensor Definitions ---
#define DHTPIN 15          // Data pin for DHT11 sensor connected to D15 (GPIO15)
#define DHTTYPE DHT11      // DHT 11
DHT dht(DHTPIN, DHTTYPE);

// --- OLED Display Definitions ---
// These dimensions are common for the integrated SSD1306 OLEDs on ESP32 boards
#define SCREEN_WIDTH 128   // OLED display width, in pixels
#define SCREEN_HEIGHT 64   // OLED display height, in pixels (or 32 for smaller screens)

// Declaration for an SSD1306 display connected to I2C.
// The integrated OLED on your board uses the ESP32's default I2C pins:
// SDA (Serial Data Line) -> GPIO 21
// SCL (Serial Clock Line) -> GPIO 22
#define OLED_RESET -1      // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Timing for Sensor Readings ---
const unsigned long SENSOR_READ_INTERVAL = 2000; // Read sensor every 2 seconds
unsigned long lastSensorReadTime = 0;

void setup() {
  Serial.begin(9600); // Initialize serial communication for debugging
  Serial.println(F("--- DHT11 to Integrated OLED Display ---"));

  // Initialize DHT sensor
  dht.begin();
  Serial.println("DHT sensor initialized.");

  // Initialize OLED display (I2C communication)
  // The address 0x3C is very common for 128x64 I2C OLEDs.
  // SSD1306_SWITCHCAPVCC generates display voltage internally from 3.3V.
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed or not found!"));
    Serial.println(F("Check if OLED is connected/working and address is 0x3C."));
    for(;;); // Don't proceed if display fails to initialize, loop forever
  }

  // Clear the display buffer and show initial black screen
  display.display();
  delay(1000); // Pause briefly

  // Display initial message on OLED
  display.clearDisplay();         // Clear the buffer
  display.setTextSize(1);         // Normal 1:1 pixel scale for small text
  display.setTextColor(SSD1306_WHITE); // Draw white text on black background
  display.setCursor(0,0);         // Start text at top-left corner (0,0)
  display.println("DHT11 & OLED");
  display.println("Ready!");
  display.display();              // Push content to the actual display
  delay(1500);                    // Pause for user to see message
}

void loop() {
  unsigned long currentMillis = millis();

  // Check if it's time to read sensor and update OLED
  if (currentMillis - lastSensorReadTime >= SENSOR_READ_INTERVAL) {
    lastSensorReadTime = currentMillis; // Update the last read time

    // Read sensor data (can take a moment)
    float temperature_F = dht.readTemperature(true); // Temperature in Fahrenheit
    float humidity = dht.readHumidity();

    // Prepare display buffer for new readings
    display.clearDisplay(); // Clear everything from previous display cycle
    display.setCursor(0,0); // Reset text cursor to top-left for fresh text

    // --- Check if sensor readings are valid ---
    if (isnan(temperature_F) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor! Check wiring.");
      display.println("DHT Read Failed!");
      display.println("Check Wiring!");
    } else {
      // --- If readings are valid, print to Serial Monitor ---
      Serial.print("Temperature: ");
      Serial.print(temperature_F);
      Serial.println(" F");
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");

      // --- Display on OLED ---
      // Use larger text for the main readings
      display.setTextSize(2);
      display.print("Temp:");
      display.print(temperature_F, 1); // Display temperature with 1 decimal place
      display.println("F");

      // Move cursor down for humidity
      // (This assumes SCREEN_HEIGHT is 64. If 32, adjust cursor or text size)
      if (SCREEN_HEIGHT == 64) {
          display.setCursor(0, 32); // Move to start of second line for 128x64
      } else { // For 128x32 or other sizes, might need more careful placement
          display.setCursor(0, 16); // Example for 128x32 to put humidity below temp
      }
      display.print("Hum:");
      display.print(humidity, 1); // Display humidity with 1 decimal place
      display.println("%");
    }

    // Push the content of the buffer to the actual OLED screen
    display.display();
  }
}