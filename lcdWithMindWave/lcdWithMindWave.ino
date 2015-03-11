
// This is example code provided by NeuroSky, Inc. and is provided
// license free.
////////////////////////////////////////////////////////////////////////

#include <LiquidCrystal.h>
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);


#define LED 13
#define BAUDRATE 57600
#define DEBUGOUTPUT 1
// define some values used by the panel and buttons
int attentionThreshold = 0;
int meditationThreshold = 0;
int threshold = 0;
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5
int i=0;

// checksum variables
byte generatedChecksum = 0;
byte checksum = 0; 
int payloadLength = 0;
byte payloadData[64] = {
  0};
byte poorQuality = 0;
byte attention = 0;
byte meditation = 0;

// system variables
long lastReceivedPacket = 0;
boolean bigPacket = false;

//////////////////////////
// Microprocessor Setup //
//////////////////////////
void setup() {

  pinMode(LED, OUTPUT);
  lcd.begin(16, 2); // start the library
  lcd.setCursor(0,0);
  Serial.begin(BAUDRATE);           // USB
  lcd.print("Choose your type");
  //lcd.clear();


}

////////////////////////////////
// Read data from Serial UART //
////////////////////////////////





byte ReadOneByte() {
  int ByteRead;

  while(!Serial.available());
  ByteRead = Serial.read();

#if !DEBUGOUTPUT  
  Serial.print((char)ByteRead);   // echo the same byte out the USB serial (for debug purposes)
#endif

  return ByteRead;
}






////////////////////////////////
// Read pushed buttons        //
////////////////////////////////






int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 // my [Mark Bramwell's] buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 195)  return btnUP; 
 if (adc_key_in < 380)  return btnDOWN; 
 if (adc_key_in < 555)  return btnLEFT; 
 if (adc_key_in < 790)  return btnSELECT;   
 return btnNONE;  // when all others fail, return this...
}





////////////////////////////////
// Output data on lcd        //
////////////////////////////////



void printElement(int i)
{
  Serial.print(i);
  lcd.clear();
}



////////////////////////////////
// Switches by buttons        //
////////////////////////////////


int selectThreshold(const char* type,int& param)
{
  
                   lcd.clear();
                   lcd.setCursor(0,0);
                   lcd.print("Select % level  ");
  delay(1000);
  boolean flag = true;
  while(flag)
  {
            
            switch(read_LCD_buttons())
            {
                 case btnSELECT:
                 {
                   lcd.clear();
                   lcd.setCursor(0,0);
                   lcd.print("Select % level  ");
                   if(param != 0) lcd.clear();threshold=param;flag=false;
                   break;
                 }
                 case btnUP:
                 {
                   delay(400);
                   lcd.setCursor(0,1);
                   Serial.println("UP");
                    param+=10;
                   (param > 100) ? param=100:param=param;
                   lcd.print(param);
                   break;
                 }
                 case btnDOWN:
                 {
                   delay(400);
                   lcd.setCursor(0,1);
                   param-=10;
                   (param < 0) ? param=0:param=param;
                   lcd.print(param);
                   break;
                 }
            }
   }
}

int enableType(const char* type,int& param)
{
    lcd.print(type);
     while(read_LCD_buttons() == btnNONE)
     {
       
       switch(read_LCD_buttons())
       {
         case btnSELECT:selectThreshold(type,param);break;
         case btnDOWN  : return 0;break;
       }
       
       
     }
     

}

int switchType()
{
 //USE MenuBackend after this bullshit
 //analogWrite(backLight, lightLevel);
 lcd.setCursor(0,0);            // move to position 13 on the second line
 lcd.setCursor(0,1);            // move to the begining of the second line
 lcd_key = read_LCD_buttons();  // read the buttons

 switch (lcd_key)               // depending on which button was pushed, we perform an action
 {

   case btnUP:
     {
       
     delay(500);
     enableType("Attention",attentionThreshold);
  
     break;
     }
   case btnDOWN:
     {
     delay(500);
     enableType("Meditation",meditationThreshold);
     
     break;
     }

   case btnNONE:
     {
     lcd.print("                ");
     break;
     }
 }
 return 0;
}





/////////////
//MAIN LOOP//
/////////////
void loop() 
{
  if(threshold == 0)
    switchType();
  if(threshold != 0)
  {
  
  // Look for sync bytes
  if(ReadOneByte() == 170) {
    if(ReadOneByte() == 170) {

      payloadLength = ReadOneByte();
      if(payloadLength > 169)                      //Payload length can not be greater than 169
          return;

      generatedChecksum = 0;        
      for(int i = 0; i < payloadLength; i++) {  
        payloadData[i] = ReadOneByte();            //Read payload into memory
        generatedChecksum += payloadData[i];
      }   

      checksum = ReadOneByte();                      //Read checksum byte from stream      
      generatedChecksum = 255 - generatedChecksum;   //Take one's compliment of generated checksum

        if(checksum == generatedChecksum) {    

        poorQuality = 200;
        attention = 0;
        meditation = 0;

        for(int i = 0; i < payloadLength; i++) {    // Parse the payload
          switch (payloadData[i]) {
          case 2:
            i++;            
            poorQuality = payloadData[i];
            bigPacket = true;            
            break;
          case 4:
            i++;
            attention = payloadData[i];                        
            break;
          case 5:
            i++;
            meditation = payloadData[i];
            break;
          case 0x80:
            i = i + 3;
            break;
          case 0x83:
            i = i + 25;      
            break;
          default:
            break;
          } // switch
        } // for loop

#if DEBUGOUTPUT

        // *** Add your code here ***

        if(bigPacket) {                             
          if(poorQuality == 0)
            digitalWrite(LED, HIGH);
          else
            digitalWrite(LED, LOW);
          Serial.print("PoorQuality: ");
          Serial.print(poorQuality, DEC);
          Serial.print(" Attention: ");
          Serial.print(attention, DEC);
          Serial.print(" Meditation: ");
          Serial.print(meditation,DEC);
          Serial.print(" Time since last packet: ");
          Serial.print(millis() - lastReceivedPacket, DEC);
          lastReceivedPacket = millis();
          Serial.print("\n");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Your level");
          lcd.setCursor(0,1);
          if(attentionThreshold !=0)
            lcd.print(attention,DEC);
          if(meditationThreshold !=0)
            lcd.print(meditation,DEC);
          Serial.print(attentionThreshold);
          Serial.println(meditationThreshold);
        } 
#endif        
      bigPacket = false;        
    }
    else {
      // Checksum Error
      }  // end if else for checksum
    } // end if read 0xAA byte
  } // end if read 0xAA byte

  }
}


