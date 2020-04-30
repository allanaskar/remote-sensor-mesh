/*
 *  This sketch implements WiFi Sensor for RSM v1: https://github.com/allanaskar/remote-sensor-mesh 
 *  Author : Allan Askar
 *  Created: 2020, April 24
 *  Hardware:
 *      m5: https://m5stack.com/products/stick-c
 *  Usage  : The sensor is in the edge connected to environment (Temp/Humidity/etc)
 *    and finds strong WiFi based on 'IoT-RSM-d*' wildcard. 
 *    
 *    After WiFi is a Go it brodcasts on port 2424 via Multicast 233.233.233.233
 *      
 *      Version: v0.1 20424 Initial version
 *               
 *  Main Loop: 
 * 1. Init 'Blank' state if never executed. Ran once.
 * 2. Run 'Provisioning' if in 'Blank'. See details in R1.
 * 3. Run 'Reset Check' if in 'Testing' state. See R2.
 * 4. Set 'Mission' state if in 'Testing' state and RTC elapsed 1 hour. Lower power
 * 5. Run 'Testing' code if in 'Testing' state.
 * 6. Run 'Mission' code if in 'Mission' state.
 * 
 */

#define M5_BOARD 1

#include <M5StickC.h>

#define DEBUG 1

#define HAS_DISPLAY 1

struct State {
  // Device State
  byte  IP3;            // Last octet of IP v4 address
  bool  WiFiConnected;  // Flag to be true if we are connected
  
  int   ReportingPeriod;// Time period in 100 ms for loop to sensor all inputs and report  
  bool  DisplayOn;      // Flag to be true if display is on (false for low power mode)
  int   RunHours;       // Number of hours running since last reboot/reset
  
  // Inputs
  int   BatVoltage;       // Voltage of the battery
};

State st = { 
    // Device State
  -1,//byte/IP3               // Last octet of IP v4 address
  false,//bool/WiFiConnected  // Flag to be true if we are connected
  50,//int/ReportingPeriod,   // Time period in 100 ms for loop to sensor all inputs and report
  false,//bool/DisplayOn,     // Flag to be true if display is on (false for low power mode)
  0,//int/RunHours,           // Number of hours running since last reboot/reset
  // Inputs
  0//int/BatVoltage,          // Voltage of the battery 
};

char ssid[48]; 
char password[48];

char* ssid = "IoT-RSM-d";     // Ex: IoT-RSM-demo
char* passcode = "pa$$-dXXX"; // Ex: pa$$-d007

const char* PN = "RSM-dSens-1.0"; // v1.0.
const char* DT = "20424";

IPAddress ip;

// Inventory
// 20424 1 & 2 -> M5,WiFi,OLED

const int snNum = 0; 
char * SN = "de"; // demo SN
const char * ReconnectMsg = " de|WiFiRestart ";

// Hardware specifics
#if defined M5_BOARD && M5_BOARD==1
  const char * Module = "M5 v1";
#else  
  const char * Module = "Unknown";
#endif

char msg[100];
char displayMsg[100];
char ccResponds[100];
const char * ccOkResponds = "ok";

// COM vars
unsigned long wifiReconnectInDemo = 30; // 30s
unsigned long wifiReconnectNormalPower = 300; // 5m
unsigned long wifiReconnectLowPower = 3600; // 1h

bool setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    for(int i=0; i<10; i++) {
      delay(100);           // 100ms
    } 
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        st.WiFiConnected = false;
        return false; // No connection
    }
    ip = WiFi.localIP();
    st.WiFiConnected = true;

#if defined DEBUG && DEBUG==1
    Serial.println("WiFi Connected");
    Serial.println(ip);
#endif

    st.IP3 = ip[3];
        
    return true; // Good
}

void setup() {
    M5.begin();
    setupLed();

#if defined DEBUG && DEBUG==1
    Serial.begin(115200); // Speed rate
#endif    

    // Set WiFi to station mode and disconnect from an AP if it was previously connected
    WiFi.mode(WIFI_STA);  
    WiFi.disconnect();    
    delay(100);           // 100ms

#if defined HAS_DISPLAY && HAS_DISPLAY==1
    // Compile display code
    // TODO: Setup display and init with starting settings
#endif

#if defined DEBUG && DEBUG==1
    Serial.println("Setup done");
#endif 
}

void logo() {

    Serial.print("RSM-v1-Demo-");
    Serial.println(PN);
#if defined HAS_DISPLAY && HAS_DISPLAY==1 
    /*u8x8.clear();
    //u8x8.drawString(0, 0, "");
    u8x8.drawString(3, 1, "ALLAN");
    u8x8.drawString(3, 2, "      ASKAR");    
    //u8x8.drawString(0, 3, "");
    u8x8.drawString(0, 4, "PN:");
    u8x8.drawString(4, 4, PN);
    u8x8.drawString(0, 5, "SN:");
    u8x8.drawString(4, 5, SN);
    u8x8.drawString(7, 5, "DT:");
    u8x8.drawString(11, 5, DT);*/
#endif
}

