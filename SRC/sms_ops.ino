

void ops(String number, String mess) {

  if ((mess == "activate") && (act == false)) {
    act = true;
    writeString(0, number);
    phoneA = number;
    String MESSAGE = F("Activated!");
    MESSAGE.concat("\r\n");
    MESSAGE.concat(F("Send 'help' for list of commands."));
    if (debug == true) Serial1.println(MESSAGE);
    send_sms(phoneA, MESSAGE);
    number = "";
    mess = "";
  }

  if (act == false) {
    String MESSAGE = (F("Please activate first by sending SMS 'activate'."));
    Serial1.println(MESSAGE);
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
    if (debug == true) Serial1.println(F("Deleting all SMSes."));
    sms.sendATCommand("AT+CMGDA=\"DEL ALL\"");  //Delete all!
    delay(1000);
    number = "";
    mess = "";
    return;
  }

  byte who = 0;                              //1=superadmin, 2=admin, 3=user
  if (number == phone4) who = 3;
  if (number == phone3) who = 3;
  if (number == phone2) who = 3;
  if (number == phone1) who = 3;
  if (number == phone0) who = 2;
  if (number == phoneA) who = 1;
  if (number == phoneB) who = 1;

  if ((who == 0) || (mess.length() < 2)) {
    number = "";
    mess = "";
    if (debug == true) Serial1.println(F("Activating or BAD sms, ignore."));
  }

  if ((mess == "resetall") && (who == 1)) {
    String MESSAGE = (F("Resetting the device. Wait until both leds blinks fast together then power cycle. Then wait another few minutes."));
    if (debug == true) Serial1.println(MESSAGE);
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
    def_eeprom();
  }

  if (mess == "signal") {
    String MESSAGE = ("Operator: ");
    MESSAGE.concat(sms.operatorName());
    MESSAGE.concat("\r\n");
    MESSAGE.concat("Signal quality: ");
    MESSAGE.concat(sms.signalQuality());
    MESSAGE.concat("\r\n");
    MESSAGE.concat("\r\n");
    MESSAGE.concat("0-31: 0-poor, 31-full, 99-unkn.");
    if (debug == true) Serial1.println(MESSAGE);
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
  }

  if (mess == "1normal") {
    relay1 = 0;
    EEPROM.put(relay1Addr, relay1);
    digitalWrite(RELAY1_PIN, relay1);
    String MESSAGE = "relay1: normal";
    Serial1.println(MESSAGE);
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
  }
  if (mess == "1stop") {
    relay1 = 1;
    EEPROM.put(relay1Addr, relay1);
    digitalWrite(RELAY1_PIN, relay1);
    String MESSAGE = "relay1: stop";
    Serial1.println(MESSAGE);
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
  }
  if (mess == "2normal") {
    relay2 = 0;
    EEPROM.put(relay2Addr, relay2);
    digitalWrite(RELAY2_PIN, relay2);
    String MESSAGE = "relay2: normal";
    Serial1.println(MESSAGE);
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
  }
  if (mess == "2stop") {
    relay2 = 1;
    EEPROM.put(relay2Addr, relay2);
    digitalWrite(RELAY2_PIN, relay2);
    String MESSAGE = "relay2: stop";
    Serial1.println(MESSAGE);
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
  }


  if (mess == "status") {
    String MESSAGE = "Relay status:";
    MESSAGE.concat("\r\n");
    MESSAGE.concat("relay1: ");
    if (relay1 == 1) MESSAGE.concat("normal");
    if (relay1 == 0) MESSAGE.concat("stop");
    MESSAGE.concat("\r\n");
    MESSAGE.concat("relay2: ");
    if (relay2 == 1) MESSAGE.concat("normal");
    if (relay2 == 0) MESSAGE.concat("stop");
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
  }


  if ((mess == "numbers") && (who == 1)) {       //numbers for super admin
    send_numbers(number);
    number = "";
    mess = "";
  }

  if ((mess == "numbers") && (who == 2)) {       //numbers for admin
    send_numbers_short(number);
    number = "";
    mess = "";
  }

  if (mess == "help") {
    String MESSAGE = F("Commands: ");
    MESSAGE.concat("\r\n");
    MESSAGE.concat(F("1normal, 1stop")), MESSAGE.concat("\r\n");
    MESSAGE.concat(F("2normal, 2stop")), MESSAGE.concat("\r\n");
    if ((who == 1) || (who == 2)) MESSAGE.concat("numbers"), MESSAGE.concat("\r\n");
    MESSAGE.concat(F("status")), MESSAGE.concat("\r\n");
    MESSAGE.concat(F("signal")), MESSAGE.concat("\r\n");
    if ((who == 1) || (who == 2)) MESSAGE.concat(F("REPx:+012345678901")), MESSAGE.concat("\r\n");
    if (who == 1)  MESSAGE.concat(F("(x=A,B,0,1,2,3,4)")), MESSAGE.concat("\r\n");
    if (who == 2)  MESSAGE.concat(F("(x=0,1,2,3,4)")), MESSAGE.concat("\r\n");
    if (who == 1)  MESSAGE.concat(F("resetall")), MESSAGE.concat("\r\n");
    
    if ((who == 1) || (who == 2)) MESSAGE.concat("\r\n"), MESSAGE.concat(F("(To del. replace by dummy ie REPx:+000000000000)")), MESSAGE.concat("\r\n");

    MESSAGE.concat("\r\n");
    MESSAGE.concat(F("v0.9 (c)Airrr(r)"));
    send_sms(number, MESSAGE);
    number = "";
    mess = "";
  }


  if ((mess.indexOf("REP") == 0) && ((who == 1) || (who == 2))) {               //Grand filter: first, REP
    if (debug == true) Serial1.println(F("REPlace # detected."));
    String repn = mess.substring(3, 4);
    if (debug == true) Serial1.print("Replace position #: ");
    if (debug == true) Serial1.println(repn);

    if (mess.indexOf("+") == 5) {                                               //second, +
      if (debug == true) Serial1.println(F("'+' Detected."));
      String nn = mess.substring(5);

      if ((nn.length() < 11) || (nn.length() > 15)) {                           //third, length
        String MESSAGE = F("Wrong number length. Must be 11-15 including +. Abort, try again.");
        Serial1.println(MESSAGE);
        send_sms(number, MESSAGE);
        repn = "";
        return;
      }

      for (byte i = 1; i < nn.length(); i++) {                                  //forth, if numeric, but first(0) is a "+"
        if (!isDigit(nn.charAt(i))) {
          Serial1.println(F("Not numeric, abort."));
          String MESSAGE = F("Phone number is not numeric, abort, try again.");
          send_sms(number, MESSAGE);
          repn = "";
          return;
        }
      }
      if (debug == true) Serial1.print(F("Replace by #: "));
      if (debug == true) Serial1.println(nn);

      //REPLACE NEW NUMBER

      if ((repn == "A") && (who == 1)) {
        writeString(0, nn);
        phoneA = nn;
        send_numbers(number);
      }
      if ((repn == "B") && (who == 1)) {
        writeString(100, nn);
        phoneB = nn;
        send_numbers(number);
      }
      if (repn == "0") {
        writeString(200, nn);
        phone0 = nn;
        if (who == 1) send_numbers(number);
        if (who == 2) send_numbers_short(number);
      }
      if (repn == "1") {
        writeString(300, nn);
        phone1 = nn;
        if (who == 1) send_numbers(number);
        if (who == 2) send_numbers_short(number);
      }
      if (repn == "2") {
        writeString(400, nn);
        phone2 = nn;
        if (who == 1) send_numbers(number);
        if (who == 2) send_numbers_short(number);
      }
      if (repn == "3") {
        writeString(500, nn);
        phone3 = nn;
        if (who == 1) send_numbers(number);
        if (who == 2) send_numbers_short(number);
      }
      if (repn == "4") {
        writeString(600, nn);
        phone4 = nn;
        if (who == 1) send_numbers(number);
        if (who == 2) send_numbers_short(number);
      }
      if (debug == true) Serial1.print("DONE.");

    } else {
      String MESSAGE = F("Phone number must start with '+' and contain numbers, try again.");
      send_sms(number, MESSAGE);
      number = "";
      mess = "";
      if (debug == true) Serial1.println(MESSAGE);
    }




    number = "";
    mess = "";
  }







  //  if (debug == true) Serial1.println(F("Deleting all SMSes."));
  //  sms.sendATCommand("AT+CMGDA=\"DEL ALL\"");  //Delete all!
  //  delay(500);
}



