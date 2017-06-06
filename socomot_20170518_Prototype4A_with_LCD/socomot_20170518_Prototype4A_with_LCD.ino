#include <SD.h>
#include <SPI.h>
#include <Bridge.h>
#include <Wire.h>

File myFile;
File userFile;
bool tempData = false;

#define PowerModeSwitch A0
#define MoistureSensor A1
int start = 0;
//*********************************************************************************************************************************************************************
//        int sensorDN = analogRead(Sensor1);
//        double sensorVoltage = sensorDN*(3.0 / 1023.0);
//
//        //int sensor1DN = analogRead(Sensor2);
//        //float sensorVoltage = sensor1DN*(3.0 / 1023.0);
        float VWC;
        
        double analogValue;
        double analogValue_sd;
        double voltage;
        double voltage_sd;
        //double VWC;
        double VWC_sd;
//*********************************************************************************************************************************************************************

const int chipSelect = 49;

//********************************* GPS Data **************************************************
#include <call.h>
#include <gps.h>
#include <GSM.h>
#include <HWSerial.h>
#include <inetGSM.h>
#include <LOG.h>
#include <SIM900.h>
#include <sms.h>
#include <Streaming.h>
#include <WideTextFinder.h>
#include <LiquidCrystal_I2C.h>

#include <TinyGPS.h>
TinyGPS gps;

#define GPS_TX_DIGITAL_OUT_PIN 7
#define GPS_RX_DIGITAL_OUT_PIN 6

long startMillis;
long secondsToFirstLocation = 0;

#define DEBUG
// define the date and time
int year = 0;
byte month = 0;
byte day = 0;
byte hour = 0;
byte minute = 0;
byte second = 0;
byte hundredths = 0;

// define the location 
float latitude = 0.0;
float longitude = 0.0;
float altitude = 0.0;

// define the speed, satellites and precision
float speed_mps = 0.0;
float satellites = 0.0;
float hdop = 0.0;

//********************************* SMS Data ********************************************
#include <call.h>
#include <gps.h>
#include <GSM.h>
#include <HWSerial.h>
#include <inetGSM.h>
#include <LOG.h>
#include <SIM900.h>
#include <sms.h>
#include <Streaming.h>
#include <WideTextFinder.h>
#include <string.h>


//
float percentage;

// define SMS 
char sms_rx[122]; //Received text SMS
byte type_sms=SMS_ALL;      //Type of SMS
byte del_sms=1;                //0: No deleting sms - 1: Deleting SMS
char number_incoming[20];
char data_sms[140];

SMSGSM sms;
int error;
boolean started=false;
bool newData = false;

unsigned long starttime;
unsigned long sampletime_ms = 30000;

char sms_year[4];
char sms_mon[3];
char sms_day[3];
char sms_hour[3];
char sms_min[3];
char sms_sec[3];
char sms_lon[12];
char sms_lat[12];
char sms_datalog[2];;

//************************* Define the thingspeak information *****************************
InetGSM inet;

char msg[50];
int numdata;
char inSerial[50];
int i = 0;
long channelId = 275218;
char datastreamId[] = "1";
bool GPSworks = false;
int failtimes = 0;
int datalog = 0; 

// ThingSpeak Channel: Socomot Prototyping channel 4b
char thingSpeakAddress[] = "184.106.153.149";
char writeAPIKey[] = "KBBWMH3RI0HFM1C1";
char sentMsg[50];
int value1;
int value2;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address

