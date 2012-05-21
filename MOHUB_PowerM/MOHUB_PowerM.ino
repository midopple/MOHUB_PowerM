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
 along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 
 */

 /*
 Changelog:
 V1.01
   First test
 V1.02 (10.05.2012)
   Measure the 3,3V Ref Voltage for AIN correctur USB 5V is not STABLE
   
 
 
 */
 
#define _VERSION "V1.02"
#define _VERSION_DATE "11.05.2012"

#include <avr/pgmspace.h>
#include "MOHUB_PowerM.h"


POWER_VAL_TABLE mohub_power_vals[MAX_MOHUBS_GROUPS+1] = {{0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0},
                                                         {0,0,0,0,0,0,0,0}};
                                                         
unsigned char AD_values_OK = 0;

                                                        
//------------------------------------------------
//Print a String from Flash to Serial (save RAM)
//------------------------------------------------                                                         
void showStringPGM (PGM_P s) 
{
  char c;
  
  while ((c = pgm_read_byte(s++)) != 0)
    Serial.print(c);
}                                                         

#define PRINT_PGM_STR(x) showStringPGM(PSTR(x))

//-----------------------------------------------

//------------------------------------------------
//INIT 
//------------------------------------------------
void setup() 
{
  
  pinMode(MUX_S1, OUTPUT); // Define Selection Pins for HEF4051 as Output
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  
  //ANALOG Reference is 5V VCC (USB 5V is not Stable :-( )
  analogReference(DEFAULT);

  // initialize the UART 
  Serial.begin(UART_BAUD_RATE); 
  
  //SHOW VERSIONIFO
  PRINT_PGM_STR("MOHUB Powermeter "); 
  PRINT_PGM_STR(_VERSION);PRINT_PGM_STR(" / "); 
  PRINT_PGM_STR(_VERSION_DATE); 
  PRINT_PGM_STR("\r\n"); 
  
}

//------------------------------------------------
//MAIN LOOP
//------------------------------------------------
void loop() 
{
  
  //-- Read Analog IN Values --
  #ifndef SIMULATION_MODE
    read_analog_in();
  #else
    set_simulation_to_array();
    AD_values_OK = 1;
  #endif
  //---------------------------
  
  //-- Calculate Ah, Wh -------
  if(AD_values_OK)calc_Ah_Wh();
  //---------------------------

  //---------------------------
  //-- Send to Uart -----------
  if(AD_values_OK)send_data_to_UART();
  
  //---------------------------
  

}


//------------------------------------------------
//Select with the HEF4051 Multiplexer the right AIN cannel
//------------------------------------------------
void select_HEF4051(unsigned char cannel)
{
  
  if(cannel & 0x01)
    digitalWrite(MUX_S1, HIGH);
  else  
    digitalWrite(MUX_S1, LOW);

  if(cannel & 0x02)
    digitalWrite(MUX_S2, HIGH);
  else  
    digitalWrite(MUX_S2, LOW);

  if(cannel & 0x04)
    digitalWrite(MUX_S3, HIGH);
  else  
    digitalWrite(MUX_S3, LOW);


}


