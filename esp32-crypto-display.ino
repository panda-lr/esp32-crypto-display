#include <Arduino_GFX_Library.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#define GFX_DEV_DEVICE ESP32_1732S019
#define GFX_BL 14

// Change width and height, and set rotation to 1 (landscape)
Arduino_DataBus *bus = new Arduino_ESP32SPI(11 /* DC */, 10 /* CS */, 12 /* SCK */, 13 /* MOSI */, GFX_NOT_DEFINED /* MISO */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 1 /* RST */, 1 /* rotation */, true /* IPS */, 170 /* width */, 320 /* height */, 35 /* col offset 1 */, 0 /* row offset 1 */, 35 /* col offset 2 */, 0 /* row offset 2 */);

const char* symbols[] = {"BTC", "ETH", "SOL", "DOGE", "PEPE"};
const char* fullSymbols[] = {"BTCUSDT", "ETHUSDT", "SOLUSDT", "DOGEUSDT", "PEPEUSDT"};
const char* ssid = "your-SSID";
const char* password = "your-password";
const char* apiKey = "your-key"; // Replace with your Binance API key
const char* secretKey = "your-secret"; // Replace with your Binance Secret key

WiFiClientSecure client;
HTTPClient http;

float previousPrices[5] = {0.0, 0.0, 0.0, 0.0, 0.0}; // Store previous prices

void displayErrorMessage(const String& message) {
  gfx->fillScreen(BLACK);
  gfx->setTextSize(3);
  gfx->setTextColor(RED);
  gfx->setCursor(10, 80);
  gfx->println(message);
}

void setupDisplay() {
  gfx->begin();
  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif
  gfx->fillScreen(BLACK);
}

void drawHeader() {
  gfx->fillRect(0, 0, 320, 30, DARKGREY);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(10, 5);
  gfx->println("Crypto Prices /USDT 5min");
}

void connectToWiFi() {
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
    String symbol = fullSymbols[i];
    String serverName = "https://api.binance.com/api/v3/ticker/price?symbol=" + symbol;

    http.begin(client, serverName);
    http.addHeader("X-MBX-APIKEY", apiKey); // Binance API Key
    http.addHeader("X-MBX-APISECRET", secretKey); // Binance Secret Key

    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      StaticJsonDocument<256> doc;
      DeserializationError jsonError = deserializeJson(doc, payload);

      if (!jsonError && doc.containsKey("price")) {
        float price = doc["price"].as<float>();

        gfx->setTextSize(2);

        int yPosition = 40 + (i * spacing);

        // Display symbol
        gfx->setTextColor(WHITE);
        gfx->setCursor(10, yPosition);
        gfx->print(symbols[i]);

        // Display price with dynamic formatting
        gfx->setCursor(80, yPosition);
        if (price < 0.01) {
          gfx->printf("$%.8f", price);
        } else if (price < 1.0) {
          gfx->printf("$%.4f", price);
        } else {
          gfx->printf("$%.2f", price);
        }

        // Calculate and display percentage change
        float prevPrice = previousPrices[i];
        if (prevPrice > 0.0) {
          float percentageChange = ((price - prevPrice) / prevPrice) * 100.0;

          gfx->setCursor(220, yPosition); // Adjusted for better alignment
          if (percentageChange >= 0) {
            gfx->setTextColor(GREEN);
            gfx->printf("+%.2f%%", percentageChange);
          } else {
            gfx->setTextColor(RED);
            gfx->printf("%.2f%%", percentageChange);
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

  // Refresh every 5 minutes (300 seconds)
  delay(300000);
}
