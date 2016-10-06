/* Programme un réveil connecté à internet
*/
#include <Time.h>
#include <ESP8266WiFi.h> //wifi library
//include the library of LiquidCrystal I2C
#include <Wire.h> 
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

/*
 Now we need a LedControl to work with.
 ***** These pin numbers will probably not work with your hardware *****
 pin D1 is connected to the SCL
 pin D2 is connected to the SDA
 */
LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

// DURATION OF THE NOTES 
#define BPM 120    //  you can change this value changing all the others
#define H 2*Q //half 2/4
#define Q 60000/BPM //quarter 1/4 
#define E Q/2   //eighth 1/8
#define S Q/4 // sixteenth 1/16
#define W 4*Q // whole 4/4

#define cB 130
#define dB 146
#define eBb 155
#define fB 174
#define gB 196
#define aBb 207
#define bBb 233
#define c 261
#define d 294
#define eb 311
#define e 329
#define f 349
#define g 391
#define gS 415
#define ab 416
#define a 440
#define aS 455
#define bb 456
#define b 466
#define cH 523
#define cSH 554
#define dH 587
#define dSH 622
#define ebH 623
#define eH 659
#define fH 698
#define fSH 740
#define gH 784
#define gSH 830
#define aH 880
#define silence 1

const int buzz = D7; //buzzer connected to pin D7
const int buzz2 = D8; //2nd buzzer
int duree = 0;
int note;
int note2;
int alarmH = 7; //Heure paramétrée de l'alarme
int alarmM = 15; //Minute paramétrée de l'alarme
int alarmhWe = 14; // Heure d'alarme de we
int alarmmWe = 41; //Minute d'alarme de we
int h=0;
int m=0;
int i=0;
char ch=0;

boolean weekend = false; //test booléen du we

// valeurs pour le WiFi

//const char* ssid     = "SFR_0790";
//const char* password = "diegpevbibapbuvriag4";

const char* ssid     = "FREEBOX-FRANCOIS-FL";
const char* password = "erint2&-issimum9-natore&-rimus2";
// valeurs pour le serveur Web
const char* host     = "api.timezonedb.com";
const char* apikey   = "JR4MY3P0ABUK";
const char* zone     = "Europe/Paris"; 

String heure = String("\"timestamp\":"); //chaîne que l'on recherche dans le JSON

// déclaration d'une variable qui stockera l'heure du réveil
int temps = 0 ;

// drapeau indiquant pendant l'analyse de la réponse du serveur
// si on est dans l'en-tête HTTP (false) ou dans le contenu de 
// la ressource.
bool inBody = false;

void setup() {
  pinMode(buzz, OUTPUT); //Pin du buzz défini en sortie
   // initialisation du port série
  Serial.begin(9600);
  lcd.init(); //initialize the lcd
  lcd.backlight();
  lcd.setCursor(2,0);
  lcd.print("Hello, Marie!");
  //}
  /*
  // on affiche le wifi sur lequel on veut se connecter
  Serial.print("Connexion au WiFi ");
  Serial.println(ssid);*/
  
  WiFi.begin(ssid, password);  // on se connecte
  
  while (WiFi.status() != WL_CONNECTED) { // on attend d'être connecté
    delay(500);
    Serial.print(".");
     }

  Serial.println("");  // on affiche les paramètres 
  Serial.println("WiFi connecte");  
  Serial.print("Adresse IP du module ESP: ");  
  Serial.println(WiFi.localIP());
  Serial.print("Adresse IP de la box : ");
  Serial.println(WiFi.gatewayIP());
}

