// Usese and .91 OLED display and a DHT22 Temp/Hum Sensor
// Button used to switch from Temp to Hum on display

#include <Dimmer.h>
#include <BlinkTask.h>
#include <FrequencyTask.h>
#include <Task.h>
#include <Heartbeat.h>
#include <Debouncer.h>
#include <Rotary.h>
#include <SoftTimer.h>
#include <TonePlayer.h>
#include <DelayRun.h>
#include <SoftPwmTask.h>
#include <PciManager.h>

#include <DHT.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

//Variables
#define INPUT_PIN 3       //Define button input pin
#define DHTPIN A0         //What pin we're connected to
#define DHTTYPE DHT22     //DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE); //Initialize DHT sensor for normal 16mhz Arduino

// variables will change:
bool isPushed = false; //Need to see how many times we pushed the button, toggle between temp and hum screens
String displayText;    //Need a place to store text for OLED
float hum;             //Stores humidity value
float temp;            //Stores temperature value
float fTemp;           //Calculate Fahrnegislaoshight (can't spell it)

//Pins for rgb led
int redPin = 11;
int greenPin = 10;
int bluePin = 9;

//Setup task to grab env data
void getEnvData(Task* me);
Task taskGetEnvData(4000, getEnvData);

//Setup task to grab button press
void onPressed();
void onReleased(unsigned long pressTimespan);
Debouncer debouncer(INPUT_PIN, MODE_CLOSE_ON_PUSH, onPressed, onReleased);

//Setup task to show temp on the display
void showTempDisplay(Task* me);
Task taskShowTempDisplay(10000, showTempDisplay);

//Setup task to show humidity on the display
void showHumDisplay(Task* me);
Task taskShowHumDisplay(10000, showHumDisplay);

void setup() {
 // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  // Clear the buffer.
  display.clearDisplay();
 
  pinMode(DHTPIN, INPUT); //Set dht - A0 pin as an input (optional)
  Serial.begin(9600);
  dht.begin();

  //Pin modes for RGB LED
  pinMode(redPin, OUTPUT); 
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);  

  SoftTimer.add(&taskGetEnvData);
  PciManager.registerListener(INPUT_PIN, &debouncer);
  SoftTimer.add(&taskShowTempDisplay);  //Task runs as the default if no button is pushed
}

//Function to set the color of the RGB LED
void setColor(int red, int green, int blue) {
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

//Function to get dht readings
void getEnvData(Task* me) {
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  fTemp = (temp * 9/5 + 32);
  if (fTemp > 79.9) {
    setColor(255, 0, 0);                    //Red
  } else if (fTemp < 68) {
    setColor(0, 0, 255);                    //Blue
  } else if (fTemp > 76 && fTemp < 81) {
    setColor(255, 171, 0);                  //Orange
  } else if (fTemp > 67 && fTemp < 77) {
    setColor(0, 255, 0);                    //Green
  }
  Serial.println(fTemp);
  Serial.println(hum);
}

//Function if button is pressed
void onPressed() {
  Serial.println("pressed");
  //Set a var of temp or hum to display on OLED
  if (isPushed) {
    displayText = "Temperature";
  } else {
    displayText = "Humidity";
  }
  isPushed = !isPushed;
  //Show text on OLED
  //display.clearDisplay();
  //display.setTextSize(1);
  //display.setTextColor(WHITE);
  //display.setCursor(0,12);
  //display.print(displayText);
  //display.display();
  if (displayText == "Humidity") {
    SoftTimer.remove(&taskShowTempDisplay);
    SoftTimer.add(&taskShowHumDisplay);
  } else if (displayText == "Temperature") {
    SoftTimer.add(&taskShowTempDisplay);
    SoftTimer.remove(&taskShowHumDisplay);
  }

}

//Function needed or debounce
void onReleased(unsigned long pressTimespan) {
  Serial.print(pressTimespan);
  Serial.println(" - released");
}

//Function to show Temp
void showTempDisplay(Task* me) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Temp");
  display.setTextSize(3);
  display.setCursor(24, 10);
  display.print(fTemp, 1);
  display.setTextSize(1);
  display.setCursor(96, 10);
  display.print(char(247)); // degree symbol
  display.display();
}

//Function to show Hum
void showHumDisplay(Task* me) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(108,0);
  display.print("Hum");
  display.setTextSize(3);
  display.setCursor(24, 10);
  display.print(hum, 1);
  display.setTextSize(1);
  display.setCursor(96, 10);
  display.print("%"); // percent symbol
  display.display();
}

//Loop should not be needed (timers are doing all the work)
//void loop(){ 
   
  //hum = dht.readHumidity();
  //temp = dht.readTemperature();
  //fTemp = (temp * 9/5 + 32);
  //buttonState = digitalRead(buttonPin);
  //Print temp and humidity values to serial monitor
  //Serial.print("Humidity: ");
  //Serial.print(hum);
  //Serial.print(" %, Temp: ");
  //Serial.print(temp);
  //Serial.print(" Celsius");
  //Serial.println(fTemp);
  //if (buttonState == LOW) {
  //  Serial.println("Button was Pressed");
  //}
  
//   //text display tests
//  display.clearDisplay();
//  display.setTextSize(1);
//  display.setTextColor(WHITE);
//  display.setCursor(0,0);
//  display.print("Temperature");
//  display.setTextSize(3);
//  display.setCursor(24, 10);
//  display.print(fTemp, 1);
//  display.setTextSize(1);
//  display.setCursor(96, 10);
//  display.print(char(247)); // degree symbol
//  display.display();
//  delay(5000);
//
//  display.clearDisplay();
//
//  // now show humidity
//  display.setTextSize(1);
//  display.setCursor(0,0);
//  display.print("Humidity");
//  
//  display.setCursor(24, 10);
//  display.setTextSize(3);
//  display.print(hum, 1);
//  display.setTextSize(1);
//  display.setCursor(96, 10);
//  display.print("%");
//  display.display();
//  delay(5000);
//  display.clearDisplay();
//  
//  display.clearDisplay();
//  display.setTextColor(WHITE);
//  display.setTextSize(2);
//  display.setCursor(0,0);
//  display.setTextWrap(false);
//  display.print("Temperature: ");
//  display.print(fTemp, 1);
//  display.setTextSize(1);
//  display.print(char(247));
//  display.display();
//
//  display.startscrollleft(0x00, 0x0F);
//  delay(8000);
//}
