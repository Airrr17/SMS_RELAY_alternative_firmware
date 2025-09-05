//SIM800C                                                                                             Generic STM32F0 series -> Generic F030C8Tx         stm32f030c8t6
//////////////////////////////////////////////////////
//GSM dual relay board alternative firmware.
//Tested with "DIYMall UNV v3.2-2018.10-11" blue board with stm32f030c8t6.
//
//Version: 0.9, 02.03.2025                 (c)Airrr(r)
//
//To activate the unit send SMS "activate" first. This number will become admin (A). Then SMS "help" for the list of commands.
//
//SMS commands:
//1normal, 1stop       - relay commands
//2normal, 2stop       - state will be stored in eeprom and loaded at startup.
//numbers              - get list of all 7 phone numbers allowed to control the device. (Super admins only!). Admin will receive five numbers list (0-4).
//status               - get relay status.
//signal               - get operator name and signal quality. 0 to 31 where 0-poor, 31-full, 99-unknown.
//REPx:+123456789012   - replace phone numbers with new ones. Where x=A,B,0,1,2,3,4. A and B are super admins, 0-admin, 1-4 users. Phone format: +xxxxxxxxxxxx (12 digits)
//                                                  to remove unwanted namber just replace him with dummy phone number like (+000000000000). (Super admins\admin only!)
//                                                  Super admin (A&B) can do everything.
//                                                  Admin (0) can change and see numbers 0-4 only.
//                                                  User (1-4) cannot change or see any phone numbers.
//resetall             - reset the device to very factory defaults (all 255). Will format then (to all 0). (Super admins only!)
//
//
//Tie LOG pin to GND at startup to force device reformat (all 0). Wait and then activate with your phone #.
//
//PA9  pin - TX  Debug port: a LOT of information, just read! May be parsed and used by another microcontroller to act somehow.
//PA10 pin - RX  115200 3.3v
//
//Serial commands: (only works while debug variable == true)
//op        - print cellular operator name.
//reg       - is registered to network, bool.
//list      - print SMS indexes.
//del       - delete all SMSes
//format    - same as 'resetall' SMS
//signal    - print signal quality.
//help      - some help.


#include "GSMSim.h"
#include "GSMSimSMS.h"
#include <EEPROM.h>


#define RELAY1_PIN          PA0                 //Relay 1
#define RELAY2_PIN          PA1                 //Relay 2
#define RESET_PIN           PA11                //Sim800l reset.           DUMMY!!!
#define LED1_PIN            PB15
#define LED2_PIN            PB14
#define GSM_PWR             PA5                 //GSM PWR_KEY
#define LED_NET             PA4                 //INPUT from sim800        not used for now
#define LOG_PIN             PB10                //LOG pin

GSMSim gsm(Serial, RESET_PIN);
GSMSimSMS sms(Serial, RESET_PIN);               //GSMSimSMS inherit from GSMSim. You can use GSMSim methods with it.

const bool debug = true;                        //All debug info. false = much less info and no serial commands.
const bool demo = true;
byte skoka = 99;
bool act = true;
bool relay1 = false;
bool relay2 = false;
int relay1Addr = 1000;
int relay2Addr = 1001;

String phoneA = "0000000000000";                //13 chars including "+" @ start
String phoneB = "0000000000000";
String phone0 = "0000000000000";
String phone1 = "0000000000000";
String phone2 = "0000000000000";
String phone3 = "0000000000000";
String phone4 = "0000000000000";
String sender = "0000000000000";
String msg = "";

HardwareSerial Serial1(PA10, PA9);                //debug serial//  RX    TX