//------------------------------------------------
//Read ADC Values and calculate the current, power and voltage
//------------------------------------------------
void read_analog_in(void)
{
    static int analog_in_sum[MAX_ANALOG_IN]={0,0,0,0,0,0};
    static unsigned long previous_millis_AIN_sample = millis();
    static unsigned char sample_cnt = 0;
    int AIN_sample = 0;
    long help_ain_curr = 0;
    long VCC_korr_val = 0;
    static int help_3_3_V = 0;
    
    if((millis() - previous_millis_AIN_sample) > 100 )
    {
      previous_millis_AIN_sample = millis();
      
      select_HEF4051(MUX_U_BAT);    //Ubat
      
      sample_cnt++;
      
      AIN_sample = analogRead(AIN_MUX_HEF4051);
      analog_in_sum[0] += AIN_sample;
      
      select_HEF4051(MUX_U_3_3V );  //3,3V

      AIN_sample = analogRead(AIN_CURR_INVERTER);
      analog_in_sum[1] += AIN_sample;

      AIN_sample = analogRead(AIN_CURR_GROUP_1);
      analog_in_sum[2] += AIN_sample;

      AIN_sample = analogRead(AIN_CURR_GROUP_2);
      analog_in_sum[3] += AIN_sample;

      AIN_sample = analogRead(AIN_CURR_GROUP_3);
      analog_in_sum[4] += AIN_sample;

      AIN_sample = analogRead(AIN_CURR_GROUP_4);
      analog_in_sum[5] += AIN_sample;
      
      //Read 3,3 V Refencetvoltage
      AIN_sample = analogRead(AIN_MUX_HEF4051);
      help_3_3_V += AIN_sample;

      select_HEF4051(MUX_U_BAT);    //Ubat
      
    }
    
    if(sample_cnt >= 10)
    {
      sample_cnt = 0;

      //Korrection with 3,3 Ref Voltage, because the USB 5V not stable
      VCC_korr_val = 6751000;
      VCC_korr_val = VCC_korr_val / help_3_3_V;
     
 
      //Inverter / Power user
      mohub_power_vals[0].spannung = (unsigned int)(((long)analog_in_sum[0] * MAX_AIN_VOLTAGE * VCC_korr_val) / 1023000);
      
      
      mohub_power_vals[1].spannung = mohub_power_vals[0].spannung;
      mohub_power_vals[2].spannung = mohub_power_vals[0].spannung;
      mohub_power_vals[3].spannung = mohub_power_vals[0].spannung;
      mohub_power_vals[4].spannung = mohub_power_vals[0].spannung;
      
      help_ain_curr = ((long)(analog_in_sum[1]) * 5 * VCC_korr_val) / 10230;
      help_ain_curr -= (OFFSET_AIN_CURRENT_INVERTER * VCC_korr_val) / 1000;    
      mohub_power_vals[0].strom = (unsigned int)((help_ain_curr*10) / MV_A_AIN_CURRENT_INVERTER);
      mohub_power_vals[0].leistung = (unsigned int)(((long)mohub_power_vals[0].strom * (long)mohub_power_vals[0].spannung) / 1000);
      
      //Current Group 1
      help_ain_curr = ((long)(analog_in_sum[2]) * 5 * VCC_korr_val) / 10230;
      help_ain_curr -= (OFFSET_AIN_CURRENT_GROUP_1 * VCC_korr_val) / 1000; 
      mohub_power_vals[1].strom = (unsigned int)((help_ain_curr*10) / MV_A_AIN_CURRENT_GROUP_1);
      mohub_power_vals[1].leistung = (unsigned int)(((long)mohub_power_vals[1].strom * (long)mohub_power_vals[1].spannung) / 1000);
      //Current Group 2
      help_ain_curr = ((long)(analog_in_sum[3]) * 5 * VCC_korr_val) / 10230;
      help_ain_curr -= (OFFSET_AIN_CURRENT_GROUP_2 * VCC_korr_val) / 1000; 
      mohub_power_vals[2].strom = (unsigned int)((help_ain_curr*10) / MV_A_AIN_CURRENT_GROUP_2);
      mohub_power_vals[2].leistung = (unsigned int)(((long)mohub_power_vals[2].strom * (long)mohub_power_vals[2].spannung) / 1000);
      //Current Group 3
      help_ain_curr = ((long)(analog_in_sum[4]) * 5 * VCC_korr_val) / 10230;
      help_ain_curr -= (OFFSET_AIN_CURRENT_GROUP_3 * VCC_korr_val) / 1000; 
      mohub_power_vals[3].strom = (unsigned int)((help_ain_curr*10) / MV_A_AIN_CURRENT_GROUP_3);
      mohub_power_vals[3].leistung = (unsigned int)(((long)mohub_power_vals[3].strom * (long)mohub_power_vals[3].spannung) / 1000);
      //Current Group 4
      help_ain_curr = ((long)(analog_in_sum[5]) * 5 * VCC_korr_val) / 10230;
      help_ain_curr -= (OFFSET_AIN_CURRENT_GROUP_4 * VCC_korr_val) / 1000; 
      mohub_power_vals[4].strom = (unsigned int)((help_ain_curr*10) / MV_A_AIN_CURRENT_GROUP_4);
      mohub_power_vals[4].leistung = (unsigned int)(((long)mohub_power_vals[4].strom * (long)mohub_power_vals[4].spannung) / 1000);
     
      analog_in_sum[0] = 0;
      analog_in_sum[1] = 0;
      analog_in_sum[2] = 0;
      analog_in_sum[3] = 0;
      analog_in_sum[4] = 0;
      analog_in_sum[5] = 0;
      help_3_3_V  = 0;
      
      AD_values_OK = 1;

    }
}  