//************************************************************************************************
void setup()
{ 
        Serial.begin(9600);
        Serial.println("Pre-Charge Mode(Wait 1 Minutes)");
        delay(60000);
        
        //**********Initialize GPS function********//
        // Serial2 is GPS
        Serial2.begin(9600);
        pinMode(PowerModeSwitch, OUTPUT);
        pinMode(MoistureSensor, INPUT);
         lcd.begin(20,4);  
            for(int i = 0; i< 3; i++)
            {
              lcd.backlight();
              delay(250);
              lcd.noBacklight();
              delay(250);
            }
          lcd.backlight(); 
            
          digitalWrite(PowerModeSwitch, HIGH);
      
          pinMode(GPS_TX_DIGITAL_OUT_PIN, INPUT);
          pinMode(GPS_RX_DIGITAL_OUT_PIN, INPUT);
       
          startMillis = millis();
          Serial.println("GPS is starting");
          lcd.setCursor(0,1);  
          lcd.print("GPS IS STARTING");
          lcd.setCursor(0,0); 
          lcd.print("SOCOMOT BOOT");
          lcd.setCursor(0,2); 
          lcd.print("GSM IS STARTING");
          delay(2000); 
          startupGSM900();
          if(started)
          {
            //delete all sms message
            Serial.println("Deleting SMS");
            lcd.setCursor(0,0);
            lcd.print("Deleting SMS");
            delay(2000); 
            char error = DeleteAllSMS();
            if (error==1)
            {
                 Serial.println("All SMS deleted");
                lcd.setCursor(0,0);
                lcd.print("All SMS deleted");
                delay(2000); 
            }      
                else
            {
                 Serial.println("SMS not deleted");
                 lcd.setCursor(0,0);
                 lcd.print("SMS not deleted");
                 delay(2000); 
            } 
            startupGPRS();
          }
          else
          {
                Serial.println("SIM900 NOT EXISTED"); 
                lcd.clear();
                lcd.setCursor(0,0); //Start at character 4 on line 0
                lcd.print("SIM900 NOT EXISTED"); 
                delay(2000);
          }
                    
          Serial.print("Initializing SD card...");
          lcd.clear();
          lcd.setCursor(0,0); //Start at character 4 on line 0
          lcd.print("Initializing SD card...");
          delay(2000);
          
          if (!SD.begin(chipSelect)) 
          { 
              Serial.println("initialization failed!");
              lcd.setCursor(0,1); //Start at character 4 on line 0
              lcd.print("initialization failed!");
              delay(2000);
              return;
          }
          Serial.println("Done.");
          lcd.setCursor(0,1); //Start at character 4 on line 0
          lcd.print("Done.");
          Serial.println(' ');
         
          starttime = millis();// start timer, millis() means current time
}


void loop()
{
    //check sdoil has method giving results every 5 second, use start
       if ( start < 5)
        { 
          digitalWrite(PowerModeSwitch, HIGH);
          int count = 0;
          while (count<10){
            Check_Soil();
            count++;
            }
                 {
              readLocation();
              Check_SMS();
              recordAllData();
              thingspeakPost();      
              start++;
                 }
         
         }
      else{
          digitalWrite(PowerModeSwitch, LOW);
          Serial.println("POWER SAVE MODE ACTIVATED FOR 15 MINUTES");
          delay(15*60000);
         //delay(60*100);
          digitalWrite(PowerModeSwitch, HIGH);
          Serial.println("STARTING GPS AND GPRS");
          startupGSM900();
          startupGPRS();
          start = 0;
        }
}


//************************************************************************************************
void startupGSM900()
{
  if (gsm.begin(9600)) 
  {
      Serial.println("\nGSM status = READY");
      lcd.clear();
      lcd.setCursor(0,0); 
      lcd.print("GSM status = READY");
      delay(2000);
      gsm.forceON();        //To ensure that SIM908 is not only in charge mode
      started=true;
  } 
  else 
  {
      Serial.println("\nGSM status = IDLE");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("GSM status = IDLE");
      delay(2000);
  }
}

//************************************************************************************************
void startupGPRS()
{
      char APN[]="internet";
      char USERNAME[]="";
      char PASSWORD[]="";
      gsm.SimpleWrite("AT");
      delay(1000);
      gsm.WhileSimpleRead();
      //GPRS attach, put in order APN, username and password.
      //If no needed auth let them blank.
    
      //if (inet.attachGPRS("internet", "", ""))
      if (inet.attachGPRS(APN, USERNAME, PASSWORD))
      {
          Serial.println("status=ATTACHED");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("status=ATTACHED");
          lcd.setCursor(0,1);
          lcd.print("APN: "&& APN);
          lcd.setCursor(0,2);
          lcd.print("USERNAME: "&&USERNAME);
          lcd.setCursor(0,2);
          lcd.print("PASSWORD: "&&PASSWORD);
          delay(2000);
      }
      else
      {
          Serial.println("status=ERROR");
          lcd.clear();
          lcd.setCursor(0,0); //Start at character 4 on line 0
          lcd.print("status=ERROR");
          delay(1000);
      }
      gsm.SimpleWriteln("AT+CIPSTATUS");
      //gsm.SimpleWriteln("AT+CIPMUX=0");
      //Read IP address.
      gsm.SimpleWriteln("AT+CIFSR");
      delay(5000);
      //Read until serial buffer is empty.
      gsm.WhileSimpleRead();
}

