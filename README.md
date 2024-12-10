# ESP32 Crypto Price Display

This project displays the current prices of popular cryptocurrencies on an ESP32 using an ST7789 TFT screen. It connects to WiFi, fetches cryptocurrency prices from an API, and displays them on the screen with dynamic decimal precision. 

By default it supports displaying prices for Bitcoin (BTC), Ethereum (ETH), Solana (SOL), Dogecoin (DOGE), and PEPE.

![Alt text](https://utfs.io/f/tZrGIXv7R3Npr3rCzhMKvHMOLbISWJni5XQzZ39T6xkNaPFy)

**Features:**


Dynamic Decimal Precision:

  - Coins under $0.01 show 8 decimal places  
  - Coins under $1.00 show 4 decimal places  
  - Coins $1.00 or above show 2 decimal places
  
Real-time Updates:

  - Fetches the latest prices every 1 hour  
  - Displays percentage change from the previous price

Error Handling:

- Displays error messages when unable to connect to WiFi or when the API is down

# Prerequisites

Hardware:

- ESP32 with an ST7789 TFT display
- Recommended display resolution: 170x320 pixels

Software:

- Arduino IDE with the Arduino_GFX_Library installed
- WiFi credentials (SSID and password)
- API key for the cryptocurrency price API (By default using [binance](https://www.binance.com/))

# Installation

Clone this repository to your local machine:

git clone [https://github.com/yourusername/esp32-crypto-display.git](https://github.com/panda-lr/esp32-crypto-tracker)

Open the project in Arduino IDE:

File > Open > Select the esp32-crypto-display.ino file

Ensure Arduino_GFX_Library is installed:

Go to Sketch > Include Library > Manage Libraries...

Search for Arduino_GFX_Library and install the latest version

Update the ssid, password, and apiKey in the esp32-crypto-display.ino file with your WiFi credentials and API key.

Compile and upload the code to your ESP32.

Monitor the Serial output to verify the connection to WiFi and API.
