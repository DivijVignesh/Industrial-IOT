#include <Filters.h> //Easy library to do the calculations
#include<ThingSpeak.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

/*---------------------CREDENTIALS NEEDED TO BE ENTERED ---------------------------------------*/
String apiKey = "O4IBMWYI7KUVZGR8";     //  Enter your Write API key from ThingSpeak
const char* ssid =  "Maple Software";     // Enter your WiFi Network's SSID
const char* pass =  "22221111"; // Enter your WiFi Network's Password
const char* server = "api.thingspeak.com";
/*---------------------**************--------------------------------------------------------*/
//For accelerometer values
#include <Wire.h>
#include <Adafruit_Sensor.h> 
#include <Adafruit_ADXL345_U.h>
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified();
#define S0 D5                             /* Assign Multiplexer pin S0 connect to pin D5 of NodeMCU */
#define S1 D6                             /* Assign Multiplexer pin S1 connect to pin D6 of NodeMCU */
#define S2 D7                             /* Assign Multiplexer pin S2 connect to pin D7 of NodeMCU */
#define S3 D8                             /* Assign Multiplexer pin S3 connect to pin D8 of NodeMCU */
#define SIG A0  
#define CTSENSORCurrent 30
int decimal = 2;                          /* Decimal places of the sensor value outputs */
int sensor0;                            /* for Channel C0 */
int sensor1;                            /* For channel c5 */
int sensor2,sensor3;                    /* For channel c15 and c1 */
double amps=-1,amps1,volt1;
double analog,peakI=0,rmsI,rmsI1,rmsI2;
//For Voltage Sensor
float testFrequency = 50;                     // test signal frequency (Hz)
float windowLength = 40.0/testFrequency;     // how long to average the signal, for statistist

int Sensor = 0; //Sensor analog input, here it's A0

float intercept = -0.04; // to be adjusted based on calibration testing
float slope = 0.0405; // to be adjusted based on calibration testing 0.0405
float current_Volts,RMSvoltage; // Voltage
float xaxis,yaxis,zaxis;

//Http clients
WiFiClient client;
 
HTTPClient http;    //Declare object of class HTTPClient

//Voltge sensor
  RunningStatistics inputStats; 
void setup()
{
  //Wifi Connection
      Serial.print("Connecting to: ");
      Serial.println(ssid);
 
 
      WiFi.begin(ssid, pass);
 
      while (WiFi.status() != WL_CONNECTED) 
     {
            delay(100);                         //iterates in this loop till there is a secure connection made between the board and the local wifi network
            Serial.print("*");
     }
      Serial.println("");
      Serial.println("***WiFi connected***");
  //For Voltage Sensor
              //Easy life lines, actual calculation of the RMS requires a load of coding

      
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  pinMode(SIG,INPUT);
   if(!accel.begin())
   {
      Serial.println("No valid sensor found");
      while(1);
   }
  Serial.begin(115200);

}

