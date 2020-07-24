/*
  Morse Coder ( for the Arduino )

  ╔══════════════════════════════════════════════════════════════════════╗
  ║  Morse Code  Transmitter & Receiver   ( Solo  or  One of a pair )    ║
  ╚══════════════════════════════════════════════════════════════════════╝
  
  The Morse code is transmitted via the LED for detection by the photocell of the
  recipient ( might be the same device - if the LED is shining back into the photocell )
  ( Similar code exists for the Raspberry Pi written in Julia )

  Will convert the Recvd morse ( from it's pair  OR in Loopback )
    and present it in characters. You can then respond by :- 
  In the INPUT section of the monitor :-
     Type in your sentence 'HOW ARE YOU'     which will get converted to Morse Code
     and transmitted at the end of the current TickerTape message 
  OR Press 5 to turn off TickerTape then
     Tap out your morse using the Tapper on the breadboard
     ( If the Tapper has been pressed in the last 10 seconds then the Morse
       receiver goes into Tapper Mode and starts to expect slower DOTS and DASHES
       In Tapper Mode if the last letter of the word FOX is received - it restarts TickerTape
  OR Press 9 to set the LED to permanently ON then
     use your finger or an object to block the LED light getting to the photocell
     thus creating Morse ( in reverse )
  
  Use ONE   space  between Letters
  and TWO   spaces between Words 
  and THREE spaces at the end of a Sentence
  
  Press 0 - To set Reporting Level 0 & Do an AutoTune  ( normal )
        1 - To set Reporting Level 1
        2 - To set Reporting Level 2
        3 - To set Reporting Level 3
        4 - To set Reporting Level 4
        5 - To toggle TickerTape OFF        ( immediately )
        6 - To toggle TickerTape ON and OFF ( after the end of the current message )
        7 - To Halve  TickerTape Speed
        8 - To Double TickerTape Speed
        9 - To toggle the LED being permanently ON

  grmccormack@googlemail.com - July 2020
  
  https://youtu.be/zY6HWqG2hSY
  https://github.com/GregMc/MorseOnArduino
*/

//const int OnBoardLED  = 13 ;
//const int led09       =  9 ;   // Address of the LED ( PWM - if needed )
//const int MorseTapper =  7 ;   // Address of the Dot Dash Pushbutton
//const int TestPB      =  6 ;   // Address of the Test Push Button
//const int Photocell   =  5 ;   // Address of the Photocell ( photocell ) pin

int TransmitPtr01 = 0 , TransmitPtrDD = 0 , SysCounter = 1 , iReportLvl , Read_1 , Read_2 , iTemp , jTemp, kTemp , iDD ;
unsigned int iOwner , Dbounce , TickTape = 1 , TickTapeEOM = 1 , LED_On , iLED ;

unsigned int DOTH_Count = 0 ; //    AutoTuning = DOTH is a candidate for a DOT or a DASH

float fTemp1 , fTemp2 ;

int TT_Period  = 80 ; // Ticker Tape Scan Period
int DotTime  = 10 ; // Photocell values are calculated ( first ten incoming DOTHs )
int DashTime = 20 ;

int rTime1 , rTime2 , rTime3 , rTime4 , rTime5 , rTime6 , rTime7 ;

unsigned long now , prev , RiseEdge , FallEdge ;
int iCount4 , iCount5 , iCount6 ;
float TickerTapeSpeed = 1.0 ; // Halves or Doubles

bool IN , IN_Old , bTemp , bTest_1 , bTest_2 ;

String StrngDD = "ijkl" ;
String MorseDD    = "" ; // "--."
String RecvdStrng = "";   // for Recvd serial data
String sTemp = "                  " , s1Temp = "   " ;

//Test Parameters
String J1String = "" ;
String Detected = "Detected a " ;
String Recvd    = "Received a " ;

String Letters[26] = {
  "A.-",   "B-...", "C-.-.", "D-..",  "E.",   "F..-.", "G--.",  "H....", "I..",
  
  "J.---", "K-.-",  "L.-..", "M--",   "N-.",  "O---",  "P.--.", "Q--.-", "R.-.",

  "S...",  "T-",    "U..-",  "V...-", "W.--", "X-..-", "Y-.--", "Z--.."           };



// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output
  pinMode(9, OUTPUT); // LED Output
  pinMode(7,  INPUT); // Morse Tapper
  pinMode(5,  INPUT); // Photocell
  pinMode(6,  INPUT); // Test Pushbutton
//  pinMode(13, OUTPUT);   OnBoardLED
  Serial.begin(57600); // Baud Rate

  StrngDD = "" ;
  now = millis() + 180 ;   // Give it some chance to get started
}

String GetAlpha(int iAscii)
// Converts ASCII codes into a string
{ if ( iAscii == 32 ) { return " " ; }
  if ((iAscii > 64 ) && (iAscii < 65 + 26 )) { 
    Message(4," iAscii=",iAscii," AlphaCharacter=" + Letters[iAscii-65]) ;
    sTemp = Letters[iAscii-65] ;
    iTemp = sTemp.length() - 1 ;
    sTemp.remove(1,iTemp) ;
    return sTemp ;
  }
  return "" ;
}

String Morse2Alpha(String sMorse)
// Converts an Recvd morse stream     ie. "--." into a single character "G"
{ iTemp = 65 ;
  for ( jTemp = 0; jTemp < 26; jTemp++ )
  { sTemp = Letters[jTemp] ;    s1Temp = sTemp;
    kTemp = sTemp.length()-1 ;
    s1Temp.remove(1,kTemp) ;
    sTemp.remove(0,1) ;
    if ( sMorse.compareTo(sTemp) == 0 ) { return s1Temp ; }
    iTemp++ ;
  }
  if ( sMorse = " ") { return " " ; }
  return sMorse+"? " ;
}

String GetMorseDD(char cChar )  { 
//  if ( cChar == 32 ){ return "0" ; }
  iTemp = char(cChar) ; if (iTemp > 96){iTemp -= (96-64) ;}  // Converting to Uppercase
  if (( iTemp > 64 ) && ( iTemp < 64+27)) { sTemp = Letters[ iTemp - 65] ;}
  sTemp.remove(0,1) ;    // Remove Leading character 'A.-' 
  return sTemp ;
}


void Message(int iReport , String StrngA , int Int1 , String StrngB ) {
  if ( iReportLvl >= iReport ) { Serial.print( StrngA + String(Int1,DEC)+"   " + StrngB ) ; }
}

void Message0(String StrngA) {
  if ( iReportLvl == 0 ) { Serial.print( StrngA ) ; }
}

void MessageSTATUS() {
  if ( iReportLvl == 0 ) {
    Serial.println( "\n\n===   Morse Code Transmitter & Receiver  ( Solo  or  One of a pair )   ===" );
    Serial.println( "  0-4 ReportLvl & 0 is also AutoTune,\n  5-TickTapeOFF immediately,\n  6-TickTapeOFF at end of message OR Restart, \n  7-Slower, 8-Faster, 9-LED ON" ) ;
  }
  Message(0,"TT_Period : " , TT_Period  , "") ;
  Message(0,"DOT  : " , DotTime  , "") ;
  Message(0,"DASH : " , DashTime , "") ;
  Message(0,"ReportLvl : " , iReportLvl , "\n" ) ;
}