void loop() {

  WiFiClient client;
  const int httpPort = 80;

  Serial.print("connexion au serveur : ");
  Serial.println(host);
  
  // On se place dans le rôle du client en utilisant WifiClient
  //WiFiClient client;

  // le serveur Web attend traditionnellement sur le port 80

  // Si la connexion échoue cela sera pour la prochaine fois
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // La connexion a réussi on forme le chemin 
  // URL complexe composée du chemin et de deux 
  // questions contenant le nom de la zone et l'API key
  
  String url = String("/v2/get-time-zone?key=") + apikey + "&format=json&by=zone&zone=" + zone;
  Serial.print("demande URL: ");
  Serial.println(url);
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");

  // On attend 1 seconde
  delay(1000);
  
  inBody = false; // on est dans l'en-tête
  
  // On lit les données reçues, s'il y en a
  while(client.available()){
    String line = client.readStringUntil('\r');
        
    if (line.length() == 1) inBody = true; /* passer l'en-tête jusqu'à une ligne vide */
    if (inBody) {  // ligne du corps du message, on cherche le mot clé
       int pos = line.indexOf(heure);

      if (pos > 0) { /* mot clé trouvé */
       // indexOf donne la position du début du mot-clé, en ajoutant sa longueur
       // on se place à la fin.
       pos += heure.length(); 

       Serial.println (&line[pos]);

       temps = atof(&line[pos]); 

     } /* fin récupération du flottant */
    } /* fin de la recherche du mot clé */
  } /* fin data available */

  //affichage de la date et de l'heure
  Serial.println();
  Serial.print ("Jour = ");
  int jour = day(temps);
  Serial.print(jour);
  Serial.print ("/");
  int mois = month(temps);
  Serial.print(mois);
  Serial.print ("/");
  int annee = year(temps);
  Serial.println(annee);
  Serial.print("Jour de la semaine : ");
  Serial.print(weekday(temps));
  weekEnd(); //vérification du test booléen week end
  if (weekend == true) {
    Serial.println(" weekend"); 
    } else {
      Serial.println(" weekday");
  }
  Serial.print ("Heure = ");
  int h = hour(temps);
  Serial.print(h);
  int m = minute(temps);
  Serial.print(":");
  Serial.print(m);
  int s = second(temps);
  Serial.print(":");
  Serial.println(s);

  lcd.setCursor(6,1);
  if(h<10) {
    lcd.print("0");
    lcd.print(h);
  }
    else {
      lcd.print(h);
    }
    lcd.print(":");
    if(m<10) {
      lcd.print("0");
      lcd.print(m);
    }
    else {
      lcd.print(m);
    }

  Serial.println("connexion fermee");
  //on lance la musique si le test alarm est réussi
    if(weekend == false && h == alarmH && m == alarmM) {
     onEcrit();
      Serial.println("c'est parti");
      delay(500);
    } else {
      if (weekend == true && h == alarmhWe && m == alarmmWe) {
      Serial.println("c'est le we, on dort");
      delay(500);
      }
    }

  //affichage de l'heure
//  LedDisplayTime();
}

void playTone(int note, int duree) {
  analogWriteFreq(note);
  analogWrite(buzz, 512); //512
  delay(duree);
}

void doublePlayTone(int note, int duree) {
  analogWriteFreq(note);
  analogWrite(buzz, 512); //512
  analogWrite(buzz2, 512);
  delay(duree);
  analogWrite(buzz, 0);
  analogWrite(buzz2, 0);
}

void playDoubleTone(int note, int note2, int duree) {
  analogWriteFreq(note);
  analogWrite(buzz, 512);
  delay(duree); // test
  analogWriteFreq(note2);
  analogWrite(buzz2, 512);
  delay(duree);
  analogWrite(buzz, 0);
  analogWrite(buzz2, 0);
}


void playNoTone(int note, int duree) {
  analogWriteFreq(note);
  analogWrite(buzz, 0); //0 pour couper le son
  delay(duree);
}