void loop() { // Will be called for unlimited amount of time

  logo();
  //while(true) delay(1);

  int tempTimeShift = snNum;
  while(tempTimeShift>10) tempTimeShift=tempTimeShift-10;
  delay(3000+tempTimeShift*200);           // 3s plus SN

  st.CycleCount++;
  //cycle++;
  
  scanNet();
  
  unsigned long startClock = millis();
  unsigned long endClock = startClock+(wifiReconnect*1000);
  bool keepRunning = true;
    
  while(millis()<endClock && st.WiFiConnected) {

#if defined M5_BOARD && M5_BOARD==1    
    st.BatVoltage = M5.Axp.GetBatVoltage();
#if defined DEBUG && DEBUG==1
    sprintf(msg, "%s|%03u:%04u ", SN, st.IP3, st.BatVoltage);
    Serial.println(msg); 
#endif        
#endif    
                
  //for (int z = 0; z < 60; ++z) {
          
    // Broadcast
    sendData(msg);
   
#if defined HAS_DISPLAY && HAS_DISPLAY==1
    // Prep for Display and display
    sprintf(msg, "%04u", st.BatVoltage);
    //u8x8.drawString(0, 4, msg);    
#endif    
    //delay(1000);           // 1 s 
    //loopLed(false);   
  }
}

static void topDisplay() {
#if defined DEBUG && DEBUG==1  
    sprintf(displayMsg, " ACTIVE %s|%u", SN, st.IP3);
    Serial.println(displayMsg); 
#endif 
#if defined HAS_DISPLAY && HAS_DISPLAY==1 
    //u8x8.drawString(0, 0, displayMsg);
#endif   
}

//static void scanNet(int cycle, int matchCount)
static void scanNet() //
{
  bool matchIsFound = false;
  ssid[0]=0x00; password[0]=0x00;
    Serial.println("scan start");

    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    
#if defined HAS_DISPLAY && HAS_DISPLAY==1
    M5.Lcd.fillScreen(WHITE);
#endif   
    if (n == 0) {
#if defined DEBUG && DEBUG==1 
        Serial.println("no networks found");
#endif
#if defined HAS_DISPLAY && HAS_DISPLAY==1       
        //u8x8.drawString(0, 0, "  Searching ...");
        M5.Lcd.setCursor(0, 20, 2);
        M5.Lcd.setTextColor(RED, WHITE);
        M5.Lcd.printf("  Searching ...");
#endif        
    } else {
#if defined DEBUG && DEBUG==1 
        Serial.print(n);
        Serial.println(" networks found");
#endif      

        topDisplay();   
        int row=2;  // On Top "List:" then empty string
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            if(WiFi.RSSI(i)>-90) { // High signals only // Old value -80
              Serial.print(i + 1); Serial.print(": ");
              Serial.print(WiFi.SSID(i)); Serial.print(" (");
              Serial.print(WiFi.RSSI(i)); Serial.print(")");
              Serial.println((WiFi.encryptionType(i) == 0)?" ":"*"); // WIFI_AUTH_OPEN=0

              // Print SSID for each network found
              char currentSSID[16]; // Up to 16 chars on the line
              WiFi.SSID(i).toCharArray(currentSSID, 16);
              //u8x8.drawString(0, row++, currentSSID);

              // Check if we found a match (''IoT-RSM-d*'
              char temp[10];
              WiFi.SSID(i).toCharArray(temp, 10);
              //u8x8.drawString(10, row++, temp);

              if(strcmp(temp, ssid) ==0){
                matchIsFound = true; strcpy(password,passcode);
                strcat( password, currentSSID );
                Serial.println(password);
              }
              
              if(matchIsFound){
                // Stop the loop
                i=n;
                // Match
                strcpy(ssid,currentSSID);
#if defined HAS_DISPLAY && HAS_DISPLAY==1              
                //u8x8.drawString(0, row, currentSSID);
                M5.Lcd.printf(currentSSID);
#endif                
                row++;
                matchCount++;
                if (setupWiFi()) {                     
                  delay(90);
                  if(st.WiFiConnected) {
                    setupUdpClient();                
                  }
                } else return;// matchIsFound;
              }           
              delay(10);
            }
        }
#if defined HAS_DISPLAY && HAS_DISPLAY==1
        M5.Lcd.print.setCursor(2,7);
        M5.Lcd.print.print(matchCount);    // Display Match Counter
        M5.Lcd.print.print(" of ");    //        
        M5.Lcd.print.print(st.CycleCount);    // Display Loop Counter
#endif        
    }
#if defined DEBUG && DEBUG==1 
    Serial.println("");
#endif     

    // Wait a bit before scanning again
    delay(3000);
    //return matchIsFound;
}
