/* 
Arduino Uno(Uart) + ESP8266 ESP-01 (1MB Flash)

Author : ChungYi Fu (Taiwan)  2018-04-06 20:00

Update AT Firmware(V2.0_AT_Firmware)
https://www.youtube.com/watch?v=QVhWVu8NnZc
http://www.electrodragon.com/w/File:V2.0_AT_Firmware(ESP).zip
nodemcu-flasher
https://github.com/nodemcu/nodemcu-flasher
(Baudrate:115200, Flash size:1MByte, Flash speed:26.7MHz, SPI Mode:QIO)
*/

String WIFI_SSID = "";   //your network SSID
String WIFI_PWD  = "";    //your network password

#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // Arduino RX:10, TX:11 

String ReceiveData="",STAIP="",STAMAC="";

void setup()
{
  Serial.begin(9600);
  
  //You must change uart baud rate of ESP-01 to 9600.
  mySerial.begin(115200);   //Default uart baud rate
  SendData("AT+UART_CUR=9600,8,1,0,0",2000);   //Change uart baud rate to 9600
  mySerial.begin(9600);  // you will get more stable data.
  mySerial.setTimeout(10);

  SendData("AT+CIPSERVER=0",2000);
  SendData("AT+CWMODE_CUR=1",2000);
  SendData("AT+CIPMUX=0",2000);
  
  if (WIFI_SSID!="") 
    SendData("AT+CWJAP_CUR=\""+WIFI_SSID+"\",\""+WIFI_PWD+"\"",5000);  
  else
    Serial.print("Please check your network SSID and password");  
}

void loop() 
{
  if (STAIP=="")
    getSTAIP();
  else
  {
    int SensorData = rand()%100;      //humidity

    //Send sensor data to database          
    String Domain="192.168.201.10";
    //If request length is too long, it can't work! 
    String request = "GET /humidity="+String(SensorData)+" HTTP/1.1\r\nHost: "+Domain+"\r\n\r\n";
    
    SendData("AT+CIPSTART=\"TCP\",\""+Domain+"\",80", 5000);
    SendData("AT+CIPSEND=" + String(request.length()+2), 2000);
    SendData(request, 2000);
    delay(1);
    SendData("AT+CIPCLOSE",2000);
    
    Serial.println(val);
    delay(5000);  
  }
}

void SendData(String data,int TimeLimit)
{
  mySerial.println(data);
  mySerial.flush();
  delay(10);
  WaitReply(TimeLimit);
}

String WaitReply(long int TimeLimit)
{
  ReceiveData="";
  long int StartTime=millis();
  while( (StartTime+TimeLimit) > millis())
  {
    if (mySerial.available())
    {
      delay(4);
      while(mySerial.available())
      {
        ReceiveData=ReceiveData+String(char(mySerial.read()));
        //ReceiveData=ReceiveData+mySerial.readStringUntil('\r'); 
      }
      //Serial.println(ReceiveData);
      return ReceiveData;
    }
  } 
  return ReceiveData;
}

void getSTAIP()
{
  ReceiveData="";
  
  if (mySerial.available())
  {
    while (mySerial.available())
    {
      char c=mySerial.read();
      ReceiveData=ReceiveData+String(c);
    } 
    
    if (ReceiveData.indexOf("WIFI GOT IP")!=-1)
    { 
      while(!mySerial.find('OK')){} 
      delay(10);

      int staipreadstate=0,stamacreadstate=0,j=0;
      mySerial.println("AT+CIFSR");
      mySerial.flush();
      delay(6);
      
      while(mySerial.available())
      {
        char c=mySerial.read();
        String t=String(c);
        
        if (t=="\"") j++;
        
        if (j==1) 
          staipreadstate=1;
        else if (j==2)
          staipreadstate=0;
        if ((staipreadstate==1)&&(t!="\"")) STAIP=STAIP+t;

        if (j==3) 
          stamacreadstate=1;
        else if (j==4)
          stamacreadstate=0;
        if ((stamacreadstate==1)&&(t!="\"")) STAMAC=STAMAC+t;
        
        delay(1);
      } 
      Serial.println("STAIP: "+STAIP+"\nSTAMAC: "+STAMAC+"\n");
    }
  }
}
