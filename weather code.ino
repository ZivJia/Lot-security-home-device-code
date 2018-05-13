#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Ethernet.h>
#include <stdint.h>
#include <SPI.h>
#include <Adafruit_SSD1306.h>

#include "RTClib.h"
RTC_DS1307 rtc;

#define OLED_RESET 16
Adafruit_SSD1306 display(OLED_RESET);


const char* ssid     = "The House";
const char* password = "";
String minTemp;
String maxTemp;
String curTemp;
String dayWeather;
String nightWeather;
String curWeather;
int displayState = 0;
int lastCurHour = -1;
bool shouldChange = false;

void setup() {
  // put your setup code here, to run once:
  attachInterrupt(digitalPinToInterrupt(12), changeDisplayIRS, RISING);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  Serial.begin(9600);
  Serial.println("ready");
  WiFi.begin(ssid, password);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  getWeather();
  getCurTemp();
  changeDisplay();

  DateTime now = rtc.now();
  Serial.println(String(now.hour()) + "/" + String(now.minute()) + "/" + String(now.second()));
}



void changeDisplayIRS(){
  delay(15);
  if (digitalRead(12) == HIGH){
    displayState = (displayState+1)%4;
    Serial.println(displayState);
    shouldChange = true;
    Serial.println("here");
    }
}

void getWeather(){
//   这个就是我们的http request，因为一天只能50此请求，所以现在用收到的结果进行代替。
  HTTPClient http;
  http.begin("http://dataservice.accuweather.com/forecasts/v1/daily/1day/349727?apikey=t1hXlQMwk2vDOqlOxyFTdZyu6sZbY98X");  //Specify request destination
  int httpCode = http.GET();                                                                  //Send the request
  String weather;
  if (httpCode > 0) { //Check the returning code
    weather = http.getString();   //Get the request response payload
    Serial.println(weather);                     //Print the response payload
  }
  http.end();
//  String weather = "{\"Headline\":{\"EffectiveDate\":\"2018-05-01T08:00:00-04:00\",\"EffectiveEpochDate\":1525176000,\"Severity\":4,\"Text\":\"Becoming much warmer Tuesday and Wednesday\",\"Category\":\"warmer\",\"EndDate\":\"2018-05-02T20:00:00-04:00\",\"EndEpochDate\":1525305600,\"MobileLink\":\"http://m.accuweather.com/en/us/new-york-ny/10007/extended-weather-forecast/349727?lang=en-us\",\"Link\":\"http://www.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/349727?lang=en-us\"},\"DailyForecasts\":[{\"Date\":\"2018-04-29T07:00:00-04:00\",\"EpochDate\":1524999600,\"Temperature\":{\"Minimum\":{\"Value\":42.0,\"Unit\":\"F\",\"UnitType\":18},\"Maximum\":{\"Value\":59.0,\"Unit\":\"F\",\"UnitType\":18}},\"Day\":{\"Icon\":4,\"IconPhrase\":\"Sunny\"},\"Night\":{\"Icon\":36,\"IconPhrase\":\"Rain\"},\"Sources\":[\"AccuWeather\"],\"MobileLink\":\"http://m.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/349727?day=1&lang=en-us\",\"Link\":\"http://www.accuweather.com/en/us/new-york-ny/10007/daily-weather-forecast/349727?day=1&lang=en-us\"}]}";
//  Serial.println(weather);
  int minTempStartInd = weather.indexOf("\"Minimum\"");
  minTemp = weather.substring(minTempStartInd+19, minTempStartInd+23);
  int maxTempStartInd = weather.indexOf("\"Maximum\"");
  maxTemp = weather.substring(maxTempStartInd+19, maxTempStartInd+23);
  int dayWeatherStartInd = weather.indexOf("\"Day\"");
  int numOfQuotes = 0;
  int spointer = 0;
  int epointer = 0;
  for(int i=dayWeatherStartInd; i < weather.length(); i++){
    if(weather[i] == '"'){
      numOfQuotes++;
      }
    if(numOfQuotes == 7 && spointer == 0){
      spointer = i+1;
      }
    if(numOfQuotes == 8 && epointer == 0){
      epointer = i;
      break;
      }
    }
  dayWeather = weather.substring(spointer, epointer);
  int nightWeatherStartInd = weather.indexOf("\"Night\"");
  numOfQuotes = 0;
  spointer = 0;
  epointer = 0;
  for(int i=nightWeatherStartInd; i < weather.length(); i++){
    if(weather[i] == '"'){
      numOfQuotes++;
      }
    if(numOfQuotes == 7 && spointer == 0){
      spointer = i+1;
      }
    if(numOfQuotes == 8 && epointer == 0){
      epointer = i;
      break;
      }
    }
  nightWeather = weather.substring(spointer, epointer);

  if (nightWeather.indexOf("Showers") > 0|| dayWeather.indexOf("Showers") > 0||
      nightWeather.indexOf("showers") > 0|| dayWeather.indexOf("showers") > 0||
      nightWeather.indexOf("Storms") > 0|| dayWeather.indexOf("Storms") > 0||
      nightWeather.indexOf("storms") > 0|| dayWeather.indexOf("storms") > 0||
      nightWeather.indexOf("Rain") >0 || dayWeather.indexOf("Rain") >0||
      nightWeather.indexOf("rain") >0 || dayWeather.indexOf("rain") >0||
      nightWeather.indexOf("Flurries") >0 || dayWeather.indexOf("Flurries") >0||
      nightWeather.indexOf("flurries") >0 || dayWeather.indexOf("flurries") >0||
      nightWeather.indexOf("Ice") >0 || dayWeather.indexOf("Ice") >0||
      nightWeather.indexOf("ice") >0 || dayWeather.indexOf("ice") >0||
      nightWeather.indexOf("Sleet") >0 || dayWeather.indexOf("Sleet")>0 ||
      nightWeather.indexOf("sleet") >0 || dayWeather.indexOf("sleet")>0 ||
      nightWeather == "Showers" || dayWeather == "Showers" ||
      nightWeather == "Rain" || dayWeather == "Rain" ||
      nightWeather == "Flurries" || dayWeather == "Flurries" ||
      nightWeather == "Snow" || dayWeather == "Snow" ||
      nightWeather == "Ice" || dayWeather == "Ice" ||
      nightWeather == "Sleet" || dayWeather == "Sleet"){
        Serial.println("warning");
        digitalWrite(15, HIGH);
        digitalWrite(16, LOW);
        }
    else{
      digitalWrite(16, HIGH);
      digitalWrite(15, LOW);
      }
 }

