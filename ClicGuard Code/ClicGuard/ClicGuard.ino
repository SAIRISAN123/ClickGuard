#include <LiquidCrystal_I2C.h>
#include <math.h>
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27,16,2);  //should change to 27 for real and prtieus 20
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#define RADIUS_OF_EARTH 6371 //earth radi in km



SoftwareSerial gpsSerial(3, 4);



TinyGPSPlus gps;

double lat1=0.0;
double lat2=0.0;
double lon1=0.0;
double lon2=0.0;
double distance = 0;
double bearing=0;
char lat1_str[20];
char lon1_str[20];
char charge="";
unsigned long startTime = 0;
char sending[60]; 
String messages="";

int flag_age=0;
int flag2=0;
int count=0;


int button5=0;
int button6=0;
int button7=0;


double toRadians(double degrees) {
  return degrees * M_PI / 180.0;
}


double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
  double deltaLat = toRadians(lat2 - lat1);
  double deltaLon = toRadians(lon2 - lon1);
  double a = pow(sin(deltaLat / 2), 2) + cos(toRadians(lat1)) * cos(toRadians(lat2)) * pow(sin(deltaLon / 2), 2);
  double c = 2 * atan2(sqrt(a), sqrt(1 - a));
  double distance = RADIUS_OF_EARTH * c;
  return distance;
}



double calculateBearing(double lat1, double lon1, double lat2, double lon2) {
  double deltaLon = toRadians(lon2 - lon1);
  double y = sin(deltaLon) * cos(toRadians(lat2));
  double x = cos(toRadians(lat1)) * sin(toRadians(lat2)) - sin(toRadians(lat1)) * cos(toRadians(lat2)) * cos(deltaLon);
  double bearing = atan2(y, x);
  bearing = fmod((bearing + 2 * M_PI), (2 * M_PI));
  return bearing;
}


void displayTimeAgo() {
  unsigned long currentTime = millis(); // Get current time
  unsigned long timeAgo = 0;            // Initialize time ago as 0

  if (flag_age == 1) {  // Check if flag_age is set to 1
    timeAgo = currentTime - startTime; // Calculate time ago in milliseconds
  }

  unsigned long secondsAgo = timeAgo / 1000;  // Convert to seconds

  unsigned long minutesAgo = secondsAgo / 60;      // Calculate minutes
  unsigned long hoursAgo = minutesAgo / 60;       // Calculate hours
  minutesAgo %= 60;                               // Get remaining minutes after calculating hours
  secondsAgo %= 60;                               // Get remaining seconds after calculating minutes

  if (hoursAgo > 0) {
    lcd.print(hoursAgo);
    lcd.print(" hr ago");
  } else if (minutesAgo > 0) {
    lcd.print(minutesAgo);
    lcd.print(" min ago");
  } else {
    lcd.print(secondsAgo);
    lcd.print(" sec ago");
  }
}






//function for converting word spaced string to array 

String* stringToArray(String inputString) {
  static String words[10]; // maximum of 10 words in the input string
  int numWords = 0;
  int wordStart = 0;
  int wordEnd = 0;
  
  while (wordEnd >= 0 && numWords < 10) {
    wordEnd = inputString.indexOf(' ', wordStart);
    if (wordEnd >= 0) {
      words[numWords] = inputString.substring(wordStart, wordEnd);
      numWords++;
      wordStart = wordEnd + 1;
    }
  }
  words[numWords] = inputString.substring(wordStart);
  numWords++;

  return words;
}





void setup() {

  lcd.begin();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
 
 

  pinMode(5,INPUT);
  pinMode(7,INPUT);
  pinMode(6,INPUT);
  pinMode(A0,OUTPUT);
  pinMode(A1,OUTPUT);
  pinMode(9,OUTPUT);

  Serial.begin(4800);
  gpsSerial.begin(9600);
  //HC12.begin(4800); 

  lcd.setCursor(1,0);   //Set cursor to character 2 on line 0
  lcd.print("ClickGuard");
  lcd.setCursor(1,1);   //Set cursor to character 2 on line 0
  lcd.print("Powering");

  delay(5000);
  lcd.clear();

  }