void loop()
{

  //For Current Sensor
    // C1 pin - binary codj 1,0,0,0
    digitalWrite(S0,HIGH); 
    digitalWrite(S1,LOW);
    digitalWrite(S2,LOW); 
    digitalWrite(S3,LOW);
    sensor0 = analogRead(SIG);
    int j;
    float avgI;
    for(int i=1;i<=10;i++)
    {
      j=analogRead(A0);
      if(peakI<j)
      peakI=j;
      delay(1);   

      }
    rmsI=((peakI/1023)*CTSENSORCurrent*3.3)/1.414;

    peakI=0;
    Serial.print("Amps 1:     ");
    Serial.print(rmsI); 
    Serial.print("RMS  ");
    Serial.println();
    delay(10);
    if(rmsI<1.2||rmsI>30)
    rmsI=0.0;
    // C2 pin - binary codj 0,1,0,0
    digitalWrite(S0,LOW); 
    digitalWrite(S1,HIGH);
    digitalWrite(S2,LOW); 
    digitalWrite(S3,LOW);
    sensor1 = analogRead(SIG);
    for(int i=1;i<=10;i++)
    {
      j=sensor1;
      if(peakI<j)
      peakI=j;
      delay(1);    
      }
    rmsI1=((peakI/1023)*CTSENSORCurrent*3.3)/1.414;
    peakI=0;
    Serial.print("Amps 2:     ");
    Serial.print(rmsI1); 
    Serial.print("RMS  ");
    Serial.println();
    delay(10);
    if(rmsI1<1.2||rmsI1>30)
    rmsI1=0.0;

    // C3 pin - binary codj 1,1,0,0
    digitalWrite(S0,HIGH);
    digitalWrite(S1,HIGH);
    digitalWrite(S2,LOW); 
    digitalWrite(S3,LOW);
    sensor2 = analogRead(SIG);
    for(int i=1;i<=10;i++)
    {
      j=analogRead(A0);
      if(peakI<j)
      peakI=j;
      delay(1);    
      }
    rmsI2=((peakI/1023)*CTSENSORCurrent*3.3)/1.414;
    peakI=0;
    Serial.print("Amps3:     ");
    Serial.print(rmsI2); 
    Serial.print("RMS  ");
    Serial.println();
    delay(10);
    if(rmsI2<1.2||rmsI2>30)
    rmsI2=0.0;
       
    //For Voltage Sensor
      inputStats.setWindowSecs( windowLength );
    //C0 pin binary code 0,0,0,0
    digitalWrite(S0,LOW); digitalWrite(S1,LOW); digitalWrite(S2,LOW); digitalWrite(S3,LOW);
    float volt[10];
    for(int i=1;i<90;i++){
    sensor3 = analogRead(SIG);
    inputStats.input(sensor3);
      
      current_Volts = intercept + slope * inputStats.sigma(); //Calibartions for offset and amplitude
      current_Volts= current_Volts*(40.3231);                //Further calibrations for the amplitude
      volt1=current_Volts;
//      Serial.println(current_Volts);
      delay(10);
    }
      Serial.print( "Voltage:     " );
      Serial.print( (float)volt1 );
      Serial.println();
    if(volt1<5)
    volt1=0.0;
    //For Accelerometer

   sensors_event_t event; 
   accel.getEvent(&event);
   Serial.println("Accelerometer Values");
   Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print("  "); xaxis=event.acceleration.x;
   Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print("  "); yaxis=event.acceleration.y;
   Serial.print("Z: "); Serial.print(event.acceleration.z); Serial.print("  "); zaxis=event.acceleration.z;
   Serial.println("m/s^2 ");

   Serial.println();
   uploadData();
   uploadDataForCurrent();
   uploadDataForCurrent2();
   uploadDataForCurrent3();
   uploadDataForAccelerometer();
   delay(15000);// repeats after 15s
    
}
void uploadData()
{
 String url="https://api.thingspeak.com/update?api_key=R2P1UNETVGPMV9BA&field1="+(String)volt1;//+"&field2=''";      //http request url
          http.begin(url,"27:18:92:DD:A4:26:C3:07:09:B9:7A:E6:C5:21:B9:5B:48:F7:16:E1");     // begin the http connection
         http.addHeader("Content-Type", "text/plain");  //Specify content-type header
       
         int httpCode = http.GET();   //Send the request
         String payload = http.getString();                  //Get the response payload
       
         Serial.print("http code voltage sensor:"+httpCode);   //Print HTTP return code
         Serial.print("entry number voltage sensor"+payload);    //Print request response payload
         Serial.print("\n"); 
         http.end();  //Close connection
         RMSvoltage=1.1;

}
void uploadDataForCurrent()
{
   String url="https://api.thingspeak.com/update?api_key=9R394OVE47WLDGTD&field1="+(String)rmsI;//+"&field2=''";      //http request url
          http.begin(url,"27:18:92:DD:A4:26:C3:07:09:B9:7A:E6:C5:21:B9:5B:48:F7:16:E1");     // begin the http connection
         http.addHeader("Content-Type", "text/plain");  //Specify content-type header
       
         int httpCode = http.GET();   //Send the request
         String payload = http.getString();                  //Get the response payload
       
         Serial.print("http codect sensor 1"+httpCode);   //Print HTTP return code
         Serial.print("entry numberct sensor 1"+payload);    //Print request response payload
         Serial.print("\n"); 
         http.end();  //Close connection
}
void uploadDataForCurrent2()
{
   String url="https://api.thingspeak.com/update?api_key=O4IBMWYI7KUVZGR8&field1="+(String)rmsI1;//+"&field2=''";      //http request url
          http.begin(url,"27:18:92:DD:A4:26:C3:07:09:B9:7A:E6:C5:21:B9:5B:48:F7:16:E1");     // begin the http connection
         http.addHeader("Content-Type", "text/plain");  //Specify content-type header
       
         int httpCode = http.GET();   //Send the request
         String payload = http.getString();                  //Get the response payload
       
         Serial.print("http codect sensor 2:"+httpCode);   //Print HTTP return code
         Serial.print("entry number ct sensor 2:"+payload);    //Print request response payload
         Serial.print("\n"); 
         http.end();  //Close connection
}
void uploadDataForCurrent3()
{
   String url="https://api.thingspeak.com/update?api_key=7YC9TNCQFW2CZ8JO&field1="+(String)rmsI2;//+"&field2=''";      //http request url
          http.begin(url,"27:18:92:DD:A4:26:C3:07:09:B9:7A:E6:C5:21:B9:5B:48:F7:16:E1");     // begin the http connection
         http.addHeader("Content-Type", "text/plain");  //Specify content-type header
       
         int httpCode = http.GET();   //Send the request
         String payload = http.getString();                  //Get the response payload
       
         Serial.print("http code ct sensor 3:"+httpCode);   //Print HTTP return code
         Serial.print("entry number ct sensor 3:"+payload);    //Print request response payload
         Serial.print("\n"); 
         http.end();  //Close connection
}
void uploadDataForAccelerometer()
{
     String url="https://api.thingspeak.com/update?api_key=5Y252W2RDINO042Z&field1="+(String)xaxis+"&field2="+(String)yaxis+"&field3="+(String)xaxis;      //http request url
          http.begin(url,"27:18:92:DD:A4:26:C3:07:09:B9:7A:E6:C5:21:B9:5B:48:F7:16:E1");     // begin the http connection
         http.addHeader("Content-Type", "text/plain");  //Specify content-type header
       
         int httpCode = http.GET();   //Send the request
         String payload = http.getString();                  //Get the response payload
       
         Serial.print("http code:"+httpCode);   //Print HTTP return code
         Serial.print("entry number:"+payload);    //Print request response payload
         Serial.print("\n"); 
         http.end();  //Close connection
}
