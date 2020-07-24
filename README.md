The Morse code is transmitted via the LED for detection by the photocell of the recipient ( might be the same device - if the LED is shining back into the photocell )   ( Similar code exists for the Raspberry Pi written in Julia )

  Will convert the Recvd morse ( from it's pair  OR in Loopback )  and present it in characters.
  In the INPUT section of the monitor :-
     Type in your sentence 'HOW ARE YOU'     which will get converted to Morse Code and transmitted at the end of the current TickerTape message 
  OR Press 5 or 6 to turn OFF TickerTape then
     Tap out your morse using the Tapper on the breadboard
     In Tapper Mode if the last letter of the word FOX is received - it restarts TickerTape
  OR Press 6 to set the LED to permanently ON then
     use your finger or an object to block the LED light getting to the photocell
     thus creating Morse ( in reverse )
  
  Use ONE   space  between Letters
  and TWO   spaces between Words 
  and THREE spaces at the end of a Sentence
  
  Press :-
        0 - To set Reporting Level 0 & Do an AutoTune  ( normal )
        1 - To set Reporting Level 1
        2 - To set Reporting Level 2
        3 - To set Reporting Level 3
        4 - To set Reporting Level 4
        5 - To toggle TickerTape OFF ( immediately )
        6 - To toggle TickerTape ON and OFF ( after the end of the current message )
        7 - To Halve  TickerTape Speed
        8 - To Double TickerTape Speed
        9 - To toggle the LED being permanently ON

  grmccormack@googlemail.com
