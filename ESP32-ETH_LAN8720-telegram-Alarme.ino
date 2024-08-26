 /*MemoArduino pour autocomplétion:
 * digitalRead() digitalWrite() pinMode() analogRead() analogReference() analogWrite() analogReadResolution() analogWriteResolution() noTone() pulseIn() pulseInLong() shiftIn() shiftOut() tone() delay() delayMicroseconds()
micros() millis() HIGH LOW INPUT OUTPUT  INPUT_PULLUP  LED_BUILTIN true false Floating Point Constants Integer Constants (unsigned int) (unsigned long) byte() char() float() int() long() word() loop() setup() 
 break continue do...while else for goto if return  switch...case  while  #define (define) #include (include)  (semicolon) {} (curly braces) abs() constrain() map() max() min() pow() sq() sqrt() 
 cos() sin() tan() isAlpha() isAlphaNumeric() isAscii() isControl() isDigit() isGraph() isHexadecimalDigit() isLowerCase() isPrintable() isPunct() isSpace() isUpperCase() isWhitespace() array bool boolean byte char double
float int long short size_t string String() unsigned char  unsigned int  unsigned long  void word random() randomSeed() bit() bitClear() bitRead() bitSet() bitWrite() highByte() lowByte() attachInterrupt() detachInterrupt() 
interrupts() noInterrupts() Serial Stream Keyboard Mouse const scope static volatile PROGMEM sizeof() 
Fonction serial: if(Serial) available() availableForWrite() begin() end() find() findUntil() flush() parseFloat() parseInt() peek() print() println() read() readBytes() readBytesUntil() readString() readStringUntil() setTimeout() write() serialEvent()
RISING FALLING
Opérateurs de Comparaison: != (not equal to)  < (less than)  <= (less than or equal to)  == (equal to)  > (greater than)  >= (greater than or equal to)
Opérateurs Booléens: ! (logical not)  && (logical and)  || (logical or)  --- Opérateurs d'accès aux Pointeurs: & (reference operator)  * (dereference operator) 
Opérateurs Bit à Bit: & (bitwise and)  << (bitshift left)  >> (bitshift right)  ^ (bitwise xor)  | (bitwise or)  ~ (bitwise not) 
Opérateurs Composés : %= (compound remainder)  &= (compound bitwise and)  *= (compound multiplication)  ++ (increment)  += (compound addition)  -- (decrement)  -= (compound subtraction)  /= (compound division)  ^= (compound bitwise xor)  |= (compound bitwise or) 

Resistances de tirage internes
* ---------------------------
    INPUT En fonction du périphérique d’entré utilisé, il faudra adapter le circuit. Par exemple, lorsqu’on utilise un bouton poussoir de type Momentary Switch,
    *  il faudra intégrer une résistance de tirage pull-up ou de rappel pull-down au circuit. Sans résistance de pull-up ou pull-down l’entrée de l’ESP32 est
    *  flottante. Le niveau logique est indéfini ce qui peut entraîner un fonctionnement erratique du programme. Par exemple ne pas détecter la pression sur 
    * un bouton ou au contraire détecter qu’il est enfoncé alors que ce n’est pas le cas.
    INPUT_PULLUP On utilise la résistance de tirage (PULL UP) de l’ESP32
    INPUT_PULLDOWN On utilise la résistance de rappel (PULL DOWN) de l’ESP32
Chaque résistance de pull-up ou pull-down a une valeur de 45 kΩ d’après le tableau DC Characteristics (3.3 V, 25 °C) de la documentation technique.
Pour lire une entrée numérique – comme un bouton – on utilise la fonction digitalRead(), qui prend comme argument le numéro de la broche du GPIO.


La puce ESP32 possède les broches de cerclage suivantes :
* GPIO UnR   avec resistance PULLUP 10k au 3,3v si port serie inutilisé pour bon demarrage
    GPIO 0 ok avec resistance PULLUP 10k au 3,3v
    GPIO 2 ok avec resistance PULLUP 10k au 3,3v
	
    GPIO 4 led flash    

*/ 
//ETH debut
#ifndef ETH_PHY_TYPE
#define ETH_PHY_TYPE  ETH_PHY_LAN8720
#define ETH_PHY_ADDR  1
#define ETH_PHY_MDC   23
#define ETH_PHY_MDIO  18
#define ETH_PHY_POWER 17
#define ETH_CLK_MODE  ETH_CLOCK_GPIO0_IN


