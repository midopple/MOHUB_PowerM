/*
 MOHUB Powermeter by Michael Doppler
 Using a Ardurino UNO
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>. */


//BAUDRATE for UART
#define UART_BAUD_RATE 57600


#define MAX_MOHUBS_GROUPS  4

#define MAX_ANALOG_IN  6

//ANALOG INPUTS
#define AIN_MUX_HEF4051    A0
#define AIN_CURR_INVERTER  A1
#define AIN_CURR_GROUP_1   A2
#define AIN_CURR_GROUP_2   A3
#define AIN_CURR_GROUP_3   A4
#define AIN_CURR_GROUP_4   A5

//MULTIPLEXER INPUTS
#define MUX_U_BAT    0
#define MUX_U_3_3V   1

//Digital IO für Multiplexer Control HEF4051
#define MUX_S1 7 // Selection Pins
#define MUX_S2 6
#define MUX_S3 5


//INPUT SCALING
#define MAX_AIN_VOLTAGE  208  //0,1 V Steps
#define MIN_AIN_VOLTAGE  0

//ACS758 200B (-/+ 200 A) --> See Datasheet
#define OFFSET_AIN_CURRENT_INVERTER  2488  //Volatge at 0A
#define MV_A_AIN_CURRENT_INVERTER 10      //mV / A

//ACS758 100U (0- 100 A)
#define OFFSET_AIN_CURRENT_GROUP_1  595  //Volatge at 0A
#define MV_A_AIN_CURRENT_GROUP_1 40     //mV / A 
//ACS758 100U (0- 100 A)
#define OFFSET_AIN_CURRENT_GROUP_2  595  //Volatge at 0A
#define MV_A_AIN_CURRENT_GROUP_2 40     //mV / A 
//ACS758 100U (0- 100 A)
#define OFFSET_AIN_CURRENT_GROUP_3  595  //Volatge at 0A
#define MV_A_AIN_CURRENT_GROUP_3 40     //mV / A 
//ACS758 100U (0- 100 A)
#define OFFSET_AIN_CURRENT_GROUP_4  595  //Volatge at 0A
#define MV_A_AIN_CURRENT_GROUP_4 40     //mV / A


//------------------------------------------------
//Function Prototyp
//------------------------------------------------
void showStringPGM (PGM_P s);
void read_analog_in(void);
void calc_Ah_Wh(void);
void calc_power_rank(void);
void send_data_to_UART(void);
void select_HEF4051(unsigned char cannel);


//------------------------------------------------
//Struct for Powermeasure Values
//------------------------------------------------
typedef struct POWER_VAL_STRUCT {
  int  strom;               //0,1 A per Unit
  int  leistung;            //1 W per Unit
  unsigned int  spannung;   //0,01 V per Unit
  long          Ah;         //0,1 Ah per Unit
  long          Wh;         // 1Wh per Unit
  unsigned char rank;       // Ranking Place 1-4
  int  help_Ah;             //help Value to acc the Ah
  int  help_Wh;             //help Value to acc the WH
} POWER_VAL_TABLE;