void getCurTemp(){
  HTTPClient http;
  http.begin("http://dataservice.accuweather.com/currentconditions/v1/349727?apikey=t1hXlQMwk2vDOqlOxyFTdZyu6sZbY98X");  //Specify request destination
  int httpCode = http.GET();                                                                  //Send the request
  String weather;
  if (httpCode > 0) { //Check the returning code
    weather = http.getString();   //Get the request response payload
    Serial.println(weather);                     //Print the response payload
  }
  http.end();
//  String weather = "[{\"LocalObservationDateTime\":\"2018-04-29T23:25:00-04:00\",\"EpochTime\":1525058700,\"WeatherText\":\"Cloudy\",\"WeatherIcon\":7,\"IsDayTime\":false,\"Temperature\":{\"Metric\":{\"Value\":8.9,\"Unit\":\"C\",\"UnitType\":17},\"Imperial\":{\"Value\":48.0,\"Unit\":\"F\",\"UnitType\":18}},\"MobileLink\":\"http://m.accuweather.com/en/us/new-york-ny/10007/current-weather/349727?lang=en-us\",\"Link\":\"http://www.accuweather.com/en/us/new-york-ny/10007/current-weather/349727?lang=en-us\"}]";
  int curTempStartInd = weather.indexOf("\"Imperial\"");
  curTemp = weather.substring(curTempStartInd+20, curTempStartInd+22);
  Serial.println(curTemp);

  int curWeatherStartInd = weather.indexOf("\"WeatherText\"");
  int numOfQuotes = 0;
  int spointer = 0;
  int epointer = 0;
  for(int i=curWeatherStartInd; i < weather.length(); i++){
    if(weather[i] == '"'){
      numOfQuotes++;
      }
    if(numOfQuotes == 3 && spointer == 0){
      spointer = i+1;
      }
    if(numOfQuotes == 4 && epointer == 0){
      epointer = i;
      break;
      }
    }
  curWeather = weather.substring(spointer, epointer);

  if (curWeather.indexOf("Showers") > 0|| 
      curWeather.indexOf("showers") > 0|| 
      curWeather.indexOf("Storms") > 0||
      curWeather.indexOf("storms") > 0||  
      curWeather.indexOf("Rain") >0 || 
      curWeather.indexOf("rain") >0 || 
      curWeather.indexOf("Flurries") >0 ||
      curWeather.indexOf("flurries") >0 ||
      curWeather.indexOf("Ice") >0 || 
      curWeather.indexOf("ice") >0 || 
      curWeather.indexOf("Sleet") >0 || 
      curWeather.indexOf("sleet") >0 || 
      curWeather == "Showers" || 
      curWeather == "Rain" || 
      curWeather == "Flurries" || 
      curWeather == "Snow" || 
      curWeather == "Ice" || 
      curWeather == "Sleet"){
        Serial.println("warning");
        digitalWrite(15, HIGH);
        digitalWrite(16, LOW);
        }
  
  }

void loop() {
  DateTime now = rtc.now();
  Serial.println(String(now.hour()) + "/" + String(now.minute()) + "/" + String(now.second()));
  
  if (lastCurHour != now.hour()){
    getWeather();
    getCurTemp();
    changeDisplay();
    lastCurHour = now.hour();
    }

  
  if(shouldChange == true){
    Serial.println("change state");
    changeDisplay();
    }
}

void changeDisplay(){
  if(displayState == 0){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.println("Current weather:"+ curTemp + ".0F");
    display.setCursor(0,12);
    display.println(curWeather);
    display.display();
    delay(20);
    }
  
  if(displayState == 1){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.println("MinTemp:" + minTemp + "F");
    display.setCursor(0,20);
    display.println("MaxTemp:" + maxTemp + "F");
    display.display();
    delay(20);
    }

   if(displayState == 2){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(WHITE);
    display.println("Day Light Weather:");
    display.setCursor(0,12);
    display.println(dayWeather);
    display.display();
    delay(20);
    }

  if(displayState == 3){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Night Light Weather:");
    display.setCursor(0,12);
    display.println(nightWeather);
    display.display();
    delay(20);
    }
   shouldChange = false; 
  }
