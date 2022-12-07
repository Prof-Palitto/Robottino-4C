#include <SoftwareSerial.h>

#define MONITOR

#define rxPin 10
#define txPin 8
#define In1 2 //Pin D2 dell'Arduino e' collegato al Input 1 del ponte H
#define In2 4 //Pin D4 dell'Arduino e' collegato al Input 2 del ponte H
#define In3 6 //Pin D6 dell'Arduino e' collegato al Input 3 del ponte H
#define In4 7 //Pin D7 dell'Arduino e' collegato al Input 4 del ponte H
#define EnM1 5 //M1 motore di sinistra
#define EnM2 9 //M2 motore destro
// Set up a new SoftwareSerial object
SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

String check4answer(){
    String str = "";
    //while (mySerial.available() == 0) {}
    while (mySerial.available() > 0) {
      char c = mySerial.read();
      str += String(c);
    }
    #ifdef MONITOR
    Serial.println(str);
    #endif
    return str;
}


String esp01cmd(String cmd) {
    #ifdef MONITOR
  Serial.println("sending: " + cmd);
    #endif
  mySerial.println(cmd);
  delay(10);
  return check4answer();
}

String cellphoneIP = "";
String str = "";

void setup()  {
    // Define pin modes for TX and RX
    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);
    pinMode(In1, OUTPUT);
    pinMode(In2, OUTPUT);
    pinMode(In3, OUTPUT);
    pinMode(In4, OUTPUT);
    pinMode(EnM1, OUTPUT);
    pinMode(EnM2, OUTPUT);
    digitalWrite(EnM1, HIGH);
    digitalWrite(EnM2, HIGH);
    
    // Set the baud rate for the SoftwareSerial object
    #ifdef MONITOR
    Serial.begin(9600);
    #endif

// nel caso in cui ESP01 sia settato alla velocità di trasmissione 11500
// che per la softserial risulta troppo veloce, possiamo andare a settare
// la velocità a 9600. basterà eseguire il seguente codice una volta solamente.
//    mySerial.begin(115200);
//    delay(100);
//    esp01cmd("AT+UART=9600,8,1,0,0");
//    delay(1000);
//    mySerial.end();
//    delay(1000);


    mySerial.begin(9600);
    delay(1000);
    esp01cmd("AT");
    delay(1000);
    esp01cmd("AT+CWMODE=2");
    delay(1000);
    esp01cmd("AT+CWSAP=\"robottino\",\"robottino\",1,4");
    delay(1000);
    esp01cmd("AT+CIFSR"); //show AP IP address
    esp01cmd("AT+CIPMUX=1"); //allow up to 1 connections at the time
    
    
    #ifdef MONITOR
    Serial.println("ESP-01 Configuration Completed");
    #endif
    avanti(1000);

    while(str.substring(11,18) != "192.168") {
    #ifdef MONITOR
      Serial.println("no connections so far... still waiting");
    #endif
      delay(1000);
      str = esp01cmd("AT+CWLIF");
    }
    int startOfSTR = str.indexOf(',',18);  //IP finsce prima della virgola
    cellphoneIP = str.substring(11,startOfSTR);
    printlnWIFI("collegato a: " + cellphoneIP, cellphoneIP);
}

void avanti(int durata){
    digitalWrite(In1, LOW); //M1 avanti
    digitalWrite(In2, HIGH);
    digitalWrite(In3, LOW); //M2 avanti
    digitalWrite(In4, HIGH);
    delay(durata);
    digitalWrite(In4, LOW); //stop
    digitalWrite(In2, LOW);
}
void indietro(int durata){
    digitalWrite(In1, HIGH); //M1 indietro
    digitalWrite(In2, LOW);
    digitalWrite(In3, HIGH); //M2 indietro
    digitalWrite(In4, LOW);
    delay(durata);
    digitalWrite(In3, LOW); //stop
    digitalWrite(In1, LOW);
}
void destra(int durata){
    digitalWrite(In1, HIGH); //M1 indietro
    digitalWrite(In2, LOW);
    digitalWrite(In3, LOW); //M2 avanti
    digitalWrite(In4, HIGH);
    delay(durata);
    digitalWrite(In4, LOW); //stop
    digitalWrite(In1, LOW);
}
void sinistra(int durata){
    digitalWrite(In1, LOW); //M1 avanti
    digitalWrite(In2, HIGH);
    digitalWrite(In3, HIGH); //M2 indietro
    digitalWrite(In4, LOW);
    delay(durata);
    digitalWrite(In3, LOW); //stop
    digitalWrite(In2, LOW);
}

void printlnWIFI(String str, String cellIP) {
      if(str != "") {
    #ifdef MONITOR
        Serial.println("Received from UDP Monitor: "+str);
    #endif
        //String str1 = "AT+CIPSEND=1," + str.length(); NOT WORKING??? bug???
        String str1 = "AT+CIPSEND=3,";
        str1 = str1 + str.length() + ",\"" + cellIP + "\",1234";
        //str1.concat(str.length());
        //Serial.println(str1);
        esp01cmd(str1);
        esp01cmd(str);        
      }    
}

void loop() {
    #ifdef MONITOR
    Serial.println("loop...");
    #endif

    #ifdef MONITOR
    Serial.println(cellphoneIP);
    Serial.println("Connection from remote device was Established!!!");
    #endif

    // AT+CIPSTART=<id>,<type>,<remote address>,<remote port>[,(<local port>),(<mode>)]
    // AT+CIPSEND=[<id>,]<length>[,<ip>,<port>]
    // AT+CIPCLOSE=<id>
    
    //Socket Server: server in ascolto, pronto a ricevere pacchetti UDP da WIFI
    //Socket ID: 3
    //accept packets from any IP address/devices
    //Listen to local port 4567
    //outgoing packets could go to any remote host without restrictions...
    esp01cmd("AT+CIPCLOSE=3"); //close socket if for any reason it was already open
    esp01cmd("AT+CIPSTART=3,\"UDP\",\"0.0.0.0\",0,4567,2"); //starting UDP Socket Server 

    
    //esp01cmd("AT+CIPSTART=1,\"UDP\",\""+cellphoneIP+"\",1234"); //starting UDP Socket Client 
    
    delay(3000);

    while(true) {

      // dati ricevuti da Modulo WIFI
      str = mySerial.readString();
      if(str != "") {

        int startOfSTR = str.indexOf(":",10)+1;
        printlnWIFI("Ricevuto: " + str, cellphoneIP);
    #ifdef MONITOR
        Serial.println("Received: "+str);
        Serial.println("Message: "+str.substring(startOfSTR));
    #endif
        str = str.substring(startOfSTR);
          
        char cmd = str[0];
        int durata = str.substring(1).toInt();
  
    #ifdef MONITOR
        Serial.println(cmd);
    #endif
  
        switch (cmd){
        case 'a': 
            printlnWIFI("Avanti di " + String(durata), cellphoneIP);
            avanti(durata);
            break;
        case 'i': 
            printlnWIFI("Indietro di " + String(durata), cellphoneIP);
            indietro(durata);
            break;
        case 'd': 
            printlnWIFI("destra di " + String(durata), cellphoneIP);
            destra(durata);
            break;
        case 's': 
            printlnWIFI("sinistra di " + String(durata), cellphoneIP);
            sinistra(durata);
            break;
        default:
            printlnWIFI("Comando non esistente: ", cellphoneIP);
            break;
        }
          
      }

    #ifdef MONITOR
      // dati ricevuti da Monitor Seriale
      str = Serial.readString(); 
      printlnWIFI(str, cellphoneIP);
    #endif
}
}