void loop() {


  
int x=analogRead(A2);
charge=map(x, 600, 860, 1, 100);

if (charge>80) {charge='H';}
else if (charge>70) {charge='A';}
else  {charge='L';}




  
  button5=digitalRead(5);
  button6=digitalRead(6);
  button7=digitalRead(7);

  delay(100);
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      // If a new GPS data is available, update the latitude and longitude
      lat1 = gps.location.lat();
      lon1 = gps.location.lng();
      
      dtostrf(lat1, 8, 9, lat1_str); // 8 = total number of characters (including decimal point), 6 = number of decimal places
      dtostrf(lon1, 9, 9, lon1_str); // 
//      lat1 = atof(lat1_str);
//      lon1 = atof(lon1_str);
  
    }
    
}

delay(100);
  
    

if ((button6==HIGH)&&(lat1*lat2>1) ){
  if (distance<2000){
    lcd.clear();
    lcd.setCursor(1,0);   //Set cursor to character 2 on line 0
    lcd.print(distance*1000);
    lcd.print("m");
    lcd.print("  ");
    
    lcd.print(bearing * 180.0 / M_PI);
    lcd.print("'");
    lcd.print("                   "); 

    lcd.setCursor(1,1);
    displayTimeAgo();  // Call the function to display time ago
  
    
    }

}





else if ((button5 == HIGH)&&(lat1>0)) {  //need to check lat>1 and 
    // Use strcpy and strcat to concatenate strings
      
      strcpy(sending, "9 ");
      strcat(sending, lat1_str);
      strcat(sending, " ");
      strcat(sending, lon1_str);
      Serial.write(sending); // Send the message
       lcd.clear();
       lcd.setCursor(1,0);   //Set cursor to character 2 on line 0
       lcd.print("Alerting");
       
       lcd.setCursor(12,1);
       lcd.print(charge);
       lcd.print("%");
       digitalWrite(9,HIGH);
       flag2=0;
       delay(1000);
      
    
}

else {
digitalWrite(9,LOW);
messages = Serial.readString();
//String messages="15767686788";
delay(15);
String* message_array = stringToArray(messages);  //convert message to array


if(messages[0]  == '9'){                              //7 is for encryption
                       
   lat2 = atof(message_array[1].c_str());                            
   lon2 = atof(message_array[2].c_str()); 
   flag2=1;
   count=0;
   flag_age=0;
   flag_age=1;
   startTime=millis();
   
    }




if ((flag2 * lat1* lat2)>1 ) {

  distance = calculateDistance(lat1, lon1, lat2, lon2);
  bearing = calculateBearing(lat1, lon1, lat2, lon2);

  if (distance<2000) {
  
    while (count<25){
 
        lcd.clear();

        lcd.setCursor(1,0);
        lcd.print("Emergency Mode");
        lcd.setCursor(1,1);   //Set cursor to character 2 on line 0
        lcd.print(distance*1000);
        lcd.print("m");
        lcd.print("  ");
        
        lcd.print(bearing * 180.0 / M_PI);
        lcd.print("'");
        lcd.print("                   "); 
        
      
        analogWrite(A0,200);
        analogWrite(A1,150);
        delay(200);  // allow them to off and help for for blinking
        
        analogWrite(A1,0);
        analogWrite(A0,0);
        delay(200);
    
        count+=1;
        
    
    
    
       }
    
    }

  
  
  
  flag2=0;
  
  
}







 

if (lat1<1 && lon1<1){ 
  lcd.clear();
  lcd.setCursor(1,0);   //Set cursor to character 2 on line 0
  lcd.print("GPS Loading..");
  
  lcd.setCursor(15,1);
  lcd.print(charge);
  }
  
else if (lat1>1 && lon1>1){
  lcd.clear();
  lcd.setCursor(1,0);   //Set cursor to character 2 on line 0
  lcd.print("Emergency Mode");
  
  lcd.setCursor(15,1);
  lcd.print(charge);
  

  }

}

}