//************************************************************************************************

int counter = 0;
//void Check_Soil() // Check the current air quality
//{ 
//
//readVH400_wStats();
//     // Setup the output struct
//              analogValue = DN_mean;
//              analogValue_sd = DN_stDev;
//              voltage = volts_mean;
//              voltage_sd = volts_stDev;
//              VWC = VWC_mean;
//              VWC_sd = VWC_stDev;
      //while(counter < 60)
      //{
                        //          readVH400_wStats();
                        //          
                        //          value1 = analogRead(MoistureSensor);
                        //          float   sensorVoltage = ((((value1)/102.30)*50)/100);
                        //          float   sensorVoltage2 = sensorVoltage * 0.62;
                        //          float VWC;
                        //          
                        //          Serial.print("sensor readings; ");
                        //          Serial.println(value1);
                        //         // percentage = ((analogRead(A0) - 5)/(70.5))*10;
                        //          Serial.println(percentage);
                        //          percentage = ((((((value1)/102.30)*50)/100) - 0.15)/2.98)*100;
                        //          if(percentage >= 99.00)
                        //          {
                        //            percentage =100.00;
                        //          }
                        //          if(percentage < 1 )
                        //          {
                        //            percentage =0.00;
                        //          }
                        //         //percentage = ((((((analogRead(A0))/102.30)*50)/100) - 0.10)/1.84)*100*0.64;
                        //          //Serial.println(percentage);
                        //          Serial.println(VH400Calibration(sensorVoltage -0.16));              
                        //          lcd.clear();
                        //          lcd.setCursor(0,0); //Start at character 4 on line 0
                        //          lcd.print("     SOCOMOT");
                        //          lcd.setCursor(0,1); //Start at character 4 on line 0
                        //          lcd.print("SV: ");
                        //          lcd.setCursor(4,1);
                        //          lcd.print(value1);
                        //          lcd.setCursor(8,1);
                        //          lcd.print("WATER%:");
                        //          lcd.setCursor(15,1);       
                        //          lcd.print(VH400Calibration(sensorVoltage -0.16));
                        //
                        //          lcd.setCursor(0,2);
                        //          lcd.print("VWC:");
                        //          lcd.setCursor(4,2);   
                        //       
                        //                    if(sensorVoltage2 <= 1.1)
                        //                    {
                        //                       VWC = 10*sensorVoltage2-1;
                        //                    } 
                        //                    else if(sensorVoltage2 > 1.1 && sensorVoltage2 <= 1.3) 
                        //                    {
                        //                        VWC = 25*sensorVoltage2-17.5;
                        //                    } 
                        //                    else if(sensorVoltage2 > 1.3 && sensorVoltage2 <= 1.82) 
                        //                    {
                        //                        VWC = 48.08*sensorVoltage2-47.5;
                        //                    } 
                        //                    else if(sensorVoltage2 > 1.82) 
                        //                    {
                        //                        VWC = 26.32*sensorVoltage2-7.89;
                        //                    }                 
                        //          lcd.print(VWC);
                        //
                        ////              analogValue = DN_mean;
                        ////              analogValue_sd = DN_stDev;
                        ////              voltage = volts_mean;
                        ////              voltage_sd = volts_stDev;
                        ////              VWC = VWC_mean;
                        ////              VWC_sd = VWC_stDev;
                        //     
                        //          lcd.setCursor(10,2);
                        //          lcd.print("VR:");  
                        //
                        //          lcd.setCursor(13,2);
                        //          lcd.print(sensorVoltage);
                        //          
                        ////          lcd.setCursor(0,3);  
                        ////          lcd.print("VWC_SD: ");
                        ////          lcd.setCursor(7,3); 
                        ////          lcd.print(VWC_sd);
                        //
                        //          lcd.setCursor(0,3);  
                        //          lcd.print("VR2: ");
                        //          lcd.setCursor(5,3); 
                        //          lcd.print(sensorVoltage2);
                        //
                        //          //lcd.setCursor(0,3);  
                        //          //lcd.print("V_SD: ");
                        //          lcd.setCursor(13,3); 
                        //          lcd.print((((value1)/10.23)*5));
                        //         // joel
                        //          delay(1000);
                        //          counter++;
      //}
      //      counter =0;
      //      readLocation();
      //      Check_SMS();
      //      recordAllData();
      //      thingspeakPost();
      //       
