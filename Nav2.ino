#include <Adafruit_NeoPixel.h>
#include <SparkFun_APDS9960.h>
#include<SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <HMC5883L.h>
#include <U8g2lib.h>
#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#define PIN 6  // Pin NeoWheel is on
#define APDS9960_INT 2 //interupt pin
char Serialdata;
HMC5883L compass;
// Initiate Display ///////////////////////////////////////////////////////////////////////////////////////////
U8G2_SSD1306_128X64_NONAME_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 10, /* data=*/ 9, /* cs=*/ U8X8_PIN_NONE, /* dc=*/ 11, /* reset=*/ 13);
// Initiate NeoPixel ///////////////////////////////////////////////////////////////////////////////////////////
Adafruit_NeoPixel strip = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);
//  Assign strip to heading directions
const int NW60=11;
const int NW30=10;
const int W0=9 ;
const int SW60=8 ;
const int SW30=7 ;
const int S0=6 ;
const int SE60 =5;
const int SE30=4 ;
const int E0=3 ;
const int NE60=2 ;
const int NE30 =1;
const int N0 =0 ;
// variable and initial value for "North" indicator
int indicator =0;
String data; 
float heading;
float headingDegrees;
//APDS///////////////////////////////////////////////////////////////////////////////////////////
#define LIGHT_INT_HIGH  1000 // High light level for interrupt
#define LIGHT_INT_LOW   10   // Low light level for interrupt
int isr_flag = 0;
uint16_t ambient_light = 0;
uint16_t threshold = 0;
SparkFun_APDS9960 apds = SparkFun_APDS9960();
///////////////////////////////////////////////////////////////////////////////////////////
// communication hardware
SoftwareSerial mySerial(3,4); //(tx,rx)

