#ifndef UVR16X2_H
#define UVR16X2_H

#include "Arduino.h"
#include "canopen.h"
#include "RemoteDebug.h"
#include "crc16.h"

#define MODE_HAND_AUS 0x00
#define MODE_HAND_AN  0x04
#define MODE_AUTO	  0x02

extern RemoteDebug Debug;
extern Crc16 crc16;


void connect(uint8_t clientID);
void disconnect(uint8_t clientID);
void readInlets();
void wait(uint32 Id, can_frame &rframe);
void sendFrame(can_frame frame);


const char* const Units[] {

"", 	// 0 dimensionslos
"°C", 	// 1 Grad Celsius Temperatur
"W/m²", // 2 Watt/Quadratmeter Globalstrahlung
"l/h", 	// 3 Liter/Stunde Durchfluss
"Sek", 	// 4 Sekunden Zeit
"Min", 	// 5 Minuten Zeit
"l/Imp",// 6 Liter/Impuls Quotient
"K", 	// 7 Kelvin Temperatur
"%", 	// 8 % % , rel Luftfeute
":", 	// 9 : : :
"kW", 	// 10  Kilowatt Leistung
"kWh", 	// 11 Kilowattstunden Wärmemenge
"MWh", 	// 12 Megawattstunden Wärmemenge
"V", 	// 13 Volt Spannung
"mA", 	// 14 milli Ampere Stromstärke
"Std", 	// 15 Stunden Zeit
"Tage", // 16 Tage Zeit
"Imp", 	// 17 Impulse Impulse
"kOhm", // 18 Kilo Ohm Widerstand
"l", 	// 19 Liter Wassermenge nicht verwendet
"km/h", // 20 Kilometer/Stunde Windgeschwindigkeit
"Hz", 	// 21 Hertz Frequenz
"l/min",// 22 Liter/Minute Durchfluss
"bar", 	// 23 bar Druck
" ", 	// 24 Arbeitszahl CAN-EZ
"km", 	// 25 Kilometer Distanz
"m", 	// 26 Meter Distanz
"mm", 	// 27 Millimeter Distanz
"m³", 	// 28 Kubikmeter Luftmenge
"Hz/km/h", 	// 29 Hertz/km/Stunde Windgeschwindigkeit
"Hz/m/s", 	// 30 Hertz/Meter/Sek Windgeschwindigkeit
"kWh/Imp",	// 31 kWh/Impuls Leistung
"m³/Imp", 	// 32 Kubikmeter/Impuls Luftmenge
"mm/Imp", 	// 33 Millimeter/Impuls Niederschlag
"L/Imp", 	// 34 Liter/Impuls Durchfluss(4 Komma)
"l/d", 		// 35 Liter/Tag Durchfluss
"m/s", 		// 36 Meter/Sekunde Geschwindigkeit
"m³/min", 	// 37 Kubikmeter/Minute Durchfluss(Gas/Luft)
"m³/h", 	// 38 Kubikmeter/Stunde Durchfluss(Gas/Luft)
"m³/d", 	// 39 Kubikmeter/Tag Durchfluss(Gas/Luft)
"mm/min", 	// 40 Millimeter/Minute Regen
"mm/h", 	// 41 Millimeter/Stunde Regen
"mm/d", 	// 42 Millimeter/Tag Regen
"", 		// 43 Einheit für Digitalwert(Aus/Ein)
"", 		// 44 Einheit für Digitalwert(Nein/Ja)
"", 		// 45 Einheit für RAS-Modus
"°C", 		// 46 RAS Temperatur Temperatur + RAS-Modus
"", 		// 47 Mischer Mischerausgang
"", 		// 48 Heizkreis Betriebsart( aus,normal,abgesenkt,Standby,Frostschutz, .... )
"", 		// 49 Heizkreis Betriebssstufe( Sonderbetrieb,Fensterk.,extern,Kalender,ras, intern )
"€", 		// 50 Euro Währung
"$", 		// 51 Dollar Währung
"g/m³",		// 52 Gramm/Kubikmeter Absolute Luftfeuchtigkeit
"", 		// 53 Preis/Einheit
"°", 		// 54 Grad Sonnenhöhe, Sonnenrichtung
"", 		// 55 Lamellenstellung(1Byte)+Jalousiestellung(1Byte)
"°", 		// 56 Grad Koordinaten
"Sek",		// 57 Sekunden Zehntel Sekunden 1Komma
"", 		// 58 dimlos 1 Komma
"%", 		// 59 Prozent Prozent ohne Komma für Jalousie Pos
"Uhr", 		// 60 Uhr Uhrzeit
"", 		// 61 Tag&Monat Tag und Monat
"", 		// 62 Datum Jahr, Monat, Tag
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

struct Setting{

	ObjectIndex OperationMode;
	ObjectIndex WaterPriority;
	ObjectIndex WaterTargetTemperature;
	

};

struct Variable{

	ObjectIndex FlowTargetTemperature;
	ObjectIndex Operation;
};



ObjectIndex NewObjectIndex(uint16 index, uint8 subIndex) ;
Inlet NewInlet(uint8 subIndex);
Outlet NewOutlet(uint8 subIndex);
Heatmeter NewHeatmeter(uint8 subIndex);
Setting NewSettings(uint8 subIndex);
Variable NewVariables(uint8 subIndex);


void readInlet(Inlet *inlet, std::string *descr, int *state, float *val, int *unit);
void readOutlet(Outlet *outlet, std::string *descr, int *imode, std::string *mode, int *state, int *unit);
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
void readBlock(ObjectIndex index, byte *data_buffer, byte *len);
void end_block_download(can_frame &frame);
void writeBlock(ObjectIndex index, byte *data_buffer, int len);
void setState(ObjectIndex index, char *data_buffer, int len);




#endif