#include <Adafruit_NeoPixel.h>
#include <sensor_dhcp_server.h>
#include <sh1106_spi.h>
#include <InfinitagGFX.h>

SensorDHCPServer SensorServer(DHCP_MASTER_ADDRESS, 30);

const int buttonPin = 2;
const int lifePin = 13;
const int shotLedDataPin = 8;
const int resetPin = 4;
const int dcPin = 5;
const int csPin = 6;

int buttonState = 0;

bool alive = true;
unsigned long timeOfDeath = 0;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(4, shotLedDataPin, NEO_GRBW + NEO_KHZ800);
sh1106_spi display = create_display(resetPin, dcPin, csPin);
Framebuffer framebuffer;

void setup() {
  Serial.begin(57600);
  Serial.println("booting");
  
  SensorServer.initialize();
  Serial.println("booting.");
  Wire.onRequest(requestEvent);
  Serial.println("booting..");

  pinMode(buttonPin, INPUT);
  pinMode(lifePin, OUTPUT);
  pinMode(shotLedDataPin, OUTPUT);

  Serial.println("booting...");
  SPI.begin();
  initialize_display(&display);
  
  Serial.println("booting....");
  strip.begin();
  colorWipe(strip.Color(0,0,0,0));
  Serial.println("boot completed");
}

void loop() {
  framebuffer.clear(BLACK);
  buttonState = digitalRead(buttonPin);

  if(alive)
  {
    framebuffer.displayText("Alive", 50, 24, WHITE);
    digitalWrite(lifePin, HIGH);
    if(buttonState == HIGH)
    {
      alive = false;
      timeOfDeath = millis();
      colorWipe(strip.Color(255,0,0));
    }
  }
  else
  {
    digitalWrite(lifePin, LOW);
    unsigned long currentTime = millis();
    unsigned long ti = currentTime - timeOfDeath;
    char number[100];
    String(ti/1000).toCharArray(number, 100);
    framebuffer.displayText(number, 55, 24, WHITE);
    if(ti > 500)
    {
      colorWipe(strip.Color(0,0,0,0));
    }
    if(ti > 20000) //20 seconds
    {
      alive = true; 
    }
  }
  SensorServer.scanIfNecessary();
  display_buffer(&display, framebuffer.getData());
  delay(10);
}

void requestEvent()
{
  //maybe we have enough time to look for the next free address?
  Serial.println("requested");
  SensorServer.registerNewClient();
}

void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
}