#include <SoftwareSerial.h>
#include <HCSR04.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <RTClib.h>
RTC_DS1307 RTC;
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
// Set this to true when you want to remap backticks to ESC (for testing the
// send text functionality).
// Specify that we're using digital pins 7 and 8 for software serial. These
// two pins connect the Arduino to the SIM900.
SoftwareSerial net(7, 8);
UltraSonicDistanceSensor distanceSensor(10, 11);  // Initialize sensor that uses digital pins 5 and 6.
char val;    // The current byte read from SIM900.  
char buf[128];  // Holds contents read from SIM900.
int lastMinute;
bool timeShould = false;
bool safeflag = false;
bool securflag = true;
bool safeEnable = true;
bool securEnable= false;
bool safemodeset = false;
bool securmodeset = false;
bool warning = false;
unsigned long sentStart;
void setup() {
  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), safeIRS, RISING);
  pinMode(3, INPUT);
  attachInterrupt(digitalPinToInterrupt(3), securIRS, RISING);
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  Serial.begin(19200);
  net.begin(19200);
  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  RTC.begin();
  RTC.adjust(DateTime(__DATE__, __TIME__));
  while (! RTC.isrunning()) {
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(3,3);
    display.println("Check RTC");
    display.display();
    delay(100);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(3,3);
  display.println("Initializing");
  display.display();
  delay(100);
  display.clearDisplay();
   net.println("AT");
  delay(100);
  while (net.available()) {
    val = net.read();
  }
  net.println("AT+E=0");
  delay(100);
  while (net.available()) {
    val = net.read();
  }
  net.println("AT+CMGF=1");
  delay(100);
  while (net.available()) {
    val = net.read();
  }
  net.println("AT+CREG=1");
  delay(100);
  while (net.available()) {
    val = net.read();
  }
  char temp = '0';
  while (temp != '1')
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(3,3);
    display.println("SIM not connected");
    display.display();
    delay(100);
    display.clearDisplay();
    net.println("AT+CREG?");
    delay(100);
    while (read_line(buf, sizeof(buf), 500)) {
    if (buf[0]=='+')
    {
      temp = buf[9];
     }
    }
    Serial.println("555555");
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,3);
  DateTime now = RTC.now();
  display.print("Current Time: ");
  display.print(now.hour());
  display.print(':');
  lastMinute = now.minute();
  display.println(now.minute());
  display.setCursor(3,20);
  display.print("Date: ");
  display.print(now.month());
  display.print('/');
  display.print(now.day());
  display.print('/');
  display.println(now.year());
  display.display();
  delay(100);
  display.clearDisplay();

}

void loop() {
  digitalWrite(12,LOW);
  digitalWrite(13,LOW);
  noTone(9);
  DateTime now = RTC.now();
  if (lastMinute != now.minute() )
  { updateTime();}
  if (safeflag){
    noTone(9);
    digitalWrite(12,LOW);
    digitalWrite(13,HIGH);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(3,3);
    display.print("Save mode: ");
    display.display();
    delay(2000);
    display.clearDisplay();
    updateTime();
    while(!securflag)
    {
      savemode();
      checkIncome();
      }
    }
    if(securflag)
    {
      noTone(9);
      digitalWrite(12,HIGH);
      digitalWrite(13,LOW);
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(3,3);
      display.print("Security mode: ");
      display.display();
      delay(2000);
      display.clearDisplay();
      updateTime();
      while(!safeflag)
      {
        securmode();
        checkIncome();
        Serial.println("fuck!!!!");
        Serial.println(distanceSensor.measureDistanceCm());
        Serial.println("......");
        if((distanceSensor.measureDistanceCm()<20)&&(distanceSensor.measureDistanceCm()>0))
        {
          delay(500);
          if((distanceSensor.measureDistanceCm()<20)&&(distanceSensor.measureDistanceCm()>0))
        {
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0,3);
          display.print("Warning someone break in!!!");
          display.display();
          Serial.println(distanceSensor.measureDistanceCm());
          warning = true;
          sending();
          warning = false;
          int count =0;
          safeEnable = false;
 
          while(!safeflag)
          {
            tone(9,1000,500);
            checkIncome();
            if(count%2==0)
            {
              digitalWrite(12,HIGH);
              }
              else
              {
                digitalWrite(12,LOW);
                }
                delay(500);    
                count++;
            
            }
            updateTime();
            
        }
        }
        }
      }
  if (timeShould){
    updateTime();
    Serial.println("updated");
    timeShould = false;
    }
}