void onEcrit() {
playNoTone(silence,H);  //1
playNoTone(silence,Q);  //
playTone(c,E);  //
playTone(d,E);  //
doublePlayTone(eb,Q);  //2
playTone(d,E);  //
playTone(c,E);  //
doublePlayTone(d,E);  //
playTone(eb,E);  //
playTone(f,E);  //
playTone(d,E);  //
doublePlayTone(eb,E);  //3
playTone(d,E);  //
playTone(c,Q);  //
playNoTone(silence,Q);  //
playTone(eb,E);  //
playTone(f,E);  //
doublePlayTone(g,Q);  //4
playTone(f,E);  //
playTone(eb,E);  //
doublePlayTone(f,E);  //
playTone(g,E);  //
playTone(ab,E);  //
playTone(g,E);  //
doublePlayTone(g,H);  //5
playNoTone(silence,Q);  //
playTone(ab,E);  //
playTone(bb,E);  //
doublePlayTone(cH,Q);  //6
playTone(bb,E);  //
playTone(ab,E);  //
doublePlayTone(g,E);  //
playTone(f,E);  //
playTone(g,E);  //
playTone(ab,E);  //
doublePlayTone(bb,Q);  //7
playTone(eb,E);  //
playTone(d,E);  //
doublePlayTone(c,Q);  //
playTone(c,E);  //
playTone(d,E);  //
doublePlayTone(eb,Q);  //8
playTone(d,E);  //
playTone(c,E);  //
doublePlayTone(f,E);  //
playTone(eb,E);  //
playTone(d,Q);  //
doublePlayTone(c,H);  //9
playNoTone(silence,H);  //
doublePlayTone(cH,Q+E);  //10
playTone(bb,E);  //
doublePlayTone(bb,Q);  //
playNoTone(silence,E);  //
playTone(f,E);  //
doublePlayTone(f,E);  //11
playTone(eb,E);  //
doublePlayTone(g,H);  //
playNoTone(silence,E);  //
playTone(eb,S);  //
playTone(f,S);  //
doublePlayTone(g,E);  //12
playTone(ab,E);  //
playTone(g,E);  //
playTone(f,E);  //
doublePlayTone(f,Q);  //
playNoTone(silence,E);  //
playTone(eb,E);  //
doublePlayTone(f,E);  //13
playTone(c,E);  //
playTone(c,Q);  //
doublePlayTone(c,Q);  //
playNoTone(silence,E);  //
playTone(c,E);  //
doublePlayTone(cH,E);  //14
playTone(cH,E);  //
playTone(cH,E);  //
playTone(bb,E);  //
doublePlayTone(bb,Q);  //
playNoTone(silence,E);  //
playTone(f,E);  //
doublePlayTone(f,Q);  //15
playTone(eb,E);  //
playTone(g,E);  //
doublePlayTone(g,E);  //
playTone(eb,E);  //
playTone(f,E);  //
playTone(c,E);  //
doublePlayTone(c,H);  //16
playNoTone(silence,H);  //
playTone(eBb,Q); //
playTone(eb,E);  //
playTone(f,E);  //
playTone(c,E);  //
doublePlayTone(c,H);  //18
playNoTone(silence,H);  //
playNoTone(silence,H+E);  //19
playTone(c,E);  //
playTone(d,E);  //
doublePlayTone(eb,Q);  //20
playTone(d,E);  //
playTone(c,E);  //
doublePlayTone(d,E);  //
playTone(eb,E);  //
playTone(f,E);  //
playTone(d,E);  //
doublePlayTone(eb,E);  //21
playTone(d,E);  //
playTone(c,Q);  //
playNoTone(silence,Q);  //
playTone(eb,E);  //
playTone(f,E);  //
doublePlayTone(g,Q);  //22
playTone(f,E);  //
playTone(eb,E);  //
doublePlayTone(f,E);  //
playTone(g,E);  //
playTone(ab,E);  //
playTone(g,E);  //
doublePlayTone(g,H);  //23
playNoTone(silence,Q);  //
playTone(ab,E);  //
playTone(bb,E);  //
doublePlayTone(cH,Q);  //24
playTone(bb,E);  //
playTone(ab,E);  //
doublePlayTone(g,E);  //
playTone(f,E);  //
playTone(g,E);  //
playTone(ab,E);  //
doublePlayTone(eb,Q);  //25
playTone(eb,E);  //
playTone(d,E);  //
doublePlayTone(c,Q);  //
playTone(c,E);  //
playTone(d,E);  //
doublePlayTone(eb,Q);  //26
playTone(d,E);  //
playTone(c,E);  //
doublePlayTone(f,E);  //
playTone(eb,E);  //
playTone(d,Q);  //
doublePlayTone(c,H);  //27
playNoTone(silence,Q);  //
playTone(c,E);  //
playTone(d,E);  //
playTone(eb,Q);  //28
playTone(d,E);  //
playTone(c,E);  //
doublePlayTone(d,E);  //
playTone(eb,E);  //
playTone(f,E);  //
playTone(d,E);  //
doublePlayTone(eb,E);  //29
playTone(d,E);  //
playTone(c,Q);  //
playNoTone(silence,Q);  //
playTone(eb,E);  //
playTone(f,E);  //
doublePlayTone(g,Q);  //30
playTone(f,E);  //
playTone(eb,E);  //
doublePlayTone(f,E);  //
playTone(g,E);  //
playTone(ab,E);  //
playTone(g,E);  //
doublePlayTone(g,H);  //31
playNoTone(silence,Q);  //
playTone(ab,E);  //
playTone(bb,E);  //
doublePlayTone(cH,Q);  //32
playTone(bb,E);  //
playTone(ab,E);  //
doublePlayTone(g,E);  //
playTone(f,E);  //
playTone(g,E);  //
playTone(ab,E);  //
doublePlayTone(bb,Q);  //33
playTone(f,E);  //
playTone(d,E);  //
doublePlayTone(c,Q);  //
playTone(c,E);  //
playTone(d,E);  //
doublePlayTone(eb,Q);  //34
playTone(d,E);  //
playTone(c,E);  //
doublePlayTone(f,E);  //
playTone(eb,E);  //
playTone(d,Q);  //
doublePlayTone(c,H);  //35
playNoTone(silence,H);  //
}


void weekEnd() {
  if(weekday(temps) == 1 || weekday(temps) == 7) {
    weekend = true;
  }
  else {
    weekend = false;
  }
}