//}

void Check_Soil(){ 

              readVH400_wStats();
               Serial.println(" ");
              Serial.print("analogValue: ");Serial.println(analogValue);
              Serial.print("analogValue_sd: ");Serial.println(analogValue_sd);
              Serial.print("voltage: ");Serial.println(voltage);
              Serial.print("voltage_sd: ");Serial.println(voltage_sd);
              Serial.print("VWC: ");Serial.println(VWC); 
              Serial.print("VWC_sd: ");Serial.println(VWC_sd); 
              Serial.print("H2O %: ");Serial.println(VH400Calibration( voltage)); 
              Serial.println(" ");
              recordAllData();
              }

//***************************************************************************************************//
char DeleteAllSMS()
{
         char ret_val = -1;
         
         if (CLS_FREE != gsm.GetCommLineStatus()) return (ret_val);
         gsm.SetCommLineStatus(CLS_ATCMD);
         ret_val = 0; // still not present
              
         gsm.SimpleWriteln(F("AT+CMGDA=\"DEL ALL\""));
              
         switch (gsm.WaitResp(8000, 50, "OK")) 
         {
               case RX_TMOUT_ERR:
                    // response was not received in specific time
                    ret_val = -2;
                    break;
           
               case RX_FINISHED_STR_RECV:
                    // OK was received => SMS deleted
                    ret_val = 1;
                    break;
           
               case RX_FINISHED_STR_NOT_RECV:
                    // other response: e.g. ERROR => SMS was not deleted
                    ret_val = 0;
                    break;
         }
         
         gsm.SetCommLineStatus(CLS_FREE);
         return (ret_val);         
}


//***************************************************************************************************//
void readLocation(){
  
  unsigned long chars = 0;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (Serial2.available())
    {
      int c = Serial2.read();
      //Serial.print((char)c); // if you uncomment this you will see the raw data from the GPS
      ++chars;
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }
  
  if (newData)
  {
    // we have a location fix so output the lat / long and time to acquire
    if(secondsToFirstLocation == 0){
      secondsToFirstLocation = (millis() - startMillis) / 1000;
      Serial.print("Acquired in:");
        lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       //lcd.print("Acquired in :"word(secondsToFirstLocation)"S");  
       delay(2000);
      Serial.print(secondsToFirstLocation);
      Serial.println("s");
    }
    
    unsigned long age;
    // extract related data from GPS signal
    gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths, &age);
    
    gps.f_get_position(&latitude, &longitude, &age);    
    latitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : latitude;
    longitude == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : longitude;
    
    altitude = gps.f_altitude();
    altitude == TinyGPS::GPS_INVALID_F_ALTITUDE ? 0.0 : altitude;
    
    speed_mps = gps.f_speed_mps();
    speed_mps == TinyGPS::GPS_INVALID_F_SPEED ? 0.0 : speed_mps;

    satellites = gps.satellites();
    satellites == TinyGPS::GPS_INVALID_SATELLITES ? 0 : satellites;

    hdop = gps.hdop();
    hdop == TinyGPS::GPS_INVALID_HDOP ? 0.0 : hdop;

    Serial.println("GPS is working!");
     lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       lcd.print("GPS is working!");
       delay(2000);
    GPSworks = true;

  }

  // print out the message in I/O port for checking
  Serial.println(data_sms);
   lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       lcd.print(data_sms);
       delay(2000);
  if (chars == 0){
    // if you haven't got any chars then likely a wiring issue
    Serial.println("Check wiring");
     lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       lcd.print("Check wiring");
       delay(2000);
  }
  else if(secondsToFirstLocation == 0){
    // still working
  }
}


