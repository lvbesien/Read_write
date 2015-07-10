#include <Adafruit_PN532.h>

#define SCK  (2)
#define MOSI (3)
#define SS   (4)
#define MISO (5)

char BUFFER[64]= {""};
char date[6] = {""};
int i = 0;
int VacSt = 16;

int bufpos = 0;
const char END = ('*');
char TRASH[16];

uint8_t first[16] = {""};
uint8_t second[16] = {""};
uint8_t third[16] = {""};


Adafruit_PN532 nfc(SCK, MISO, MOSI, SS);

void setup() {
  Serial.begin(115200);
  Serial.println("Hello!");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); 
  Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); 
  Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); 
  Serial.println((versiondata>>8) & 0xFF, DEC);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ..."); 
}


void loop() {
  
/*   uint8_t success;
   uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  
   uint8_t uidLength;                        
   
   //Wait for card and display Basic Info
   success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
 
   if (success) {
   Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
   Serial.print("  UID Value: ");
   nfc.PrintHex(uid, uidLength);
   Serial.println("");
   }
   */
   Welcome: 
   Serial.println ("Type 1 to Read Bracelet, 2 to Write ID info, 3 to Add Vaccine Records, and 4 to Clear Bracelet");
   Wait();
   int a = Serial.read ();
   switch (a) {
     case '1':{
       ReadBracelet ();
       break;
     }
     case '2': {
       Serial.println ("Name?");
       Wait();
       WriteInput (1);
       break;
     }
     case '3': {
       AddVaccines ();
       //ReadBracelet ();
       break;
     }
     case '4': {
       Serial.println ("Type 1 to Clear Entire Bracelet, 2 to Clear Individual Sector");
       Wait();
       int y = Serial.read ();
       switch (y) {
         case '1':{
           ClearBracelet (1,15);
          // ReadBracelet();
           break;
         }
         case '2': {
           Serial.println ("Which sector? (HEX)");
           Wait();
           int s = Serial.read ();
           ClearBracelet (s,s);
           break;
         }
       }
     }
   }

Serial.println ("Continue with same card? Type Y/N");
Wait();
int cont = Serial.read();
if (cont == 'N'){
     char newcard[16];
     Serial.println ("Place New Card and type OK");
     Wait();
     while (Serial.available () > 0) {
     Serial.readBytes (newcard, 2);
     } 
  }


} 

void Wait () {
  while (Serial.available ()==0) {
    delay(50);
  }
}

void PrintArray (const char *msg) {
  Serial.print("Array: ");
  Serial.println(msg);
}

int Authenticate (int sector) {
  int auth;
  uint8_t uid[] = { 
    0, 0, 0, 0, 0, 0, 0   };  
  uint8_t uidLength;  
  uint8_t keya[6] = { 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF   };  

  auth = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  auth = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, sector * 4, 0, keya);
  if (! auth) {
    Serial.print ("Failed Authentication of Sector "); 
    Serial.println (sector);
    return 0;
    Wait ();
  }
  else {
   // Serial.print ("Authenticated Sector "); 
   // Serial.println (sector);
    return 1;
  }
}

//Store 48 characters in 3 arrays


//Read 48 characters for WriteInput
//Serial.read as you go on....end loop if Serial.available() = 0,
void Clear (){
  int p = 0;
 for (p = 0; p < 64; p++) {
  BUFFER[p] = '*';
 }
 p=0;
}

void StoreInput () {
  Clear (); 
  char ch;
  int length = Serial.available();
  while (Serial.available () > 0) {
    for (i =0; i < length; i++){
      ch = Serial.read ();
       BUFFER[i] = ch;
      //Serial.print (ch);
    }
  }
    if (length > 62 ) {
      Serial.println ("Message cut off after 63 characters");
    }
  
  PrintArray (BUFFER); 
}