void setup() {
  pinMode(RELAY1_PIN, OUTPUT);
  pinMode(RELAY2_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED_NET, INPUT);
  pinMode(GSM_PWR, OUTPUT);
  pinMode(LOG_PIN, INPUT_PULLUP);

  Serial1.begin(115200);                           //Serial for DEBUG...
  Serial.begin(115200);                            //Sim800c radio. PA2, PA3

  startup();

  Serial1.println(F("Version: 0.9, 01.03.2025            (c)Airrr(r)"));
  Serial1.println(F("GSM RELAY Starting..."));

  if (debug == false) Serial1.println(F("---WAIT---"));
  if (demo == true) Serial1.println(F("---DEMO---VERSION---"));
  if (debug == true) Serial1.println(F("---DEBUG---VERSION---"));

  delay(100);
  if (digitalRead(LOG_PIN) == 0) format_eeprom();
  read_all_eeprom();

  //Apply relays:
  digitalWrite(RELAY1_PIN, relay1);
  delay(15);                                       //To prevent current burst
  digitalWrite(RELAY2_PIN, relay2);

  if (debug == true) {
    Serial1.print ("R1: ");
    Serial1.print (relay1);
    Serial1.print (", ");
    Serial1.print ("R2: ");
    Serial1.println (relay2);

    Serial1.print("#: ");
    Serial1.print(phoneA);
    Serial1.print(", ");
    Serial1.print(phoneB);
    Serial1.print(", ");
    Serial1.print(phone0);
    Serial1.print(", ");
    Serial1.print(phone1);
    Serial1.print(", ");
    Serial1.print(phone2);
    Serial1.print(", ");
    Serial1.print(phone3);
    Serial1.print(", ");
    Serial1.println(phone4);
  }

  if (debug == true) Serial1.print(F("GSM power on..."));

  /////////////////////////////////////////////////////////////////////////////////////////////////////REAL POWER ON//
  digitalWrite (GSM_PWR, HIGH);
  delay(1100);
  digitalWrite (GSM_PWR, LOW);
  if (debug == true) Serial1.println(F("OK"));
  delay(2000);

  gsm.init();

  if (debug == true) {
    Serial1.print(F("GSM manufacturer: "));
    Serial1.println(gsm.moduleManufacturer());
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(500);
    Serial1.print(F("GSM model: "));
    Serial1.println(gsm.moduleModel());
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(500);
    Serial1.print(F("GSM revision: "));
    Serial1.println(gsm.moduleRevision());
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(500);
    Serial1.print(F("GSM IMEI: "));
    Serial1.println(gsm.moduleIMEI());
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(500);
  }

  sms.init();

  if (debug == true) {
    Serial1.print(F("Set Phone Function: "));
    Serial1.println(sms.setPhoneFunc(1));
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(500);
    Serial1.print(F("Registered to Network?: "));
    Serial1.println(sms.isRegistered());
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(500);
    Serial1.print(F("Signal Quality: "));
    Serial1.println(sms.signalQuality());
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(500);
    Serial1.print(F("Operator Name: "));
    Serial1.println(sms.operatorName());
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(500);

    Serial1.print(F("Init SMS: "));
    Serial1.println(sms.initSMS());
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(700);

    Serial1.print(F("List Unread SMS: "));
    Serial1.println(sms.list(true));
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
    delay(700);
  }

  digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
  sms.sendATCommand("AT+CMGDA=\"DEL ALL\"");  //Delete all!
  delay(500);

  Serial1.println(F("GO!!!"));
}







void loop() {
  //  if (Serial.available()) {
  //    Serial1.println(Serial.readString());
  //  }

  if ((Serial1.available()) && (debug == true)) {
    String r = Serial1.readString();
    //Serial.println(r);

    //local serial operations:
    if (r == "op") Serial1.print(F("Operator name: ")), Serial1.println(sms.operatorName());
    if (r == "reg") Serial1.print(F("Registered: ")), Serial1.println(sms.isRegistered());
    if (r == "list") Serial1.println(sms.list(false));                                                                                                                        //If onlyUnread equal to false, method returns ALL messages.
    if (r == "del") sms.sendATCommand("AT+CMGDA=\"DEL ALL\""), delay(500);                                                                                                    //sms.deleteAll();
    if (r == "format") Serial1.println(F("Resetting the device. Wait until both leds blinks fast together then power cycle. Then wait another few minutes.")), def_eeprom();  //same as 'resetall' SMS
    if (r == "signal") Serial1.print(F("Signal: ")), Serial1.println(sms.signalQuality());                                                                                    //print signal quality.
    if (r == "help") Serial1.println(F("Commands: op - operator name, reg - is registerd, list - sms indexes, del - del all sms, format - factory reset, signal - 0-to-31"));

  }


  if (Serial.available()) {                                        //got sms!
    String buffer = Serial.readString();

    if (buffer.indexOf("+CMTI:") != -1) {

      if (debug == true) Serial1.print(F("SMS Index #: "));
      int indexno = sms.indexFromSerial(buffer);
      if (debug == true) Serial1.println(indexno);

      Serial1.print(F("Sender#: "));
      sender = sms.getSenderNo(indexno);
      if (debug == true) Serial1.println(sender);

      String rawMSG = sms.readFromSerial(buffer);
      if (debug == true) Serial1.print(F("RAW: "));
      if (debug == true) Serial1.println(rawMSG);

      msg = rawMSG.substring(rawMSG.indexOf("SAGE:") + 5);
      Serial1.print(F("Message: "));
      Serial1.println(msg);

      sms.deleteOne(indexno);

    } else {
      Serial1.println(buffer);
      if (buffer.indexOf("ERROR") != -1) {
        if (debug == true) Serial1.print(F("Some GSM ERROR. Check with your provider. (out of SMSes?)"));
        for (byte b = 0; b < 30; b++) {
          digitalWrite(LED1_PIN, LOW);
          digitalWrite(LED2_PIN, LOW);
          delay(100);
          digitalWrite(LED1_PIN, HIGH);
          digitalWrite(LED2_PIN, HIGH);
          delay(100);
        }
      }
      buffer = "";
      return;
    }


    if (debug == true) Serial1.println(F("Deleting all SMSes."));
    sms.sendATCommand("AT+CMGDA=\"DEL ALL\"");  //Delete all!
    delay(500);
    if (debug == true) Serial1.println(sms.list(false));


    ops(sender, msg);           //GO operations!


  }







  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, HIGH);
  delay(100);
  digitalWrite(LED1_PIN, HIGH);
  digitalWrite(LED2_PIN, LOW);
  delay(100);

}






void startup() {
  delay(100);
  Serial1.println("");
  Serial1.print("-");
  for (byte j = 0; j < 15; j++) {
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, LOW);
    delay(50);
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, HIGH);
    delay(50);
    Serial1.print("---");
  }
  digitalWrite(LED1_PIN, HIGH);
  Serial1.println("-");
}