void ReadPhotocell() {
  now = millis() ;
  bool Immed = 1 - digitalRead(5); // Photocell
  // ========== Debounce =================================================
  if (Immed != IN ){ Dbounce ++ ; 
    if ( Dbounce > 5 ) { IN = Immed ; Dbounce = 0 ; }
  }
  if (( IN == true ) && ( IN_Old == false )) { // =========== Rising Edge ==========================================
    RiseEdge  = now ;
    if (DOTH_Count > 10 ) {
      float NowMinusFallE = now - FallEdge ; 
      // =========== Dip was considered long enough to be a 'space' thus indicating the completion of a letter =====
      fTemp1 = float(DashTime) ; fTemp2 = fTemp1 / 5.0 ;
//      if ((NowMinusFallE > ( fTemp1 - fTemp2 )) && ( NowMinusFallE < ( fTemp1 + fTemp2 ))) {
      if (NowMinusFallE > ( fTemp1 - fTemp2 )) {
//        Serial.println(StrngDD) ;
        sTemp = Morse2Alpha(StrngDD) ; StrngDD = "" ;
        Message0(sTemp) ;
        Message(2,Detected+" LETTER : " , NowMinusFallE,sTemp) ;
        if ((iOwner != 1) && (sTemp == "X")){ // Detected the user has typed the last letter of the word FOX
          iOwner = 1 ; // Force change of owner back to TickerTape
          TransmitPtr01 = 21 ; // Set the pointer to start JUMPED OVER ...
        }
      }
      // =========== Dip was considered long enough to be a 'SPACE' between words =================================
      fTemp1 = float(DashTime) * 2 ; fTemp2 = fTemp1 / 3.25 ;
      if ((NowMinusFallE > ( fTemp1 - fTemp2 )) && ( NowMinusFallE < ( fTemp1 + fTemp2 ))) {
        sTemp = " " ; 
        Message0(sTemp) ;
        StrngDD = "" ;
        Message(1,Detected+" WORD  : " , NowMinusFallE,sTemp) ;
      }
      // =========== Dip was considered long enough to be the end of a Sentence =================================
      fTemp1 = float(DashTime) * 3 ; fTemp2 = fTemp1 / 4.0 ;
      if ((NowMinusFallE > ( fTemp1 - fTemp2 )) && ( NowMinusFallE < ( fTemp1 + fTemp2 ))) {
        sTemp = "\n" ; 
        Message0(sTemp) ;
        StrngDD = "" ;
        Message(1,"Detected END of SENTENCE : " , NowMinusFallE,sTemp) ;
      }
    }
  }
  if (( IN == false ) &&  IN_Old )  { // ========= FALLING EDGE ==============================================================
    FallEdge  = now ;
    if ( DOTH_Count < 12 ) 
      { // ======================== AutoTune = CALCULATE LENGTHS OF DOTs and DASHes ========= based on incoming stream ===
        rTime1 = now - RiseEdge ;
        //   if ON time is greater than 1 second - then Reset and Abort
        if ( rTime1 > 1000 ){ DOTH_Count = 0 ; return ; }
        if (DOTH_Count < 11 ) {
          if ( DOTH_Count == 0 ) {  
            if (( DotTime > 5 ) && ( DashTime > 10 )) {Serial.print( "\n\nAutoTuning ..." ) ; }// First DOTH Candidate ( either a DOT or a DASH )
            rTime4  = 0 ; rTime5  = 0 ; rTime6  = 0 ;
            iCount4 = 0 ; iCount5 = 0 ; iCount6 = 0 ;
            StrngDD = "" ;
            rTime2 = rTime1 * 2 ; rTime3 = rTime1 / 2 ; rTime7 = rTime1 ;  } 
          else
            {  // Subsequent DOTHs
            bTemp = false ;
            if (( rTime1 > (rTime3 - (rTime3/5))) && (rTime1 < (rTime3 + (rTime3/5)))) { rTime4 += rTime1 ; iCount4 += 1 ; bTemp = true ;}
            if (( rTime1 > (rTime2 - (rTime2/5))) && (rTime1 < (rTime2 + (rTime2/5)))) { rTime5 += rTime1 ; iCount5 += 1 ; bTemp = true ;}
            if (( rTime1 > (rTime7 - (rTime7/5))) && (rTime1 < (rTime7 + (rTime7/5)))) { rTime6 += rTime1 ; iCount6 += 1 ; bTemp = true ;}
            }
            if ( bTemp == false ) { Message(2,"Houston, we have a problem : " , rTime6 , ""); }
          }
        else  
          {  //  === After tenth DOTH - Set the DOT and DASH times
          rTime4 = rTime4 / iCount4 ; rTime5 = rTime5 / iCount5 ; rTime6 = rTime6 / iCount6 ;
          if ( rTime4 == 0 ) //  === Means that the first DOTH was a DOT
            { DashTime = rTime5 ; DotTime = rTime6 ; }
          else
            { DashTime = rTime6 ; DotTime = rTime4 ; }
          // ==== If necessary stay in AutoTune mode, restarting to the first DOTH to resolve the issue
          if (( DotTime < 10 ) || ( DashTime < 10 ))
            { DOTH_Count = -1 ; Serial.println( " ..." ) ; }
          else
            { MessageSTATUS() ; }
          }
        DOTH_Count += 1 ;
      }
    else
      {
        float NowMinusRiseE = now - RiseEdge ;
        fTemp1 = float(DashTime) ; fTemp2 = fTemp1 / 5.0 ;
        if ((NowMinusRiseE > ( fTemp1 - fTemp2 )) && ( NowMinusRiseE < ( fTemp1 + fTemp2 ))) { StrngDD += "-" ;
          // ============ DASH DETECTED ============================================
          Message(3,Recvd+" DASH  : ",NowMinusRiseE,StrngDD) ;
        }
        fTemp1 = float(DotTime) ; fTemp2 = fTemp1 / 5.0 ;
        if ((NowMinusRiseE > ( fTemp1 - fTemp2 )) && ( NowMinusRiseE < ( fTemp1 + fTemp2 ))) { StrngDD += "." ;
          // ============= DOT DETECTED ==========================================
          Message(3,Recvd+"  DOT  : ",NowMinusRiseE,StrngDD) ;
        }
      }
  }
  IN_Old = IN ; // Copy IN into Previous Scan version  ( IN_Old )
}