int WriteInput (int sector) { //For storing serial input in one sector. Message limited to 48 characters.
  uint8_t first[16] = {""};
  uint8_t second[16] = {""};
  uint8_t third[16] = {""};
  i = 0;
  
            Wait();
            while (Serial.available ()) {
              if (i > 47){
                Serial.println ("Message cut off after 48 characters");
                Serial.readBytes (TRASH, 16);
                break;
              }
              if (i < 16){
                first[i]= Serial.read ();
                i++;
              }
            
              else if (i < 32){
                second[(i-16)] = Serial.read();
                i++;
              }
              else if (i<48){
                third[(i-32)] = Serial.read();
                i++;
              } 
            }
    Authenticate (sector);
    WriteSector(sector, first, second, third);
    ReadSector (sector);
}



int WriteBuffer (int sector, int space) {
  int lastsector = sector;
  for (lastsector = sector; lastsector < sector + space; lastsector++) {
    //Make this separate function? StoreInput, returns 3 arrays)

    uint8_t first[16] = {""};
    uint8_t second[16] = {""};
    uint8_t third[16] = {""};
    i = 0;
    int diff = lastsector - sector;       

    while ( i < 48) {  
      if (BUFFER[i + diff*48] == '*') {
        //Serial.println (i);
        //Serial.println (lastsector);
        Serial.println ("Reached End");
        break;
      }
      else {
        if (i < 16){
          first[i]= BUFFER[i + diff*48];
         Serial.write(first[i]);
        }

        else if (i < 32){
          second[(i-16)] = BUFFER[i + diff*48];
        }
        else if (i<48){
          third[(i-32)] = BUFFER[i + diff*48];
        }                  
        i++;
      }
    }

    Authenticate (lastsector);
    WriteSector(lastsector, first, second, third);
    ReadSector (lastsector);
    if (BUFFER[i + diff*48] == '*') {
        Serial.println (" End");
        break;
      }  
  }
}

int WriteSector(int sector, uint8_t first[16], uint8_t second[16], uint8_t third[16]) {
  int startblock = sector * 4;
  int written;

  written = nfc.mifareclassic_WriteDataBlock (startblock, first) & nfc.mifareclassic_WriteDataBlock (startblock + 1, second) & nfc.mifareclassic_WriteDataBlock (startblock + 2, third); 
  if (written) {
    return 1;
  }
  else {
    Serial.println ("Could not write");
    return 0;
  }
}

void ReadSector (int sector) {
  int startblock = sector * 4;
  uint8_t success;
  uint8_t first[16] = {
    ""  };
  uint8_t second[16] = {
    ""  };
  uint8_t third[16] = {
    ""  };
  // Try to read what you wrote
  success = nfc.mifareclassic_ReadDataBlock(startblock, first) & nfc.mifareclassic_ReadDataBlock(startblock + 1, second) & nfc.mifareclassic_ReadDataBlock(startblock + 2, third);

  if (success){
    if (first[0] == 0 & second[0] == 0 & third[0] == 0) {
     // Serial.print ("Nothing written on Sector "); Serial.println (sector);
    }
    
    else {    
    Serial.print("Reading Sector "); 
    Serial.println (sector);
    nfc.PrintHexChar(first, 16);
    nfc.PrintHexChar(second, 16);
    nfc.PrintHexChar(third, 16);
    //wait a sec
    delay(1000);
    }
  }

  else {
    Serial.println("Unable to read sector. Try another key?");
  }
}

/*void ClearSector (int sector){
  char EMPTY[48] = ("");
}
*/
  

void ContinueWriting (int startsector, int space){ //continue writing input in sector
  
  int sector;
  
  
  for (sector = startsector; sector <= space; sector++) {
    int blockone = sector*4;
    
  Authenticate (sector);
  nfc.mifareclassic_ReadDataBlock(blockone, first);
  nfc.mifareclassic_ReadDataBlock(blockone + 1, second);
  nfc.mifareclassic_ReadDataBlock(blockone + 2, third);
  
  //nfc.PrintHexChar(first, 16);
  
  
  
  i = 0;
  while (Serial.available () > 0) {

    if (i < 16){
      if ((first[i]) != 0){
        Serial.print (i); Serial.print("is filled "); Serial.println (first[i], HEX);
        i++;
      }
      else {
      first[i] = Serial.read ();
      i++;
      Serial.println (i);
      }
    }
    if (i >= 16){
      break;
    }
  }
  nfc.PrintHexChar(first, 16);
}
}