void send_numbers(String number) {
  String MESSAGE = "Active#: ";
  MESSAGE.concat("\r\n");
  MESSAGE.concat("A:");
  MESSAGE.concat(phoneA);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("B:");
  MESSAGE.concat(phoneB);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("0:");
  MESSAGE.concat(phone0);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("1:");
  MESSAGE.concat(phone1);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("2:");
  MESSAGE.concat(phone2);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("3:");
  MESSAGE.concat(phone3);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("4:");
  MESSAGE.concat(phone4);
  send_sms(number, MESSAGE);
}

void send_numbers_short(String number) {
  String MESSAGE = "Active#: ";
  MESSAGE.concat("\r\n");
  MESSAGE.concat("0:");
  MESSAGE.concat(phone0);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("1:");
  MESSAGE.concat(phone1);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("2:");
  MESSAGE.concat(phone2);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("3:");
  MESSAGE.concat(phone3);
  MESSAGE.concat("\r\n");
  MESSAGE.concat("4:");
  MESSAGE.concat(phone4);
  send_sms(number, MESSAGE);
}



void send_sms(String number, String mess) {

  if ((demo == true) && (skoka == 1)) {
    String demotext = F("Trial version is over. Power cycle to continue or get a full version. You can still control the realys. Thanks for using.");
    mess = demotext;
    Serial1.println(mess);
  }

  int strLen = mess.length() + 1;
  char messageChar[strLen];
  mess.toCharArray(messageChar, strLen);

  int phLen = number.length() + 1;
  char phoneChar[phLen];
  number.toCharArray(phoneChar, phLen);

  sms.send(phoneChar, messageChar);

  if (debug == true) {
    Serial1.println(F("------------------>"));
    Serial1.print(F("Debug: send-sms-to: "));
    Serial1.println(phoneChar);
    Serial1.println(F("Debug: sms-content: "));
    Serial1.println(messageChar);
    Serial1.println(F("------------------>"));
  }
  if (demo == true) skoka--;
  if (skoka <= 1) skoka = 1;
}