void safeIRS(){
  if (safeEnable){
    Serial.println("safe pressed");
    safeflag = true;
    securEnable = true;
    safeEnable = false;
    securflag = false;
    }
  }
  

void securIRS(){
if (securEnable){
  Serial.println("secur pressed");
  securflag = true;
  securEnable = false;
  safeEnable = true;
  safeflag = false;
  }
}
void savemode(){
  DateTime now = RTC.now();
   if (lastMinute != now.minute())
  { updateTime();}  
    }
void securmode(){
  DateTime now = RTC.now();
   if (lastMinute != now.minute())
  { updateTime();}  
    }

void checkIncome()
{
  Serial.println("666666");
  bool found = false;
  while(net.available() > 0) {
    val = net.read();
  }
  net.println("AT+CMGL=\"ALL\"");
  delay(100);
  while (read_line(buf, sizeof(buf), 500)) {
    if (found){
      
      char *safec = strstr(buf,"safemode");
      char *secur = strstr(buf,"securitymode");
      if(safec&&!secur)//&&safeEnable
      {
        safeflag = true;
        securflag = false;
        safeEnable = false;
        securEnable = true;
        safemodeset = true;
        net.println("AT+CMGD=1,4");
        delay(100);
        updateTime(); 
        sending();
        }
      
      if(secur&&!safec&&securEnable)
      {
        safeflag = false;
        securflag = true;
        safeEnable = true;
        securEnable = false;
        securmodeset = true;
        delay(100);
        net.println("AT+CMGD=1,4");
        delay(100); 
        updateTime();
        sending();
        }
        
      found = false;
     
    }
    if (buf[0] == '+' && buf[3] == 'G'){
      char *number = strstr(buf,"***********");// user's phone number
        if(number){
          found = true;
          }

      }
  }
  while(net.available() > 0) {
    val = net.read();
  }
 }
 
bool read_line(char* buf, int buflen, int timeout_ms) {
  buf[0] = 0;
  int current_offset = 0;
  unsigned long start = millis();
  while (true) {
    if (millis() - start > timeout_ms) return false;
    if (net.available()) {
      char current_char = net.read();
      buf[current_offset++] = current_char;
      buf[current_offset] = 0;
      if (current_offset >= buflen - 1) return false;
      if (current_char == '\n') {
        if (current_offset >= 2 && buf[current_offset - 2] == '\r') {
          buf[current_offset - 1] = 0;
          buf[current_offset - 2] = 0;
        }
        return true;
      }
    }
  }
  return false;
}


void updateTime(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,3);
  DateTime now = RTC.now();
  display.print("Current Time: ");
  display.print(now.hour());
  display.print(':');
  lastMinute = now.minute();
  display.println(now.minute());
  display.setCursor(3,20);
  display.print("Date: ");
  display.print(now.month());
  display.print('/');
  display.print(now.day());
  display.print('/');
  display.println(now.year());
  display.display();
  delay(100);
  display.clearDisplay();
  }
  void sending(){
    sentStart = millis();
    net.println("AT");
    delay(100);
    net.println("AT+CMGS=\"+***********\"");// user's phone number
    delay(100);
    while (net.available()) {
    val = net.read();
    }
    if (safemodeset)
    {
    net.print("Safe mode setting successful.");
    safemodeset = false;
    }
    if (securmodeset)
    {
    net.print("Security mode setting successful.");
    securmodeset = false;
      }
    if (warning)
    {
    net.print("Warning someone break in!!!.");
    warning = false;
    }
    net.write(0x1A);
    while (net.available()) {
    val = net.read();
    }
  }
