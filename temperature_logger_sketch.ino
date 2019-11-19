#include <SoftwareSerial.h> //Library to establish connection with ESP8266 via digital pins
#include "DHT.h" //Library to establish a communication with DHT22 via digital pin

#define RX 10 //Pin to connect TX ESP8266 pin
#define TX 11 //Pin to connect RX ESP8266 pin (it is normal that they are reversed)
#define sensPort 2 //Pin to connect DHT22 data pin

String ssid     = ""; //Wifi SSID 
String password = ""; //Wifi Password 
String apiKeyIn = ""; //API Key thingspeak

SoftwareSerial AT(RX,TX); //Initialization of serial connection for ESP8266
DHT dht(sensPort,DHT22); //Initialization of library for connection to DHT22

const unsigned int writeInterval = 25000; // write interval

String host = "api.thingspeak.com";  // API host name
String port = "80";      // Port

int AT_cmd_time = 0; //This variable is used to count how many times an AT command will be repeated (in function sendATcmd)
boolean AT_cmd_result = false; //Variable that tracks the success or failure of AT commands

void setup() {
  Serial.begin(9600); //Beginning of the 9600 baud Arduino serial connection
  AT.begin(9600); //Beginning of the 9600 baud ESP8266 serial connection
  dht.begin(); //Beginning of DHT22 connection
  sendATcmd("AT",5,"OK"); //Command to check if ESP8266 is ready
  sendATcmd("AT+CWMODE=1",5,"OK"); //Set Wi-Fi mode on for ESP8266
  Serial.print("Connecting to WiFi:");
  Serial.println(ssid); //Print ssid on serial monitor
  sendATcmd("AT+CWJAP=\""+ ssid +"\",\""+ password +"\"",20,"OK"); //Command to connect ESP8266 to Wi-Fi
}

void loop() {
  float t = dht.readTemperature(); //Get temperature from DHT22
  float h = dht.readHumidity(); //Get humidity from DHT22
  if (isnan(t) || isnan(h)) { //Checking if value retrieved from t and h are numbers or not
    Serial.println("The sensor cannot be read!");
  } else {
    String url = "GET /update?api_key="+ apiKeyIn +"&field1="+String(t)+"&field2="+String(h); //Building URL to send data on Thingspeak
    
    Serial.print("requesting URL: ");
    Serial.println(url); //Print url on serial monitor
    
    String requestLen = String(url.length() + 4); //Get request length
    Serial.println("Open TCP connection");
    sendATcmd("AT+CIPMUX=1", 10, "OK"); //This command configures ESP8266 for a multi IP connection
    sendATcmd("AT+CIPSTART=0,\"TCP\",\"" + host +"\"," + port, 20, "OK"); //Starting TCP connection with the host
    sendATcmd("AT+CIPSEND=0," + requestLen, 10, ">"); //Preparing request sending the lenght of url
    AT.println(url); //Prints the url on ESP8266 terminal
    sendATcmd("AT+CIPCLOSE=0", 5, "OK"); //Close the TCP connection
    
    Serial.println("Close TCP Connection");
    
    delay(writeInterval); // delay
  }

}

//function to send AT command via ESP8266
void sendATcmd(String AT_cmd, int AT_cmd_maxTime, char readReplay[]) { //first parameter is for command, second parameter is for maximum number of times the command will be retryed, third parameter is for output expected
  Serial.print("AT command:");
  Serial.println(AT_cmd); //Prints the command that is to be executed

  while(AT_cmd_time < (AT_cmd_maxTime)) { //Repeats the command a number of times until it is accepted
    AT.println(AT_cmd); //The command is executed is ESP8266 terminal
    if(AT.find(readReplay)) { //If the ESP8266 output is equal to parameter "readReplay", the command is successful and while cycle is termined
      AT_cmd_result = true; //This flag is setted true because the command has been successfully executed
      break;
    }
  
    AT_cmd_time++; //Increments this variable to count the number of times the command is executed
  }
  Serial.print("...Result:");
  if(AT_cmd_result == true) { //If the command execution was successful, the word "DONE" is printed on serial monitor
    Serial.println("DONE");
    AT_cmd_time = 0;
  }
  
  if(AT_cmd_result == false) { //If the command execution was unsuccessful, the word "FAILED" is printed on serial monitor
    Serial.println("FAILED");
    AT_cmd_time = 0;
  }
  
  AT_cmd_result = false;
}