void checkAmbient()
{
if ( isr_flag == 1 ) {
if(!apds.readAmbientLight(ambient_light)){
     Serial.println("Error reading light values");
   }
   else{
 //   Serial.print("Interrupt! Ambient: ");
 //   Serial.println(ambient_light);
   }
}
if(ambient_light>=1000){
      strip.setBrightness(100);
}
else  if(ambient_light>400 && ambient_light>1000){
   strip.setBrightness(50); 
}
else{
    strip.setBrightness(10);
}
}
///////////////////////////////////////////////////////////////////////////////////////////
void setup(void) 
{
  strip.begin();
  strip.show(); // Initialize all strip to 'off'
 pinMode(APDS9960_INT, INPUT);
 attachInterrupt(0, isr_flag = 1, FALLING);
   Serial.begin(9600);
  mySerial.begin(9600);
  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Set high and low interrupt thresholds
  if ( !apds.setLightIntLowThreshold(LIGHT_INT_LOW) ) {
    Serial.println(F("Error writing low threshold"));
  }
  if ( !apds.setLightIntHighThreshold(LIGHT_INT_HIGH) ) {
    Serial.println(F("Error writing high threshold"));
  }
  
  // Start running the APDS-9960 light sensor (no interrupts)
  if ( apds.enableLightSensor(false) ) {
    Serial.println(F("Light sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during light sensor init!"));
  }

  // Read high and low interrupt thresholds
  if ( !apds.getLightIntLowThreshold(threshold) ) {
    Serial.println(F("Error reading low threshold"));
  } else {
    Serial.print(F("Low Threshold: "));
    Serial.println(threshold);
  }
  if ( !apds.getLightIntHighThreshold(threshold) ) {
    Serial.println(F("Error reading high threshold"));
  } else {
    Serial.print(F("High Threshold: "));
    Serial.println(threshold);
  }
 u8g2.begin(); 
 u8g2.clearBuffer();  
 drawIntro();
 delay(2000);
  while (!compass.begin())
  {
    Serial.println("Could not find a valid HMC5883L sensor, check wiring!");
    delay(500);
    while(1);
  }
  if(compass.begin())
  {
    // one cycle of red around the ring to signal we're good to go!
     Start();
  // Set measurement range
  compass.setRange(HMC5883L_RANGE_1_3GA);
  compass.setMeasurementMode(HMC5883L_CONTINOUS);
  compass.setDataRate(HMC5883L_DATARATE_30HZ);
  compass.setSamples(HMC5883L_SAMPLES_8);
  compass.setOffset(0,0); // 0,0 default
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
void loop(void) 
{
checkAmbient();
Vector norm = compass.readNormalize();
  // Calculate heading
  float heading = atan2(norm.YAxis, norm.XAxis);
// Set declination headingDegrees on your location and fix heading
  // You can find your declination on: http://magnetic-declination.com/
  // (+) Positive or (-) for negative
  // For Bytom / Mumbai declination headingDegrees is 0.18 (positive)
  // Formula: (deg + (min / 60.0)) / (180 / M_PI);
float declinationheadingDegrees = (0.18 - (26.0 / 60.0)) / (180 / M_PI);
  heading += declinationheadingDegrees;
  // Normalize to 0-360
  if (heading < 0)
  {
  heading += 2 * PI;
  }
  
  if (heading > 2 * PI)
  {
    heading -= 2 * PI;
  }
  headingDegrees = heading * 180/M_PI; 
//  Serial.print(" Degress = ");
//  Serial.println(headingDegrees);
 // use heading to set & show Neostrip on ring
  BLEData();
  setIndicator();
    u8g2.firstPage();  
  do {
  drawCompass();
  } while( u8g2.nextPage() );
  showIndicator();
  delay(50);    // just to let things settle a bit
}
///////////////////////////////////////////////////////////////////////////////////////////
void setIndicator(){ 
  if ((headingDegrees >= 0.25)&&(headingDegrees < 90.25));  // NNE to E 
  {
      if ((headingDegrees >= 0.25) && (headingDegrees < 33.75))
    {
      indicator = NE30;
     // Serial.println("NE30");
      data="NE30";
    } 
      if ((headingDegrees >= 33.75) && (headingDegrees < 60.25))
    {
      indicator = NE60;
    //  Serial.println("NE60");
           data="NE60";
    }  
    if ((headingDegrees >= 60.25) && (headingDegrees < 90.25))
    {
      indicator = E0;
    //Serial.println("E0");
         data="E0";
    }  
  }    //end if NNE to E

    if ((headingDegrees >= 90.25) && (headingDegrees < 180.25))    // ESE to S
  {
      if ((headingDegrees >= 90.25) && (headingDegrees < 120.75))
    {
      indicator = SE30;    
     // Serial.println("SE30");
           data="SE30";
    }  

      if ((headingDegrees >= 120.75) && (headingDegrees < 150.25))
    {
      indicator = SE60;
      //Serial.println("SE60");
           data="SE60";
    }   
    if ((headingDegrees >= 150.25) && (headingDegrees < 180.25))
    {
      indicator = S0;
     // Serial.println("S0");
           data="S0";
    }   
  }    //end if ESE to S
  if ((headingDegrees < 270.25) && (headingDegrees > 180.25))    // SSW to W
  {

    if ((headingDegrees >= 180.25) && (headingDegrees < 210.75))
    {
      indicator = SW30;
      //Serial.println("SW30");
           data="SW30";
    }  

      if ((headingDegrees >= 210.75) && (headingDegrees < 240.25))
    {
      indicator = SW60;
     // Serial.println("SW60");
    }   

      if ((headingDegrees >= 240.25) && (headingDegrees < 270.25))
    {
      indicator = W0;
     // Serial.println("W0");
           data="W0";
    }  //end if WSW
  }
     //end if SSW to W

    if ((headingDegrees >= 270.25) || (headingDegrees < 0.25))    // WNW to N
  {
    if ((headingDegrees >= 270.25) && (headingDegrees < 300.75))
    {
      indicator = NW30;
     // Serial.println("NW30");
           data="NW30";
    }    //end if WNW

      if ((headingDegrees >= 300.75) && (headingDegrees < 330.25))
    {
      indicator = NW60;
     // Serial.println("NW60");
           data="NW60";
    }  //end if NW

      if ((headingDegrees >= 330.25) || (headingDegrees < 0.25))
    {
      indicator = N0;
     // Serial.println("N0");
           data="N0";
    }   //end if N
  }  // end if WNW to N
}  
///////////////////////////////////////////////////////////////////////////////////////////
void showIndicator()
{
  // set a little border to highlight the North indicator
  int indicatorLeft = indicator - 1;
  int indicatorRight = indicator + 1;
  // scale / normalize to 0 - 11
  if (indicatorLeft < 0)
  {
    indicatorLeft += 12; 
  }    
  if (indicatorRight > 11)
  {
    indicatorRight -= 12; 
  }    
  colorWipe(strip.Color(0, 0, 255), 0);             //set All Blue (background dial color)
  strip.setPixelColor(indicator, 255, 255, 255);        // set indicator White
  strip.setPixelColor(indicatorLeft, 0, 255, 0);    // set indicator border GREEN
  strip.setPixelColor(indicatorRight, 0, 255, 0);   // set indicator border GREEN
  strip.show();                                   // Push bits out!
}  
///////////////////////////////////////////////////////////////////////////////////////////
void Start()
{
   strip.clear();
   checkAmbient();
   StartrainbowCycle(); 
}

void StartrainbowCycle()
{
    strip.clear();
  uint16_t i, j;
   
  for(j=0; j<256; j++) //1 Cycle
  {                   
    for(i=0; i< strip.numPixels(); i++) 
    {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(5);
  }

}
uint32_t Wheel(byte WheelPos) 
{
   
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);// end void showIndicator()
    strip.show();
    delay(wait);
  }
}
///////////////////////////////////////////////////////////////////////////////////////////
void drawCompass() {
static int armLength = 20;
  static int cx = 80;
  static int cy = 20;
  int armX, armY;
  
  //convert degree to radian
  float bearingRad = PI * (float) headingDegrees / 180.0;
  armX = armLength*cos(bearingRad);
  armY = -armLength*sin(bearingRad);

  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.setCursor(0, 60);
  u8g2.print("Compass : ");
  u8g2.setCursor(80, 60);
  u8g2.print(headingDegrees);
  u8g2.setCursor(0,20);
  u8g2.print(data);
  u8g2.drawLine(cx, cy, cx-armX, cy-armY);
  u8g2.drawCircle(cx, cy, armLength, U8G2_DRAW_ALL);
}
///////////////////////////////////////////////////////////////////////////////////////////
void drawIntro() {
u8g2.firstPage(); // draw intro
do {
u8g2.setFont(u8g2_font_ncenB14_tr);
u8g2.setCursor(10,40);
u8g2.print("SmartNav");
 u8g2.sendBuffer();
} while ( u8g2.nextPage() );
}
///////////////////////////////////////////////////////////////////////////////////////////
void BLEData()
{
  while(mySerial.available())
{
Serialdata = mySerial.read();
}
 switch(Serialdata)
{
  case '0':
  Serial.println("CLEAR");
  for (int i=0; i<12;i++)
{
  strip.clear();
  strip.show();
}
  break;
  case '1':
  Serial.println("Start");
  Start();
  break;
  case '2':
  Serial.println("Forward");
  forward();
  break;
  case '3':
  Serial.println("RIGHT");
  Right();
  break;
  case '4':
  Serial.println("LEFT");
  left();
  break;
  case '5':
  Serial.println("UTurn");
  uTurn();
  break;
  case '6':
  Serial.println("Start");
  DestinationReach();
  break;
}
}
void forward()
{
  strip.clear();
for (int i=3; i<=9;i++)
{
  strip.setPixelColor(i, strip.Color(0,250,0)); // Moderately bright green color.
  strip.show();
}
}

void uTurn()
{
  strip.clear();
for (int i=9; i<=11;i++)
{
  for(int j=0;j<=3;j++)
  {
  strip.setPixelColor(i, strip.Color(250,0,0)); // bright red color.
  strip.setPixelColor(j, strip.Color(250,0,0));
  strip.show();
}
}
delay(500);
for (int i=9; i<=11;i++)
{
  for(int j=0;j<=3;j++)
  {
  strip.setPixelColor(i, strip.Color(0,0,0)); 
  strip.setPixelColor(j, strip.Color(0,0,0));
  strip.show();
}
}
delay(500);
}
void Right()
{
  strip.clear();
for(int i =6;i<=12;i++)
{
  strip.setPixelColor(i, strip.Color(0,250,0)); // bright green color.
  strip.setPixelColor(0, strip.Color(0,250,0)); // bright green color.
  strip.show();
}

 strip.setPixelColor(9, strip.Color(250,250,250)); // bright green color.
  delay(500);
  strip.show();
  strip.setPixelColor(9, strip.Color(0,0,0)); // bright green color.
  delay(500);
  strip.show();
}
void left()
{
strip.clear();
  for(int i =0;i<=3;i++)
  {
  for(int j=4;j<=6;j++)
{
strip.setPixelColor(i, strip.Color(0,250,0)); // bright green color.
strip.setPixelColor(j, strip.Color(0,250,0)); // bright green color.
strip.show();
}
}
  strip.setPixelColor(3, strip.Color(250,250,250)); // bright green color.
  delay(500);
  strip.show();
  strip.setPixelColor(3, strip.Color(0,0,0)); // bright green color.
  delay(500);
  strip.show();
}

void DestinationReach()
{
  strip.clear();
 for(int i=0;i<=12;i++)
{
strip.setPixelColor(i, strip.Color(180,180,180)); // bright green color.
strip.show();
}
delay(250);
 for(int i=0;i<=12;i++)
{
strip.setPixelColor(i, strip.Color(0,0,0)); // bright green color.
strip.show();
}
delay(250);
}

