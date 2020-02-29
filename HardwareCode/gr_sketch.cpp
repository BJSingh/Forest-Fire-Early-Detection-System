#include <Arduino.h>
#include <stdlib.h>
#include <String.h>
#include "DHT.h"

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE);

//Ubidots account data
String token = "NzfYyssVvBNoaheLO6kTyLUw7imvXAZcCjCAGehx4FdjtV4TMLEGDbgqycRq";      //your token to post value
String temp_id="55e17b7e76254213705d5df1";                                 //ID of your temperature variable
String hum_id="55e20a1e76254213b3940005";                                 //ID of your humidity variable
String co_id="564ca4607625423cf0bdf116";                                 //ID of your carbon monoxide variable
String smoke_id="564ca4c17625424007e02dbc";                             //ID of your smoke variable
String alert_id="5650b0a27625421aeb72bb7e";					           //ID of alert variable to indicate unauthorised access to system
String bat_id = "566a9702762542379289cbb0";                                     //ID of your variable

//sensor output connections
const int smoke_out = A0;
const int co_out = A1;

//battery level reading pin
const int bat_out = A2;

//variables to store sensor data
float temp=0,hum=0;
int bat,co=0,smoke=0;

//accelerometer variables x, y and z axis
const int xout = A3;
const int yout = A4;
const int zout = A5;
int out1,out2,out3;
int initial_out1, initial_out2, initial_out3;

int unauthorised_access(){
//reading x, y and z parameter for corresponding change if any movements happens
  int Xchange=0,Ychange=0,Zchange=0;
  out1 = analogRead(xout);           
  delay(2);                    

  out2 = analogRead(yout);           
  delay(2);                
  
  out3 = analogRead(zout);           
  delay(2);                
    
  if(out1<initial_out1){
  	if(out1<(initial_out1-10)){
  		Xchange=1;
  	}
  	
  } 
  else if(out1>initial_out1){
  		if(out1>(initial_out1+10)){
  			Xchange=1;
  		}	
  }
  else {
  	Xchange=0;
  }
  
  if(out2<initial_out2){
  	if(out2<(initial_out2-10)){
  		Ychange=1;
  	}
  	
  } 
  else if(out2>initial_out2){
  		if(out2>(initial_out2+10)){
  			Ychange=1;
  		}	
  }
  else {
  	Ychange=0;
  }
  
  if(out3<initial_out3){
  	if(out3<(initial_out3-10)){
  		Zchange=1;
  	}
  	
  } 
  else if(out3>initial_out3){
  		if(out3>(initial_out3+10)){
  			Zchange=1;
  		}	
  }
  else {
  	Zchange=0;
  }
  	
  if((Xchange==1)||(Ychange==1)||(Zchange==1)){
  	return 1;
  }	
  else return 0;
}  


void ShowSerialData()
{
  while(Serial2.available()!=0)
  Serial.write(Serial2.read());
}

//this function is to send the sensor data to Ubidots, you should see the new value in Ubidots after executing this function
void save_value(String payload)
{
   String le;                                               // length of the payload in characters
   le = String(payload.length());      //this is to calcule the length of payload 
    
  for(int i = 0;i<7;i++)
  {
    Serial2.println("AT+CGATT?");                                                   //this is made repeatedly because it is unstable
    delay(2000);
    ShowSerialData();
  }
  
  Serial2.println("AT+CSTT=\"aircelgprs.pr\"");                                    //replace with your providers' APN
  delay(1000);
  ShowSerialData();
  Serial2.println("AT+CIICR");                                                      //bring up wireless connection
  delay(3000);
  ShowSerialData();
  Serial2.println("AT+CIFSR");                                                      //get local IP adress
  delay(2000);
  ShowSerialData();
  Serial2.println("AT+CIPSPRT=0");
  delay(3000);
  ShowSerialData();
  Serial2.println("AT+CIPSTART=\"tcp\",\"things.ubidots.com\",\"80\"");             //start up the connection
  delay(3000);
  ShowSerialData();
  Serial2.println("AT+CIPSEND");                                                    //begin send data to remote server
  delay(3000);
  ShowSerialData();
  Serial2.print(F("POST /api/v1.6/collections/values/?token="));
  delay(100);
  ShowSerialData();
  Serial2.print(token);
  delay(100);
  ShowSerialData();
  Serial2.println(F(" HTTP/1.1"));
  delay(100);
  ShowSerialData();
  Serial2.println(F("Content-Type: application/json"));
  delay(100);
  ShowSerialData();
  Serial2.print(F("Content-Length: "));
  Serial2.println(le);
  delay(100);
  ShowSerialData();
  Serial2.print(F("Host: "));
  Serial2.println(F("things.ubidots.com"));
  Serial2.println(); 
  delay(100);
  ShowSerialData();
  Serial2.println(payload); 
  Serial2.println();
  delay(100);
  ShowSerialData();
  Serial2.println((char)26);
  delay(7000);
  Serial2.println();
  ShowSerialData();
  Serial2.println("AT+CIPCLOSE");                                                //close the communication
  delay(1000);
  ShowSerialData();
}

