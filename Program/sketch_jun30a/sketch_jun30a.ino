#include <MsTimer2.h>
#include <SoftwareSerial.h>

#define SIM800_TX_PIN 2
#define SIM800_RX_PIN 3
#define LAMA_TANGISAN 500 //500ms = 0.5 detik
#define LAMA_TLF 30000 //2000ms = 2 detik
#define LAMA_SMS 10000 //2000ms = 2 detik
#define SMS_MAX 1
#define RESET 0
#define BANYAK_SAMPLE 300
#define TRIG 10
#define RESET_PIN 12

SoftwareSerial gsm(SIM800_TX_PIN, SIM800_RX_PIN); //(X,RX)


int dataSuara [BANYAK_SAMPLE];
int maxAmp, minAmp, hitungSuara, delayTlf = 0, delaySMS = 0;
byte minmax, hitungSMS = 0;
boolean trigSuara = 0;
boolean trigTlf = 0;
boolean tombolReset = 0, onCall = 0, onSMS = 0;
String data;

int ledSystem = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  pinMode(A2, INPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  Serial.begin(9600);
  gsm.begin(9600);               // the GPRS baud rate

  digitalWrite(13, 1);
  delay(100);
  digitalWrite(13, 0);
  delay(100);
  digitalWrite(13, 1);
  delay(100);
  digitalWrite(13, 0);
  delay(100);
  digitalWrite(13, 1);
  delay(100);
  digitalWrite(13, 0);
  delay(100);
  MsTimer2::set(1, MSTimer2); // 1ms period
  MsTimer2::start();
  gsmInit();

}

void loop() {
  if (!onCall && !onSMS)
  {
    ambilDataSuara();
    cekDataSuara();
    tombolReset = digitalRead(RESET_PIN);
    if (tombolReset == 0)
    {
      Serial.println("reset");
      reset();
    }
  }
}

void reset()
{
  hitungSuara = RESET;
  trigTlf = RESET;
  hitungSMS = RESET;
  trigSuara = RESET;
}

void MSTimer2()   //counter 1ms
{
  if (trigSuara == 1)
    hitungSuara++;
  if (hitungSuara > LAMA_TANGISAN)
  {
    sms();
    hitungSuara = 0;
  }
  //Serial.println(hitungSuara);
  if (onCall == 1)
  {
    delayTlf++;
    hitungSuara = 0;
    digitalWrite(13, 1);
  }
  else
    digitalWrite(13, 0);

  if (delayTlf > LAMA_TLF)
  {
    onCall = 0;
    delayTlf = 0;
  }

  if (onSMS == 1)
  {
    hitungSuara = 0;
    delaySMS++;
    digitalWrite(13, 1);
  }
  else
    digitalWrite(13, 0);

  if (delaySMS > LAMA_SMS)
  {
    onSMS = 0;
    delaySMS = 0;
  }
}

void sms()
{
  onSMS = 1;
  if (hitungSMS + 1 > SMS_MAX)
    tlf();
  if (trigTlf == 0)
  {
    hitungSMS++; //Set SMS format to ASCII
    gsm.write("AT+CMGF=1\r\n");
    delay(1000);

    //HEADER
    gsm.write("AT+CMGS=\"085346414910\"\r\n");            //ganti nomer hp
    delay(1000);
    //ISI
    gsm.write("SOUND DETECTED");
    delay(1000);
    //END
    gsm.write((char)26);
    delay(1000);
    Serial.println("SMS Sent!");
  }
  hitungSuara = 0;
}

void tlf()
{
  trigTlf = 1;
  onCall = 1;
  gsm.println("ATD085346414910;");                      // ganti nomer hp
  Serial.println("CALLING");
}

void ambilDataSuara() {
  maxAmp = 0;
  minAmp = 1024;
  for (int a = 0; a < BANYAK_SAMPLE; a++)
  {
    dataSuara[a] = analogRead(A2);
    maxAmp = max(dataSuara[a], maxAmp);
    minAmp = min(dataSuara[a], minAmp);
  }
}

void cekDataSuara()
{
  minmax = maxAmp - minAmp;
  //Serial.print("minmax: ");
  //Serial.println(minmax);
  if (minmax > TRIG && trigSuara == 0 ) {
    trigSuara = 1;
  }
  else
    trigSuara = 0;
}

void gsmInit() {
  gsm.write("AT\r");
  delay(100);
  gsm.write((char)26);//the ASCII code of the ctrl+z is 26
  delay(100);
  byte a = 0;
  byte b = 0;
  while (1)
  {
    char kata;
    if (gsm.available() > 0)
    {
      Serial.print(gsm.readString());
      goto gsmEndsInit;
    }
    b++;
    //Serial.println(b);
    if (b == 50)
    {
      b = 0;
    }
  }
gsmEndsInit:
  Serial.print("\nDone\n");
  gsm.write("AT+CMGF=1\r");
  delay(200);
  gsm.write("AT+CMGL=\"REC UNREAD\"\r"); // to read ALL the SMS in text mode
  delay(200);
  Serial.println("READY");
}