//------------------------------------------------
//Write Simulation values to the Array, only for testing
//------------------------------------------------
#ifdef SIMULATION_MODE
void set_simulation_to_array(void)
{
      static long sim_strom[5] = {600,400,300,500,450};
      static long sim_volt = 1354;
      long sim_leistung = 0;
      int help_sim = 0;
      static unsigned long previous_millis_SIM_sample = millis();
      
      
      if((millis() - previous_millis_SIM_sample) > 100 )
      {
        previous_millis_SIM_sample = millis();
        
        
        help_sim = random(400, 1400);
        if(help_sim > sim_strom[0])sim_strom[0]++;
        if(help_sim < sim_strom[0])sim_strom[0]--;
  
  
        for(unsigned char cnt_s=1;cnt_s<5;cnt_s++)
        {
          help_sim = random(200, 900);
          if(help_sim > sim_strom[cnt_s])sim_strom[cnt_s]++;
          if(help_sim < sim_strom[cnt_s])sim_strom[cnt_s]--;
        }  
        
      
    
   
        mohub_power_vals[0].spannung = (unsigned int)(sim_volt+random(10, 30));
        
        
        mohub_power_vals[1].spannung = mohub_power_vals[0].spannung;
        mohub_power_vals[2].spannung = mohub_power_vals[0].spannung;
        mohub_power_vals[3].spannung = mohub_power_vals[0].spannung;
        mohub_power_vals[4].spannung = mohub_power_vals[0].spannung;
      
        mohub_power_vals[0].strom = (unsigned int)(sim_strom[0]);
        mohub_power_vals[0].leistung = (unsigned int)(((long)mohub_power_vals[0].strom * (long)mohub_power_vals[0].spannung) / 1000);
        
        //Current Group 1
        mohub_power_vals[1].strom = (unsigned int)(sim_strom[1]);
        mohub_power_vals[1].leistung = (unsigned int)(((long)mohub_power_vals[1].strom * (long)mohub_power_vals[1].spannung) / 1000);
        //Current Group 2
        mohub_power_vals[2].strom = (unsigned int)(sim_strom[2]);
        mohub_power_vals[2].leistung = (unsigned int)(((long)mohub_power_vals[2].strom * (long)mohub_power_vals[2].spannung) / 1000);
        //Current Group 3
        mohub_power_vals[3].strom = (unsigned int)(sim_strom[3]);
        mohub_power_vals[3].leistung = (unsigned int)(((long)mohub_power_vals[3].strom * (long)mohub_power_vals[3].spannung) / 1000);
        //Current Group 4
        mohub_power_vals[4].strom = (unsigned int)(sim_strom[4]);
        mohub_power_vals[4].leistung = (unsigned int)(((long)mohub_power_vals[4].strom * (long)mohub_power_vals[4].spannung) / 1000);
      }
  
}
#endif

//------------------------------------------------
//Accumulate the Ah and Wh
//------------------------------------------------
void calc_Ah_Wh(void)
{
  static unsigned long previous_millis_calc_Energie = millis();
    
  if((millis() - previous_millis_calc_Energie) > 1000 )
  {
    previous_millis_calc_Energie = millis();
    
    for(char cnt_i = 0;cnt_i < (MAX_MOHUBS_GROUPS+1);cnt_i++)
    {
      mohub_power_vals[cnt_i].help_Ah += mohub_power_vals[cnt_i].strom;
      if(mohub_power_vals[cnt_i].help_Ah > 3600)
      {
        mohub_power_vals[cnt_i].help_Ah-=3600;
        mohub_power_vals[cnt_i].Ah++;
      }
      if(mohub_power_vals[cnt_i].help_Ah < -3600)
      {
        mohub_power_vals[cnt_i].help_Ah+=3600;
        mohub_power_vals[cnt_i].Ah--;
      }
      
      mohub_power_vals[cnt_i].help_Wh += mohub_power_vals[cnt_i].leistung;
      if(mohub_power_vals[cnt_i].help_Wh > 3600)
      {
        mohub_power_vals[cnt_i].help_Wh-=3600;
        mohub_power_vals[cnt_i].Wh++;
      }
      if(mohub_power_vals[cnt_i].help_Wh < -3600)
      {
        mohub_power_vals[cnt_i].help_Wh+=3600;
        mohub_power_vals[cnt_i].Wh--;
      }
      
    }
  }
}