#endif
#include <ETH.h>
static bool eth_connected = false;
//ETH fin

//telegram debut
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

static bool alarmeStatut = true;
static bool verrouDetection = false;

//detection mouvements
#define PIN_PIR 13

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"
unsigned long BOT_MTBS = 1000; // mean time between scan messages
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
unsigned long bot_lasttime; // last time messages' scan has been done
String chat_id;

//configuration serveur horloge
const char* ntpServer = "fr.pool.ntp.org";

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++)
  {
    chat_id = bot.messages[i].chat_id;
    String text = bot.messages[i].text;

   String from_name = bot.messages[i].from_name;
    if (from_name == "")
      from_name = "Guest";

    if (text == "/alarme_on")
    {
      
      alarmeStatut = true;
      bot.sendMessage(chat_id, "Alarme en marche", "");
    }

    if (text == "/alarme_off")
    {
      alarmeStatut = false;
      
      bot.sendMessage(chat_id, "Alarme arrêtée", "");
    }

    if (text == "/etat_alarme")
    {
      if (alarmeStatut)
      {
        bot.sendMessage(chat_id, "Alarme en marche", "");
      }
      else
      {
        bot.sendMessage(chat_id, "Alarme arrêtée", "");
      }
    }

    if (text == "/start")
    {
      String welcome = "Alarme maison " + from_name + ".\n\n";
      welcome += "/alarme_on \n\n";
      welcome += "/alarme_off \n\n";
      welcome += "/etat_alarme \n";
      bot.sendMessage(chat_id, welcome, "");
    }
  }
}

//telegram fin


//ETH debut
// WARNING: onEvent is called from a separate FreeRTOS task (thread)!
void onEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_ETH_START:
      Serial.println("ETH Started");
      // The hostname must be set after the interface is started, but needs
      // to be set before DHCP, so set it from the event handler thread.
      ETH.setHostname("esp32-ethernet");
      break;
    case ARDUINO_EVENT_ETH_CONNECTED: Serial.println("ETH Connected"); break;
    case ARDUINO_EVENT_ETH_GOT_IP:
      Serial.println("ETH Got IP");
      Serial.println(ETH);
      eth_connected = true;
      break;
    case ARDUINO_EVENT_ETH_LOST_IP:
      Serial.println("ETH Lost IP");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
      Serial.println("ETH Disconnected");
      eth_connected = false;
      break;
    case ARDUINO_EVENT_ETH_STOP:
      Serial.println("ETH Stopped");
      eth_connected = false;
      break;
    default: break;
  }
}

void testClient(const char *host, uint16_t port) {
  Serial.print("\nconnecting to ");
  Serial.println(host);

  NetworkClient client;
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }
  client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
  while (client.connected() && !client.available());
  while (client.available()) {
    Serial.write(client.read());
  }

  Serial.println("closing connection\n");
  client.stop();
}

//ETH Fin

void setup() {
  pinMode(PIN_PIR, INPUT);
  Serial.begin(115200);
  //ETH debut
  Network.onEvent(onEvent);  
  ETH.begin();
  //ETH fin
  
  //telegram debut

  delay(10);
  
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org

  //configuration france & heure d'été   https://sites.google.com/a/usapiens.com/opnode/time-zones  
	configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", ntpServer);
//telegram fin
 
//Fin gestion détecteur mouvement
}

void loop() {
  if (digitalRead(PIN_PIR) && verrouDetection == false && alarmeStatut){
      Serial.println("Intrusion!!!");
      bot.sendMessage(chat_id, "Intrusion!!!", "");
      verrouDetection = true;
       }
  

  if (eth_connected) {
    if (millis() - bot_lasttime > BOT_MTBS)
  {
    
    if (digitalRead(PIN_PIR)== 0 && verrouDetection ) {
    verrouDetection = false;
  }    
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages)
    {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }

    bot_lasttime = millis();
   
  }
  
  
  }
 
}