//*******************************************************************************88//
void recordAllData()
{
  myFile = SD.open("socomot.txt", FILE_WRITE);
  if (myFile) {
    Serial.print("Writing to socomot.txt...");
     lcd.clear();
     lcd.setCursor(0,0); //Start at character 4 on line 0
     lcd.print("Writing to socomot.txt...");
     delay(2000);
              myFile.println(' ');    
              //myFile.print("analogValue: ");
              myFile.print(analogValue);myFile.print("\t"); 
              //myFile.print("analogValue_sd: ");
              myFile.print(analogValue_sd);myFile.print("\t"); 
              //myFile.print("voltage: ");
              myFile.print(voltage);myFile.print("\t"); 
              //myFile.print("voltage_sd: ");
              myFile.print(voltage_sd);myFile.print("\t"); 
              //myFile.print("VWC: ");
              myFile.print(VWC); myFile.print("\t"); 
              //myFile.print("VWC_sd: ");
              myFile.print(VWC_sd); myFile.print("\t"); 
              //myFile.print("H2O %: ");
              myFile.print(VH400Calibration( voltage)); 
              myFile.print("\t"); 
      
     myFile.print("\t");
    myFile.print(year);
    myFile.print(",");
    myFile.print(month);
    myFile.print(",");
    myFile.print(day);
    myFile.print("  ");
    myFile.print(hour);
    myFile.print(":");
    myFile.print(minute);
    myFile.print(":");
    myFile.print(second);
    myFile.print(":");
    myFile.print(hundredths);
    myFile.print("\t: ");
    myFile.print(latitude, 6);
    myFile.print(" , ");
    myFile.print(longitude, 6);
    myFile.print(" , ");
    myFile.print(altitude, 6);
    myFile.print("SAT=");
    myFile.print(satellites, 0);
    myFile.print(" PREC=");
    myFile.print(hdop/100);
    myFile.print(" Speed= ");
    myFile.print(speed_mps);
    myFile.print("m/s");
    //myFile.println(' '); 
    myFile.close();   
    Serial.println("OK.");
    datalog = 1;
  } else {
    // if the file didn't open, print an error:
    Serial.println("error in opening socomot.txt");  
    datalog = 0;
    lcd.clear();
    lcd.setCursor(0,0); //Start at character 4 on line 0
    lcd.print("error in opening socomot.txt");  
    delay(2000);

  }
}

void recordUserData()
{
    userFile = SD.open("users.txt", FILE_WRITE);
    if (userFile) {
        Serial.println("Recording the user number...");
        lcd.clear();
        lcd.setCursor(0,0); //Start at character 4 on line 0
        lcd.print("Recording the user number...");
        delay(2000);
        userFile.print("Received SMS from ");
        userFile.print(number_incoming);
        userFile.print(" at ");
        userFile.print(year); 
        userFile.print(",");
        userFile.print(month);
        userFile.print(",");
        userFile.print(day);
        userFile.print("  ");
        userFile.print(hour);
        userFile.print(":");
        userFile.print(minute);
        userFile.print(":");
        userFile.println(second);
        Serial.println("OK.");
    }
    userFile.close();
}

