#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Define the display
#define GFX_DEV_DEVICE ESP32_1732S019
#define GFX_BL 14

Arduino_DataBus *bus = new Arduino_ESP32SPI(11 /* DC */, 10 /* CS */, 12 /* SCK */, 13 /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 1 /* RST */, 1 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */, 0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);

// WiFi and API credentials
const char* symbols[] = {"BTC", "ETH", "SOL", "DOGE", "KAS"}; // Max 5 symbols
const char* ssid = "YOUR-SSID";
const char* password = "YOUR-PASSWORD";

WiFiClientSecure client;
HTTPClient http;

float previousPrices[5] = {0.0, 0.0, 0.0, 0.0, 0.0}; // Store previous prices for max 5 symbols

// Colors
#define BLACK gfx->color565(0, 0, 0)
#define WHITE gfx->color565(255, 255, 255)
#define RED gfx->color565(255, 0, 0)
#define GREEN gfx->color565(0, 255, 0)
#define GREY gfx->color565(200, 200, 200)
#define DARKBLUE gfx->color565(0, 50, 100)

// Helper function to show error
void displayErrorMessage(const String& message) {
  gfx->fillScreen(BLACK);
  gfx->setTextSize(3);
  gfx->setTextColor(RED);
  gfx->setCursor(10, 80);
  gfx->println(message);
}

// Initialize the display
void setupDisplay() {
  gfx->begin();
  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif
  gfx->fillScreen(BLACK);
}

// Gradient background
void drawBackground() {
  for (int y = 0; y < gfx->height(); y++) {
    uint16_t color = gfx->color565(0, y / 2, y); // Blue gradient
    gfx->drawFastHLine(0, y, gfx->width(), color);
  }
}

// Header bar
void drawHeader() {
  gfx->fillRect(0, 0, 320, 25, DARKBLUE); // Dark blue bar
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(10, 5);
  gfx->println("Crypto Prices USD 5min");
}

// Connect to WiFi
void connectToWiFi() {
  gfx->fillScreen(BLACK);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  gfx->setCursor(10, 40);
  gfx->println("Connecting to WiFi...");

  WiFi.begin(ssid, password);
  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    attempts++;

    if (attempts > 10) {
      Serial.println("Failed to connect to WiFi");
      gfx->fillRect(10, 40, 300, 20, BLACK);
      displayErrorMessage("NO WIFI");
      return;
    }
  }

  Serial.println("Connected to WiFi");
  client.setInsecure();
  gfx->fillRect(10, 40, 300, 20, BLACK);
}

// Fetch and display prices
bool fetchCryptoPrices(String& errorMessage) {
  gfx->fillScreen(BLACK);
  gfx->setTextSize(3);
  gfx->setTextColor(WHITE);
  gfx->setCursor(50, 70);
  gfx->println("FETCHING...");

  delay(2000);

  gfx->fillScreen(BLACK);
  drawHeader();

  bool anyPriceObtained = false;
  int itemCount = sizeof(symbols) / sizeof(symbols[0]);
  int spacing = (gfx->height() - 40) / itemCount; // Dynamic spacing

  for (int i = 0; i < itemCount; i++) {
    String serverName = "https://api.kraken.com/0/public/Ticker?pair=" + String(symbols[i]) + "USD";

    http.begin(client, serverName);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      StaticJsonDocument<1024> doc;
      DeserializationError jsonError = deserializeJson(doc, payload);

      if (!jsonError && doc.containsKey("result")) {
        JsonObject result = doc["result"];
        String key = String(result.begin()->key().c_str());
        JsonObject ticker = result[key];
        float price = ticker["c"][0].as<float>();

        gfx->setTextSize(2);

        int yPosition = 35 + (i * spacing);
        int symbolBoxWidth = 60; // Width for symbol box
        int priceBoxWidth = 160; // Width for price box
        int percentageBoxWidth = 80; // Width for percentage box
        int boxHeight = 25; // Box height

        // Calculate centered position
        int xPosition = (gfx->width() - symbolBoxWidth - priceBoxWidth - percentageBoxWidth) / 2; // Centering for all boxes

        // Draw box around the symbol
        gfx->drawRect(xPosition, yPosition, symbolBoxWidth, boxHeight, WHITE);
        gfx->setCursor(xPosition + 5, yPosition + 5);
        gfx->setTextColor(WHITE);
        gfx->print(symbols[i]);

        // Draw box around the price
        xPosition += symbolBoxWidth;
        gfx->drawRect(xPosition, yPosition, priceBoxWidth, boxHeight, WHITE);
        gfx->setCursor(xPosition + 5, yPosition + 5);
        if (price < 0.01) {
          gfx->printf("$%.8f", price);
        } else if (price < 1.0) {
          gfx->printf("$%.4f", price);
        } else {
          gfx->printf("$%.2f", price);
        }

        // Draw box around the percentage change
        xPosition += priceBoxWidth;
        gfx->drawRect(xPosition, yPosition, percentageBoxWidth, boxHeight, WHITE);
        gfx->setCursor(xPosition + 5, yPosition + 5);
        float prevPrice = previousPrices[i];
        if (prevPrice > 0.0) {
          float percentageChange = ((price - prevPrice) / prevPrice) * 100.0;
          if (percentageChange >= 0) {
            gfx->setTextColor(percentageChange >= 0 ? GREEN : RED);
            gfx->printf(percentageChange >= 0 ? "+%.2f%%" : "%.2f%%", percentageChange);
          }
        }

        // Update previous price
        previousPrices[i] = price;

        anyPriceObtained = true;
      }
    }

    http.end();
    delay(500);
  }

  if (!anyPriceObtained) {
    errorMessage = "API DOWN";
    return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  setupDisplay();
  drawBackground();
  drawHeader();
  connectToWiFi();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    String errorMessage;
    if (!fetchCryptoPrices(errorMessage)) {
      displayErrorMessage(errorMessage);
    }
  } else {
    displayErrorMessage("NO WIFI");
  }

  delay(300000); // Update every 15 minutes (900000)
}
