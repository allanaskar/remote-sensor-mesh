/*
 *  This sketch file is for Udp Client to broadcast sensor data
 *  Author : Allan Askar
 *  Created: 20424
 *  Usage  : Add this to your arduino project and call setup 
 *      
 *      Version: v1.0 20424 Initial
 */

#include <WiFi.h>
#include <WiFiUdp.h>

  WiFiUDP udpC;

unsigned int localPort = 2424;      // local port to send udp packets
IPAddress hubIPAddress(233,233,233,233);  // Multicast address on hub

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet,

void setupUdpClient(){  // Start the client
  udpC.begin(localPort);
}

void sendData(char* msg) { // Send udp data to multicast address
    udpC.beginPacket(hubIPAddress, localPort);
    udpC.write(msg);  
    udpC.endPacket(); 
}
