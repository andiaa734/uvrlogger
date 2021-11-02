#ifndef UVR16X2_H
#define UVR16X2_H

#include "Arduino.h"
#include "canopen.h"
#include "RemoteDebug.h"

#define MODE_HAND_AUS 0x00
#define MODE_HAND_AN  0x04
#define MODE_AUTO	  0x02

extern RemoteDebug Debug;

void connect(uint8_t clientID);
void disconnect(uint8_t clientID);
void readInlets();
void wait(uint32 Id, can_frame &rframe);
void sendFrame(can_frame frame);


const char* const Units[] {

"", // dimensionslos
"°C", // Grad Celsius Temperatur
"W/m²", // Watt/Quadratmeter Globalstrahlung
"l/h", // Liter/Stunde Durchfluss
"Sek", // Sekunden Zeit
"Min", // Minuten Zeit
"l/Imp",// Liter/Impuls Quotient
"K", // Kelvin Temperatur
"%", // % % , rel Luftfeute
":", //: : :
"kW", // Kilowatt Leistung
"kWh", //Kilowattstunden Wärmemenge
"MWh", //Megawattstunden Wärmemenge
"V", //Volt Spannung
"mA", //milli Ampere Stromstärke
"Std", //Stunden Zeit
"Tage", //Tage Zeit
"Imp", //Impulse Impulse
"kOhm", //Kilo Ohm Widerstand
"l", //Liter Wassermenge nicht verwendet
"km/h", //Kilometer/Stunde Windgeschwindigkeit
"Hz", //Hertz Frequenz
"l/min", //Liter/Minute Durchfluss
"bar", //bar Druck
" ", //Arbeitszahl CAN-EZ
"km", //Kilometer Distanz
"m", //Meter Distanz
"mm", //Millimeter Distanz
"m³", //Kubikmeter Luftmenge
"Hz/km/h", //Hertz/km/Stunde Windgeschwindigkeit
"Hz/m/s", // Hertz/Meter/Sek Windgeschwindigkeit
"kWh/Imp", //kWh/Impuls Leistung
"m³/Imp", //Kubikmeter/Impuls Luftmenge
"mm/Imp", //Millimeter/Impuls Niederschlag
"L/Imp", //Liter/Impuls Durchfluss(4 Komma)
"l/d", //Liter/Tag Durchfluss
"m/s", //Meter/Sekunde Geschwindigkeit
"m³/min", //Kubikmeter/Minute Durchfluss(Gas/Luft)
"m³/h", //Kubikmeter/Stunde Durchfluss(Gas/Luft)
"m³/d", //Kubikmeter/Tag Durchfluss(Gas/Luft)
"mm/min", //Millimeter/Minute Regen
"mm/h", //Millimeter/Stunde Regen
"mm/d", //Millimeter/Tag Regen
"", //Einheit für Digitalwert(Aus/Ein)
"", //Einheit für Digitalwert(Nein/Ja)
"", //Einheit für RAS-Modus
"°C", //RAS Temperatur Temperatur + RAS-Modus
"", //Mischer Mischerausgang
"", //Heizkreis Betriebsart( aus,normal,abgesenkt,Standby,Frostschutz, .... )
"", //Heizkreis Betriebssstufe( Sonderbetrieb,Fensterk.,extern,Kalender,ras, intern )
"€", //Euro Währung
"$", //Dollar Währung
"g/m³", //Gramm/Kubikmeter Absolute Luftfeuchtigkeit
"", //Preis/Einheit
"°", //Grad Sonnenhöhe, Sonnenrichtung
"", //Lamellenstellung(1Byte)+Jalousiestellung(1Byte)
"°", //Grad Koordinaten
"Sek", //Sekunden Zehntel Sekunden 1Komma
"", //dimlos 1 Komma
"%", //Prozent Prozent ohne Komma für Jalousie Pos
"Uhr", //Uhr Uhrzeit
"", //Tag&Monat Tag und Monat
"", //Datum Jahr, Monat, Tag
};

struct Inlet {
	ObjectIndex Description;
	ObjectIndex Value;
	ObjectIndex Mode;
	ObjectIndex State; 
};

struct Outlet {
	ObjectIndex Description; 
	ObjectIndex StartDelay; 
	ObjectIndex RunOnTime;   
	ObjectIndex Mode;        
	ObjectIndex State;       
	ObjectIndex SpeedStage;  
};

struct Heatmeter {

	ObjectIndex Description; 
	ObjectIndex Flowtemperatur; 
	ObjectIndex Returntemperatur;   
	ObjectIndex Flow;        
	ObjectIndex Power;       
	ObjectIndex PowerTotal;
	ObjectIndex State;  

};
ObjectIndex NewObjectIndex(uint16 index, uint8 subIndex) ;
Inlet NewInlet(uint8 subIndex);
Outlet NewOutlet(uint8 subIndex);
Heatmeter NewHeatmeter(uint8 subIndex);

void readInlet(Inlet *inlet, std::string *descr, int *state, float *val, int *unit);
void readOutlet(Outlet *outlet, std::string *descr, int *imode, std::string *mode, int *state);
void readHeatmeter(Heatmeter *heatmeter, std::string *descr, float *tflow, int *tfunit, float *treturn, int *trunit, int *flow, int *funit, std::string *state, float *power, int *upower, float *powertotal, int *ptunit);
//void readInlets();
void readOutlets();
void readHeatMeters();
void getObject(ObjectIndex index, byte *data_buffer, byte *len);

void getTime();
std::string readStringfromIndex(byte *input, byte *len);
float readfloatfromIndex(byte *input, byte *len);
int readintfromIndex(byte *input, byte *len);
void readError();
void retransmit(can_frame &frame, char &ackseq, char &blocksize);
void ack_block(can_frame &frame, char &ackseq, char &blksize);
void end_upload(can_frame frame);
void readBlock(ObjectIndex index, byte *data_buffer, byte *len);
void end_block_download(can_frame &frame);
void writeBlock(ObjectIndex index, byte *data_buffer, int len);
void setState(Outlet outlet, int state);




#endif