void ReadSerial() {
  // ============ Read Serial from PC ==============
  // If a Carriage Return OR LineFeed is detected then convert that to a space
  // **************** Should perhaps be a double space **********
  //  This needs developing as at the moment it just Prints to the screen 
  int RecvdByte = 0;   // for Recvd serial data
  while (Serial.available() > 0) {
    RecvdByte = toUpperCase(char(Serial.read()));
    if ( RecvdByte == 48 ){  // ============================== 0  to AutoTune ========
      DOTH_Count = 0 ; 
    } 
    if (( RecvdByte > 47 ) && ( RecvdByte < 53 )){ // ==== To Set Reporting Level   0 - 4 ====
      iReportLvl = RecvdByte - 48 ; RecvdByte = 0 ;
      if (DOTH_Count > 10 ){ MessageSTATUS(); } ;
    }
    if ( RecvdByte == 53 ){  // ==== 5  to Stop TickerTape ( immediately ) ====
        iOwner = 3 ; //TickTape = 1 - TickTape ;
        TickTapeEOM = 0 ;
        Message(0,"\n  Ticker Tape " , TickTape , "" ) ;
        MessageSTATUS() ; RecvdByte = 0 ;} 
    if ( RecvdByte == 54 ){  // ==== 6  to Stop TickerTape ( at end of message ) OR ReStart ==== 
        TickTapeEOM = 1 - TickTapeEOM ;
        MessageSTATUS() ; 
        if (TickTapeEOM == false)
          {Message(0,"\n  Ticker Tape " , TickTapeEOM , "will take effect at end of current message ( Press 6 again to Toggle )\n" ) ;}
        else
          {if (iOwner == 3){iOwner=0;}}   // If it were in Tapper Mode - release the Owner
        RecvdByte = 0 ;} 
    if (( RecvdByte == 55 ) || ( RecvdByte == 56 )){  
        if ( RecvdByte == 55 ){ fTemp1 = 0.5 ; sTemp = "Slower" ;}// ==== 7  to Halve  TickerTape Speed
        if ( RecvdByte == 56 ){ fTemp1 = 2.0 ; sTemp = "Faster" ;}// ==== 8  to Double TickerTape Speed
        TT_Period /= fTemp1 ;
        //DotTime    /= fTemp1 ;
        //DashTime   /= fTemp1 ;
        MessageSTATUS() ;
        Message0("TickerTape speed " + sTemp + "\n" ) ; RecvdByte = 0 ;
    } 
    if ( RecvdByte == 57 ){  // ==== 9  to Toggle LED permanently ON
        LED_On = 1 - LED_On ;
        MessageSTATUS() ;
        Message(0,"LED override = " , LED_On , " ( Press 9 again to Toggle )" ) ; RecvdByte = 0 ;
    } 
    if ( RecvdByte > 0 ){ 
      if ( RecvdByte == 10 ) {
        if ( RecvdStrng.length() > 0 ) { RecvdStrng += " " ; }
        Read_1 += 1 ; }
      else
        { RecvdStrng += GetAlpha(RecvdByte) ;}
      if (( RecvdStrng.length() > 0 ) && (Read_1 != Read_2)) { 
//        Serial.println("Waiting for slot to transmit :- " + RecvdStrng )
        Read_2 = Read_1 ;
      }
    }
  }
  if (((iOwner == 0 ) || (iOwner == 2)) && (RecvdStrng.length() > 0 )){ 
    iOwner = Transmit(RecvdStrng,2) ;
    if (iOwner == 0 ) { RecvdStrng = "" ;}
  }
}



