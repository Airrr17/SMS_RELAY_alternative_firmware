void def_eeprom() {
  Serial1.println(F("EEPROM FACTORY DEFAULTS LOADING"));
  for (word k = 0; k < 1024; k++) {      //1024 bytes    //Serial1.println(EEPROM.length()); //1024
    EEPROM.write(k, 255);
    digitalWrite(LED2_PIN, !digitalRead(LED2_PIN));
  }
  Serial1.println(F("DONE."));

  Serial1.print(F("EEPROM: "));
  for (word k = 0; k < 1024; k++) {
    Serial1.print(EEPROM.read(k));
  }
  Serial1.println(F(" :EEPROM END"));
  Serial1.println(F("DONE."));
  while (true) {
    digitalWrite(LED1_PIN, HIGH);
    digitalWrite(LED2_PIN, HIGH);
    delay(25);
    digitalWrite(LED1_PIN, LOW);
    digitalWrite(LED2_PIN, LOW);
    delay(25);
    Serial1.println(F("Power cycle to format."));
  }
}



void format_eeprom() {
  if (debug == true) Serial1.println(F("Fromatting... Wait until both leds start reblinking."));
  for (word k = 0; k < 1024; k++) {      //1024 bytes    //Serial1.println(EEPROM.length()); //1024
    EEPROM.write(k, 0);
    digitalWrite(LED1_PIN, !digitalRead(LED1_PIN));
  }
  writeString(0, phoneA);
  writeString(100, phoneB);
  writeString(200, phone0);
  writeString(300, phone1);
  writeString(400, phone2);
  writeString(500, phone3);
  writeString(600, phone4);

  Serial1.println(F("DONE."));
  read_all_eeprom();
}


void read_all_eeprom() {                 //And check for new device
  word check = 0;
  byte eep = 0;
  Serial1.print(F("EEPROM: "));
  for (word k = 0; k < 1024; k++) {
    eep = EEPROM.read(k);
    if (debug == true) Serial1.print(eep);
    if (eep == 255) check++;
  }
  Serial1.println(F(" :EEPROM END"));

  if (check > 500) {                   //new device detected!
    Serial1.println(F("Unformatted device found!"));
    for (byte b = 0; b < 30; b++) {
      digitalWrite(LED1_PIN, LOW);
      digitalWrite(LED2_PIN, LOW);
      delay(100);
      digitalWrite(LED1_PIN, HIGH);
      digitalWrite(LED2_PIN, HIGH);
      delay(100);
    }
    format_eeprom();
    return;
  }


  //Cont:
  Serial1.println(F("Apply EEPROM, setting relays..."));

  relay1 = EEPROM.read(relay1Addr);
  relay2 = EEPROM.read(relay2Addr);

  phoneA = read_String(0);
  phoneB = read_String(100);
  phone0 = read_String(200);
  phone1 = read_String(300);
  phone2 = read_String(400);
  phone3 = read_String(500);
  phone4 = read_String(600);

  if ((phoneA == "0000000000000") && (phoneB == "0000000000000")) {
    Serial1.println(F("---NOT ACTIVATED---"));
    act = false;
  }




}



void writeString(int add, String data)
{
  int _size = data.length();
  for (int i = 0; i < _size; i++)
  {
    EEPROM.put(add + i, data[i]);
  }
  EEPROM.put(add + _size, '\0'); //Add termination null character for String Data

}



String read_String(int add)
{
  int i;
  char data[20];                //Max 20 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 20) //Read until null character
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}