void thingspeakPost() {
  char itoaBuffer[8];

  char end_c[2];
  end_c[0] = 0x1A;
  //end_c[1] = '\0';


char toPostAnalogValue[20];
char toPostAnalogValue_SD[20];
char toPostVoltage[20];
char toPostVoltage_SD[20];
char toPostVWC[20];
char toPostVWC_SD[20];
char toPostPercentage[20];

dtostrf(analogValue, 8, 2, toPostAnalogValue);
dtostrf(analogValue_sd, 8, 2, toPostAnalogValue_SD);
dtostrf(voltage, 8, 2, toPostVoltage);
dtostrf(voltage_sd, 8, 2, toPostVoltage_SD);
dtostrf(VWC, 8, 2, toPostVWC);
dtostrf(VWC_sd, 8, 2, toPostVWC_SD);
dtostrf(VH400Calibration( voltage), 8, 2, toPostPercentage);

if (inet.connectTCP(thingSpeakAddress, 80)) {

   Serial.println("connected to thingspeak");
   lcd.clear();
   lcd.setCursor(0,0); //Start at character 4 on line 0
   lcd.print("connected to thingspeak");
    delay(2000);
    gsm.SimpleWrite("POST /update HTTP/1.1\r\n");
    gsm.SimpleWrite("Host: api.thingspeak.com\r\n");
    gsm.SimpleWrite("Connection: close\r\n");
    gsm.SimpleWrite("X-THINGSPEAKAPIKEY: ");
    gsm.SimpleWrite(writeAPIKey);
    gsm.SimpleWrite("\r\n");
    gsm.SimpleWrite("Content-Type: application/x-www-form-urlencoded\r\n");
    gsm.SimpleWrite("Content-Length: ");
    //sprintf(sentMsg, "field1=%s&field2=%s&field3=%s",Sensor1,percentage,datalog);
    //itoa(strlen(sentMsg), itoaBuffer, 10);

   
    sprintf(sentMsg, "field1=%s&field2=%s&field3=%s&field4=%s&field5=%s&field6=%s&field7=%s&field8=%d",toPostAnalogValue,toPostAnalogValue_SD,toPostVoltage,toPostVoltage_SD,toPostVWC,toPostVWC_SD,toPostPercentage,datalog);
    itoa(strlen(sentMsg), itoaBuffer, 10);

    gsm.SimpleWrite(itoaBuffer);

    gsm.SimpleWrite("\r\n\r\n");

    gsm.SimpleWrite(sentMsg);

    gsm.SimpleWrite("\r\n\r\n");

    delay(200);
    
    gsm.SimpleWrite(end_c);
    
    
    //gsm.SimpleWrite("AT+CIPCLOSE");

    Serial.println(sentMsg);
    lcd.clear();
    lcd.setCursor(0,0); //Start at character 4 on line 0
    lcd.print(sentMsg);
    delay(2000);

    //************************
    failtimes = 0;
  }
  else
  {
    Serial.println("Uploading failed");
       lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       lcd.print("Uploading failed");
       delay(2000);
    failtimes = failtimes + 1 ;
    Serial.print("Fail times now is; ");
    lcd.clear();
    lcd.setCursor(0,0); //Start at character 4 on line 0
       //lcd.print("Fail times now is; "word(failtimes));
    delay(2000);
    Serial.println(failtimes);
    if (failtimes == 2)
    {
      Serial.println("System will restart the GSM and GPRS");
       lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       lcd.print("System will restart the GSM and GPRS");
       delay(2000);
//    delay(30000);
      startupGSM900();
      startupGPRS();
      failtimes = 0;
      
    }
  }
}



//************************************************************************************************
void Check_SMS()  //Check if there is an sms 'type_sms'
 {
     char pos_sms_rx;  //Received SMS position
     pos_sms_rx=sms.IsSMSPresent(type_sms);
     if (pos_sms_rx!=0)
     {
       //Read text/number/position of sms
       sms.GetSMS(pos_sms_rx,number_incoming,15,sms_rx,120);
       Serial.print("Received SMS from ");
       Serial.print(number_incoming);
       Serial.print("(sim position: ");
       //Serial.print(word(pos_sms_rx));
       Serial.println(")");
       Serial.println(sms_rx);
       lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       //lcd.print("Received SMS from "word(number_incoming));

       lcd.setCursor(0,1); //Start at character 4 on line 0
       //lcd.print("(sim position: "word(pos_sms_rx)")");

       delay(2000);
       if (del_sms==1)  //If 'del_sms' is 1, i delete sms
       {
         error=sms.DeleteSMS(pos_sms_rx);
         if (error==1)Serial.println("SMS deleted");
          lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       //lcd.print("Received SMS from "word(number_incoming));
       delay(2000);
       }
         else
       {
       Serial.println("SMS not deleted");
       lcd.clear();
       lcd.setCursor(0,0); //Start at character 4 on line 0
       lcd.print("SMS not deleted");
       delay(2000);
       }

       // Check the receiving message is "air"
       if((strstr(sms_rx,"air")!=0)&&(strlen(sms_rx)==3))
       {
          // store the coming number in SD card
          recordUserData();

          // send the message
          Serial.println("\nsending SMS");
          lcd.clear();
          lcd.setCursor(0,0); //Start at character 4 on line 0
          lcd.print("\nsending SMS");
          delay(2000);
          // if newData is true, that means there is new data received from GPS.
          if(newData)
          {
            if (sms.SendSMS(number_incoming, data_sms))
            {
              Serial.println("\nSMS sent OK");
              lcd.clear();
              lcd.setCursor(0,0); //Start at character 4 on line 0
              lcd.print("\nSMS sent OK");
              delay(2000);
            }
            else
            {
              Serial.println("\nSMS sent error");
              lcd.clear();
              lcd.setCursor(0,0); //Start at character 4 on line 0
              lcd.print("\nSMS sent error");
              delay(2000);
             }
          }
          else
          {
              if (sms.SendSMS(number_incoming, "GPS is not ready."))
              {
              Serial.println("\nSMS sent OK");
              lcd.clear();
              lcd.setCursor(0,0); //Start at character 4 on line 0
              lcd.print("\nSMS sent OK");
              delay(2000);
              }
              else
              {
              Serial.println("\nSMS sent error");
              lcd.clear();
              lcd.setCursor(0,0); //Start at character 4 on line 0
              lcd.print("\nSMS sent error");
              delay(2000);
              }
          }
     Serial2.flush();
     }
     newData = false;
     return;
 }
 }







