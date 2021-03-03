// Please visit https://github.com/sevolord/LanThermometer
// pin 2 - DHT
// pin 3 - LedPower Dust Sensor
// pin A0 - distPin
// pins A4, A5  - SCL, SDA lcd
// Ethernet shield attached to pins 10, 11, 12, 13

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};                                  // можно сгенерировать индивидуальный
IPAddress ip(192, 168, 1, 16);     
//IPAddress myDns(192, 168, 1, 1);
//IPAddress gateway(192, 168, 1, 1);
//IPAddress subnet(255, 255, 0, 0);
EthernetServer server(80);          // telnet defaults to port 80

#include "DHT.h" 

DHT dht(2, DHT22);                  // pin, type dht


#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); //  0x3f - альтернативный адресс


boolean newInfo = 0;                //переменная для новой информации


void setup()
{
  Ethernet.begin(mac, ip);  // (mac, ip, myDns, gateway, subnet)
  server.begin();           // initialize the server
  dht.begin();              // initialize the temperature sensor
  lcd.init();               // initialize the lcd
  lcd.backlight();          // включаем подсветку
  pinMode(3, OUTPUT);       //ledPower DustSensor
  pinMode(A0, INPUT);       // dust pin
}

void loop()
{
  EthernetClient client = server.available(); // если кто-то подключился 
  if (client) {
    Serial.println("new client");
    boolean currentLineIsBlank = true;
    while (client.connected()) {              // пока клиент подключен
      if (client.available()) {
        char c = client.read();               //читаем входящие данные
        Serial.write(c);

        if (c == '\n' && currentLineIsBlank) {
          float h = dht.readHumidity(); // влажность
          float t = dht.readTemperature(); // температура
          float d = readDustSensor(); // пыльность
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          //client.println();
          client.print("{\"h\":\"");
          client.print(h);
          client.print("\", \"t\":\"");
          client.print(t);
          client.println("\"}");
          client.println("Refresh: 5");       // период автообновления страницы 
          client.println();
          client.println("<!DOCTYPE HTML>");  //HTML тип документа
          client.println("<html>");           //открытие тега HTML
          client.println("<head> ");          //открытие тега Head
          client.println("<meta http-equiv='Content-Type' content='text/html ; charset=utf-8'/> ");
          client.print("<title>Server room thermometer</title>"); //название страницы
          client.println("</head>");          //заголовочная информация
          client.println("<body>");
          client.print("<H1>Server room thermometer and dust sensor ver 1.2</H1>"); //заголовк на странице
          client.println("<br>");             //перенос на след. строчку
          client.println("<hr/>");            //линия=====================================
          client.print("Temperature: ");
          client.println(t);
          client.print(" *C: ");
          client.println("<br>");
          client.print("Humidity: ");
          client.println(h);
          client.println();
          client.println("<br>");
          client.print("Dust level: ");
          client.println(d);
          client.println("<br>");
          client.println ("<hr/>"); 
          client.println("<br>");
          client.println("Data compared to air quality:");
          client.println("<br>");
          client.println("2500 + = Very Bad ");
          client.println("<br>");
          client.println("1050-2500 = Bad ");
          client.println("<br>");
          client.println("500-1050 = Ordinary ");
          client.println("<br>");
          client.println("300-500 = Good ");
          client.println("<br>");
          client.println("<300 = Very Good ");
          client.println("<br>");
          client.println("<br>");
          client.print ("<a href=https://wiki.keyestudio.com/Ks0196_keyestudio_PM2.5_Shield><button>Dust sensor web page</button></a>"); // ссылка на описание сенсора пыли на сайте производителя 
          client.print("<a href=\"/$1\"><button>Reset Arduino</button></a>"); //кнопка перезагрузки
          client.println("</body>");
          client.println("</html>");
          client.println();
          break;
        }
        if (c == '$') {
          // если переменная "с", несущая отправленный нам запрос, содержит символ $
          // "$" подразумевает разделение получаемой информации (символов)
          newInfo = 1; //то пришла новая информация, ставим метку новой информации в 1
        }
        //Проверяем содержание URL - присутствует $1 или $2
        if (newInfo == 1) { //если есть новая информация
          Serial.println (c);
          if (c == '1') { //и "с" содержит 1
            Serial.println ("Перезагрузка");
            void(* resetFunc) (void) = 0;//объявляем функцию reset с адресом 0
            resetFunc(); //Перезагружаем Arduino
          }
        }
          if (c == '\n') {
            // you're starting a new line
            currentLineIsBlank = true;
          } else if (c != '\r') {
            // you've gotten a character on the current line
            currentLineIsBlank = false;
          }
        }
      }
      delay(1);
      client.stop();
      Serial.println("client disconnected");
    }

    float h = dht.readHumidity(); // влажность
    float t = dht.readTemperature(); // температура
    float d = readDustSensor(); // пыльность
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.setCursor(2, 0);
    lcd.print(t);
    lcd.setCursor(7, 0);
    lcd.print("C");
    lcd.setCursor(9, 0);
    lcd.print("H:");
    lcd.setCursor(11, 0);
    lcd.print(h);
    lcd.setCursor(0, 1);
    lcd.print("Dust:");
    lcd.setCursor(6, 1);
    lcd.print(d);
    delay(3000);

  }


float readDustSensor() {

  digitalWrite(3, LOW);
  delayMicroseconds(280);
  float dustVal = analogRead(A0);
  delayMicroseconds(40);
  digitalWrite(3, HIGH);
  delayMicroseconds(9680);

  delay(1000);
  float d = (((dustVal / 1024) - 0.0356) * 120000 * 0.035);
  return d;
}