void setup()
{
  Serial2.begin(9600);                                                             //sim 900 coonected to serial2 of GR-Kaede at 9600 baud rate
  Serial.begin(9600);                                                              //serial communication baud rate
  dht.begin();
  
  delay(3000);
  //initial values which will be compared during any unauthorised movements of installed device
  out1 = analogRead(xout);           
  initial_out1=out1;
  delay(2);                    

  out2 = analogRead(yout);           
  initial_out2=out2;
  delay(2);                
  
  out3 = analogRead(zout);           
  initial_out3=out3;
  
  delay(2000);
}

void loop()
{
  int alert=0,cnt=0;
  String payload;                                           //Variable to collect all sensor data for data upload on Webserver
  
  if (Serial2.available())
  {
  Serial.write(Serial2.read());
  }
  
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)  
    hum = dht.readHumidity(); 	    
    temp = dht.readTemperature();
    
    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(temp) || isnan(hum)) 
    {
      hum = 0;
      temp = 0;
      Serial.println("Failed to read from DHT");
    } 
    else 
    {
      Serial.print("Humidity: "); 
      Serial.print(hum);
      Serial.print(" %\t");
      Serial.print("Temperature: "); 
      Serial.print(temp);
      Serial.println(" *C");
     }
    
    co = analogRead(co_out);                                                     
    co = map(co, 0 , 1023, 0, 100);
    Serial.print("CO = ");
    Serial.println(co);
    
    smoke = analogRead(smoke_out);                                                     
    smoke = map(smoke, 0 , 1023, 0, 100);
    Serial.print("Smoke = ");
    Serial.println(smoke);
    
    alert=unauthorised_access();
    Serial.print("Alert = ");
    Serial.println(alert);
    
    if(alert==1)
    {
    	Serial.println("Unauthorised activity, Sending Alert");
    	delay(2);
    }
    else {
    	Serial.println("Safe");
    	delay(2);
    }
   
   while(cnt<5)
   {
   Serial.println();
   Serial.println("Uploading Sensors Data to Ubidots Cloud Service");
   payload = "[{\"variable\":\"" + temp_id + "\",\"value\":"+ String(temp)+"},{\"variable\":\""+ hum_id+ "\",\"value\":" + String(hum) + "},{\"variable\":\"" +co_id+ "\",\"value\":" + String(co) + "},{\"variable\":\"" + smoke_id + "\",\"value\":" + String(smoke) + "},{\"variable\":\"" + alert_id + "\",\"value\":"+ String(alert) + "}]";
   save_value(payload);                                                      //call the save_value function
   cnt++;
   delay(20000);
    
    hum = dht.readHumidity(); 	    
    temp = dht.readTemperature();
        
    co = analogRead(co_out);                                                     
    co = map(co, 0 , 1023, 0, 100);
    
    smoke = analogRead(smoke_out);                                                     
    smoke = map(smoke, 0 , 1023, 0, 100);

    alert=unauthorised_access();   
   }
   
   delay(20000);   
   cnt=0;
   bat = analogRead(bat_out);                                                     
   bat = map(bat, 0 , 1023, 0, 100);
   Serial.print("Battery Level = ");
   Serial.println(bat);
   
   alert=unauthorised_access();

  while(cnt<2)
  {
   Serial.println("Uploading Battery Level and activity to Ubidots Cloud Service");
   payload = "[{\"variable\":\"" + bat_id + "\",\"value\":"+ String(bat)+"},{\"variable\":\""+ alert_id+ "\",\"value\":" + String(alert) + "}]";
   save_value(payload);                                                      //call the save_value function
   cnt++;
   delay(20000);
  }   
   //Delay for 90 seconds
   delay(45000);
   delay(45000);
}