//*********************************************************************************************************************************************************************
float readVH400()
{
  // This function returns Volumetric Water Content by converting the analogPin value to voltage
  // and then converting voltage to VWC using the piecewise regressions provided by the manufacturer
  // at http://www.vegetronix.com/Products/VH400/VH400-Piecewise-Curve.phtml
  
  // Read value and convert to voltage  
  
          int sensor1DN = analogRead(value1);
          float sensorVoltage = sensor1DN*(5.0 / 1023.0);
         // float VWC;
          
          // Calculate VWC
          if(sensorVoltage <= 1.1) {
            VWC = 10*sensorVoltage-1;
          } else if(sensorVoltage > 1.1 && sensorVoltage <= 1.3) {
            VWC = 25*sensorVoltage-17.5;
          } else if(sensorVoltage > 1.3 && sensorVoltage <= 1.82) {
            VWC = 48.08*sensorVoltage-47.5;
          } else if(sensorVoltage > 1.82) {
            VWC = 26.32*sensorVoltage-7.89;
          }
          return(VWC);
}
//*********************************************************************************************************************************************************************

//*********************************************************************************************************************************************************************
struct VH400 
{
        double analogValue;
        double analogValue_sd;
        double voltage;
        double voltage_sd;
        double VWC;
        double VWC_sd;
};
//*********************************************************************************************************************************************************************

void readVH400_wStats() 
{
      // This variant calculates the mean and standard deviation of 1000 measurements over 60 seconds.
      // It reports mean and standard deviation for the analog value, voltage, and WVC.
          
      // This function returns Volumetric Water Content by converting the analogPin value to voltage
      // and then converting voltage to VWC using the piecewise regressions provided by the manufacturer
               
              int nMeasurements = 100; 
              int delayBetweenMeasurements = 50;
              
              //struct VH400 result;
              
              // Sums for calculating statistics
              int sensorDNsum = 0;
              double sensorVoltageSum = 0.0;
              double sensorVWCSum = 0.0;
              double sqDevSum_DN = 0.0;
              double sqDevSum_volts = 0.0;
              double sqDevSum_VWC = 0.0;
            
              // Arrays to hold multiple measurements
              int sensorDNs[nMeasurements];
              double sensorVoltages[nMeasurements];
              double sensorVWCs[nMeasurements];
double DN_mean2 =0.0;
            
              // Make measurements and add to arrays
              for (int i = 0; i < nMeasurements; i++) 
              { 
                    // Read value and convert to voltage 
                 
                        
                    // Calculate VWC
                      // Read value and convert to voltage 
                    int sensorDN = analogRead(A1);
                    double sensorVoltage = sensorDN*(5.0 / 1023.0);
                    //Serial.print(sensorDN); Serial.print(" "); Serial.println(sensorVoltage);
                        
                    // Calculate VWC
                    float VWC;
                    if(sensorVoltage <= 1.1)
                    {
                       VWC = 10*sensorVoltage-1;
                    } 
                    else if(sensorVoltage > 1.1 && sensorVoltage <= 1.3) 
                    {
                        VWC = 25*sensorVoltage-17.5;
                    } 
                    else if(sensorVoltage > 1.3 && sensorVoltage <= 1.82) 
                    {
                        VWC = 48.08*sensorVoltage-47.5;
                    } 
                    else if(sensorVoltage > 1.82) 
                    {
                        VWC = 26.32*sensorVoltage-7.89;
                    }               
                
                    // Add to statistics sums
                    sensorDNsum += sensorDN;
                    sensorVoltageSum += sensorVoltage;
                    sensorVWCSum += VWC;
                
                    // Add to arrays
                    sensorDNs[i] = sensorDN;
                    sensorVoltages[i] = sensorVoltage;
                    sensorVWCs[i] = VWC;
                
                    // Wait for next measurement
                    delay(delayBetweenMeasurements);
              }
            
              // Calculate means
              double DN_mean = double(sensorDNsum)/double(nMeasurements);
              double volts_mean = sensorVoltageSum/double(nMeasurements);
              double VWC_mean = sensorVWCSum/double(nMeasurements);

            
            
              // Loop back through to calculate SD
              for (int i = 0; i < nMeasurements; i++) 
              { 
                    sqDevSum_DN += pow((DN_mean - double(sensorDNs[i])), 2);
                    sqDevSum_volts += pow((volts_mean - double(sensorVoltages[i])), 2);
                    sqDevSum_VWC += pow((VWC_mean - double(sensorVWCs[i])), 2);

                    DN_mean2 += sensorDNs[i];
              }
              double DN_stDev = sqrt(sqDevSum_DN/double(nMeasurements));
              double volts_stDev = sqrt(sqDevSum_volts/double(nMeasurements));
              double VWC_stDev = sqrt(sqDevSum_VWC/double(nMeasurements));

                        
              // Setup the output struct
              //analogValue = DN_mean;
              analogValue = DN_mean2/nMeasurements;
              analogValue_sd = DN_stDev;
              voltage = volts_mean;
              voltage_sd = volts_stDev;
              VWC = VWC_mean;
              VWC_sd = VWC_stDev;
            
              // Return the result
              //return(result);
              
              
 }
