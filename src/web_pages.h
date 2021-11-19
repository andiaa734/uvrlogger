#ifndef WEB_PAGES_H
#define WEB_PAGES_H

#include "Arduino.h"

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>CAN Frame Builder</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem; color: #FF0000;}
  </style>
  </head><body>
  <h2>CAN Frame Builder</h2> 
  <form action="/get">
    COB ID: <input type="text" name="cobid">
  <br>
    Response COB ID: <input type="text" name="rescobid">
  <br>
    Node ID: <input type="text" name="nodeid">
  <br>
    DLC: <input type="text" name="dlc">
  <br>
    Data Byte 0: <input type="text" name="b0" value="0x0">
  <br>
    Data Byte 1: <input type="text" name="b1" value="0x0">
  <br>
    Data Byte 2: <input type="text" name="b2" value="0x0">
  <br>
    Data Byte 3: <input type="text" name="b3" value="0x0">
  <br>
    Data Byte 4: <input type="text" name="b4" value="0x0">
  <br>
    Data Byte 5: <input type="text" name="b5" value="0x0">
  <br>
    Data Byte 6: <input type="text" name="b6" value="0x0">
  <br>
    Data Byte 7: <input type="text" name="b7" value="0x0">
 
  <div>
  <input type="checkbox" id="singleframe" name="singleframe">
  <label for="singleframe">Single Frame</label>
  <input type="checkbox" id="blockread" name="blockread">
  <label for="blockread">Block lesen</label>
  <input type="checkbox" id="crc" name="crc">
  <label for="string">CRC16 anhaengen</label>
   Laenge: <input type="text" name="crclen" value="0">
  </div>
    <div>
  <input type="checkbox" id="write" name="write">
  <label for="write">Schreiben</label>
  
  </div>
  <div>
  <input type="checkbox" id="string" name="string">
  <label for="string">Rueckgabe als String</label>
  
  
  <input type="checkbox" id="int" name="int">
  <label for="int">Rueckgabe als int</label>
  
  
  <input type="checkbox" id="float" name="float">
  <label for="float">Rueckgabe als float</label>
  </div>
   <br>
    <input type="submit" value="Submit">  
  <br>
    <form action="/disconnect">
    <input type="submit" value="Disconnect">

  </form>
</body></html>)rawliteral";

#endif