int Transmit(String Source, int i_Owner) {
  //            i_Owner = 0 - No Owner
  //                      1 - From TickerTape Loop - Quick Brown Fox Jumped over the lazy dog
  //                      2 - Serial Read ( from serial port of Arduino )
  //                      3 - Morse from Tapper Pushbutton
  if ( now > ( prev + TT_Period )) { prev = now ;
    if ( iDD < 1 ) {   // New Letter needed
      if (( TransmitPtrDD >= MorseDD.length()) || ( MorseDD.length() == 0 )) { TransmitPtrDD = 9 ; TransmitPtr01 ++ ; }
      if ( TransmitPtr01  > Source.length() ){ TransmitPtr01 = 0 ; return 0 ; }
      sTemp = Source[TransmitPtr01-1] ;
      if ( sTemp == ' ' ) {
        TransmitPtrDD = 99 ; } // ======== Puts an exta space in stream after a word
      else {
        MorseDD = GetMorseDD( Source[TransmitPtr01-1] ) ;
       }
      if ( TransmitPtrDD == 9 )
        { iDD = 1 ; TransmitPtrDD = 0 ; } //  ====== Puts an extra space in stream after a letter.
      else {
        if (TransmitPtrDD == 99 )
          { iDD = 1 ; TransmitPtrDD = 0 ;}
        else
          {
            if ( MorseDD[TransmitPtrDD] == '-' ) { iDD = 3 ; }  // 110
            if ( MorseDD[TransmitPtrDD] == '.' ) { iDD = 2 ; }  // 10
            TransmitPtrDD ++ ;
          }
      }
    }
    iLED = iDD ;
    iDD -- ; // Subtract one from iDD
  }
  return i_Owner ;
}



void loop() {
//  int Sys2Counter = 0 ;
  if ( SysCounter == 1 ) {  ReadPhotocell() ;} // Read the Photocell
  if ( SysCounter == 2 ) {  ReadSerial()    ;} // Read the Serial Port ( PC )
  SysCounter += 1 ;  if ( SysCounter >= 3 ) { SysCounter = 1 ; } // Sys2Counter ++ }
  if (((iOwner==0) && (RecvdStrng.length() == 0 ) && ( TickTapeEOM)) || (iOwner == 1)) {
    iOwner = Transmit("The quick brown fox jumped over the lazy dog   ",1) ; }
  if ( digitalRead(7) ){  // ==== MorseTapper Stops TickerTape ( immediately ) ====
    iOwner = 3 ; //TickTape = 1 - TickTape ;
    TickTapeEOM = 0 ;
  }
  iTemp = (iLED * (iOwner != 3) ) + (LED_On * 2) ;
  if ( digitalRead(7)) { iTemp = 2 ; } // MorseTapper

  test1("dummy") ;

  if ( iTemp > 1 ) { iTemp = 255 ;} else { iTemp = 0; }
//  iTemp = ((iLED + LED_On * 2 + digitalRead(MorseTapper)) > 1) * 255 ;
  analogWrite(9, iTemp) ; // LED to Send Morse
}



void test1(String Strng9 )
//void test(int i, int j , int k )
  { bTemp = digitalRead(6) ; // Test Pushbutton
  if ( bTemp && ( bTest_1 == false )) { 
     Serial.println(String(iOwner,DEC)+" "+String(iLED,DEC)+" "+String(iTemp,DEC)+" "+String(DOTH_Count,DEC)) ;
//    Serial.print("  1=" + String(i,DEC) + "  2=" + String(j,DEC) + "  3=" + String(k,DEC) + "/n");
    }
  bTest_1 = bTemp ;
  }

void test2(String Strng9 )
  { bTemp = digitalRead(6) ; // Test Pushbutton
  if ( bTemp && ( bTest_2 == false )) { 
     Serial.println(" iOwner = " + String(iOwner,DEC) + "   RecvdStrng = " + RecvdStrng ) ;
//     Serial.println("GetMorseDD('G') = "+GetMorseDD('G') );
//     Serial.println(" GetMorse01('.-.') = " + GetMorse01(".-.")) ;
    }
  bTest_2 = bTemp ;
  }

// Ennd of Morse