//*********************************************************************************************************************************************************************



//*********************************************************************************************************************************************************************
int VH400Calibration( float sensorVoltage)
{
   
//          int sensor1DN = analogRead(value1);
//          float sensorVoltage = sensor1DN*(5.0 / 1023.0);
          int result;

          if(sensorVoltage <= 0.60) {
            result = 0;
          } else if(sensorVoltage > 0.60 && sensorVoltage<= 0.68) {
            result = 5;
          } else if(sensorVoltage > 0.68 && sensorVoltage <= 1.10) {
            result = 10;
          } else if(sensorVoltage > 1.10 && sensorVoltage <= 1.30) {
            result = 15;
          } else if(sensorVoltage > 1.30 && sensorVoltage <= 1.40) {
            result = 20;
          } else if(sensorVoltage > 1.40 && sensorVoltage <= 1.50) {
            result = 25;
          } else if(sensorVoltage > 1.50 && sensorVoltage <= 1.60) {
            result = 30;
          } else if(sensorVoltage > 1.60 && sensorVoltage <= 1.70) {
            result = 35;
          } else if(sensorVoltage > 1.70 && sensorVoltage <= 1.80) {
            result = 40;
          } else if(sensorVoltage > 1.80 && sensorVoltage <= 2.00) {
            result = 45;
          } else if(sensorVoltage > 2.00 && sensorVoltage <= 2.20) {
            result = 50;
          } else if(sensorVoltage > 2.20 && sensorVoltage <= 2.28) {
            result = 55;  
          } else if(sensorVoltage > 2.28 && sensorVoltage <= 2.35) {
            result = 60;  
          } else if(sensorVoltage > 2.35 && sensorVoltage <= 2.45) {
            result = 65;  
          } else if(sensorVoltage > 2.45 && sensorVoltage <= 2.52) {
            result = 70;  
          } else if(sensorVoltage > 2.52 && sensorVoltage <= 2.60) {
            result = 75;
          } else if(sensorVoltage > 2.60 && sensorVoltage <= 2.68) {
            result = 80;    
          } else if(sensorVoltage > 2.68 && sensorVoltage <= 2.75) {
            result = 85;    
          } else if(sensorVoltage > 2.75 && sensorVoltage <= 2.85) {
            result = 90; 
          } else if(sensorVoltage > 2.85 && sensorVoltage <= 2.91) {
            result = 95;       
          } else if(sensorVoltage > 2.91) {
            result = 100;    
          }
          return(result);
}
//*********************************************************************************************************************************************************************