void ContinueBlock (int block){ //write a buffer starting where it's empty
  i = 0;
  int sector = int (block/4);
  Serial.print ("Sector : "); Serial.println (sector);
  Authenticate (sector);
  PrintArray (BUFFER);
  nfc.mifareclassic_ReadDataBlock(block, first);
  
  while (i < 16) {
  if ((first[i]) != 0){
       // Serial.print (i); Serial.print("is filled "); Serial.println (first[i], HEX);
        i++;
      }
  else {
    i++;
    first[i] = BUFFER[bufpos];
    i++;
    bufpos++;
  }
  }
  nfc.mifareclassic_WriteDataBlock (block, first);
  nfc.PrintHexChar (first, 16);
}
     
  
void StoreDate (){
  Serial.println ("Type the date (DDMMYY)");
  Wait ();
  for (i = 0; i < 6; i++) {
    date [i] = Serial.read ();
  }
  if (Serial.available ()){
    Serial.readBytes (TRASH, 16);
  }
  Serial.println (date);
 // Clear();
 // Serial.println (BUFFER);
 // Serial.print ("after 63 : "); Serial.println (BUFFER[80]);
}


void VaccineBuffer (){
  Clear ();
  Serial.println ("What vaccine are you using? (Type 1 character)");
  Wait ();
  int v = Serial.read ();
  BUFFER[0] = v;
  BUFFER[1] = (' ');
  for (i = 2; i < 8; i++) {
    BUFFER[i] = date[i-2];
  }
}
    
int CheckBlock (int block){// this function doesn't authenticate. MUST AUTHENTICATE BEFORE READING 
  nfc.mifareclassic_ReadDataBlock(block, first);
  if (first[0] == 0) {
    return 1;
  }
  else {return 0;};
}

void WriteEmptyBlock (){
  //Serial.println ("WriteEmptyBlock function");
  uint8_t data[16] = ("");
  int block = VacSt;
  Authenticate (block/4);
  for (block = VacSt; block < 63; block++){
    if (block % 4 == 3) {
      block++;
      Authenticate ((block)/4);
    }
    
    int empty = CheckBlock (block);
      if (empty){
        for (i=0; i<16 & data[i] != '*'; i++){
          data[i] = BUFFER [i];
          nfc.mifareclassic_WriteDataBlock(block, data);
        }
          Serial.print ("New Data on Block " ); Serial.println (block);
          nfc.PrintHexChar (data, 16);
          break;
        
      }
      //else {Serial.print (block); Serial.println (" is filled");};
  }
}

void AddVaccines (){
  int block = VacSt;
  while (block < 64){
    Serial.println ("Add new vaccine info? Type Y/N");
    Answer: 
    Wait ();  
    char y = Serial.read ();
      if (y == 'Y'){
        StoreDate();
        VaccineBuffer();
        WriteEmptyBlock();
        block++;
      }
      else if (y == 'N') {
        Serial.println ("OK, No new vaccines");
        break;
      }
      else {
        Serial.println ("Command not Recognized. Try Again");
        goto Answer;
      }
}
}

void ClearBracelet(int start,int finish){
  int sector;
  uint8_t first[16] = {""};
  uint8_t second[16] = {""};
  uint8_t third[16] = {""};
  for (sector = start; sector <= finish; sector++) {
    Authenticate (sector);
    WriteSector(sector, first, second, third);
    ReadSector (sector);
  }
}

void ReadBracelet(){
  int sector;
  Serial.println ("Reading Whole Card");
  for (sector = 1; sector < 16; sector++){
    Authenticate(sector);
    ReadSector(sector);
  }
}
  

/*Needed functions

ReadCard (parts that are written)
ClearCard
VaccineBuffer
WriteVaccine

 */ 