//------------------------------------------------
//Make the ranking, how produce more Power
//------------------------------------------------
void calc_power_rank(void)
{
  unsigned char place = 0; 
  unsigned char max_ID = 0xff;
  int max_leistung = -32000;
  unsigned char array_ID = 0;
  
  for(array_ID = 0;array_ID < (MAX_MOHUBS_GROUPS+1);array_ID++)
  {
    mohub_power_vals[array_ID].rank = 0;
  }
  
  for(place = 1;place <= 4;place++)
  {
    for(array_ID = 1;array_ID < (MAX_MOHUBS_GROUPS+1);array_ID++)
    {
      if((mohub_power_vals[array_ID].leistung > max_leistung) && (mohub_power_vals[array_ID].rank == 0))
      {
        max_leistung = mohub_power_vals[array_ID].leistung;
        max_ID = array_ID;
      }
    }
    if(max_ID < (MAX_MOHUBS_GROUPS+1))
    {
      mohub_power_vals[max_ID].rank = place;
      //Serial.print(max_ID*1);Serial.print(";");Serial.print(mohub_power_vals[max_ID].leistung);Serial.print(";");Serial.println(place);
    }
    max_ID = 0xff;
    max_leistung = -32000;
  }
  
}

//------------------------------------------------
//Send Data over UART to PC
//------------------------------------------------
void send_data_to_UART(void)
{
  
  static unsigned char array_ID = 0;
  static long sum_strom = 0;
  static long sum_leistung = 0;
  static long sum_Ah = 0;
  static long sum_Wh = 0;
  static unsigned long previous_millis_UART = millis();
  unsigned long online_time = 0;
    
  if((millis() - previous_millis_UART) > 100 )
  {
    previous_millis_UART = millis();
    
   
    switch(array_ID)
    {
      case 0:     
        calc_power_rank();
        PRINT_PGM_STR("Verbraucher;");
      break; 
      case 1:     
        PRINT_PGM_STR("Gruppe 1;");
      break; 
      case 2:     
        PRINT_PGM_STR("Gruppe 2;");
      break; 
      case 3:     
        PRINT_PGM_STR("Gruppe 3;");
      break; 
      case 4:     
        PRINT_PGM_STR("Gruppe 4;");
      break;
      case 5:     
        PRINT_PGM_STR("Gesamt: ");
        online_time = millis()/1000;
        Serial.print(online_time/3600);PRINT_PGM_STR(":");
        online_time %= 3600;
        Serial.print(online_time/60);PRINT_PGM_STR(":");
        online_time %= 60;
        Serial.print(online_time);PRINT_PGM_STR(";");
        Serial.print((float)sum_strom/10);PRINT_PGM_STR(";");
        Serial.print(sum_leistung);PRINT_PGM_STR(";");
        PRINT_PGM_STR("0;");
        Serial.print((float)sum_Ah/10);PRINT_PGM_STR(";");
        Serial.print(sum_Wh);PRINT_PGM_STR(";\r\n");
        PRINT_PGM_STR("I Love OTELO\r\n");        
        sum_strom = 0;
        sum_leistung = 0;
        sum_Ah = 0;
        sum_Wh = 0;
      break;      
    }
    
    if(array_ID < (MAX_MOHUBS_GROUPS+1))
    {
      Serial.print((float)mohub_power_vals[array_ID].strom/10);PRINT_PGM_STR(";");
      if(array_ID > 0)sum_strom+= mohub_power_vals[array_ID].strom;
      Serial.print(mohub_power_vals[array_ID].leistung);PRINT_PGM_STR(";");
      if(array_ID > 0)sum_leistung += mohub_power_vals[array_ID].leistung;
      Serial.print((float)mohub_power_vals[array_ID].spannung/100);PRINT_PGM_STR(";");
      Serial.print((float)mohub_power_vals[array_ID].Ah/10);PRINT_PGM_STR(";");
      if(array_ID > 0)sum_Ah += mohub_power_vals[array_ID].Ah;
      Serial.print(mohub_power_vals[array_ID].Wh);PRINT_PGM_STR(";");
      if(array_ID > 0)sum_Wh += mohub_power_vals[array_ID].Wh;
      Serial.print(mohub_power_vals[array_ID].rank);PRINT_PGM_STR("\r\n");
    }
   
    array_ID++;
    if(array_ID > 20) array_ID = 0;
  }
  
}



