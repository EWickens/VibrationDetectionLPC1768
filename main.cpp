/**
*
* Eoin Wickens
* R00151204
* eoin.wickens@mycit.ie
*
**/

#include "mbed.h"
#include "MMA7660.h"
#include "C12832.h"
#include "LM75B.h"
#include "main.h"
#include "MSCFileSystem.h"

//Declare FileSystem
MSCFileSystem fs("fs");

//Declare LEDs
DigitalOut blue_led(p25);
DigitalOut red_led(p23);

//Declare pin to set HIGH
//Response time was measured as .2000050
//ISR is set to trigger every .2 of a second
DigitalOut pin_to_high(p15);

//Declare Timer to read response time of pin_to_high
Timer timer;

//Declare Speaker
PwmOut spkr(p26);

//Declare LCD
C12832 lcd(p5, p7, p6, p8, p11);

//Accelerometer
MMA7660 MMA(p28, p27);
//Temp Sensor
LM75B sensor(p28,p27);

//Potentiometers
AnalogIn vibPot(p19);
AnalogIn tempPot(p20);

//Tickers
Ticker red_ticker;
Ticker blue_ticker;
Ticker spkr_ticker;

//Axis'
float fZaxis_p;
float fZaxis_n;

float fYaxis_p;
float fYaxis_n;

float fXaxis_p;
float fXaxis_n;

char cAxisID;

float fVibrationThresholdReached = 0;

float fPinResponseTime;

float fCurrentTemp;

//Prints out values to Terminal
Serial pc(USBTX,USBRX);

int main(){
    //Turns LEDs off
    blue_led = 1;
    red_led = 1;
    
    //Instantiates vibration threshold variables
    float fPosVibThresh = 0.0;
    float fNegVibThresh = 0.0;
    
    
    while(1){
                
        if(fVibrationThresholdReached == 0){
            //Sets vibration threshold between 0 - 1.5G's
            fPosVibThresh = vibPot * fACCEL_MULTIPLIER; //Mutliplies the potentiometer reading by fACCEL_MULTIPLIER to map the potentiometer to 1.5G's
            fNegVibThresh = -fNegVibThresh;
            
            //Updates the LCD with the current threshold reading
            LCD_Stat_Update(fPosVibThresh);
            
            //Sets axis variables
            fZaxis_p = MMA.z();
            fZaxis_n = -MMA.z();
            
            fXaxis_p = MMA.x();
            fXaxis_p = -MMA.x();
            
            fYaxis_p = MMA.y();
            fYaxis_p = -MMA.y();
            
            //Logs Axis data to USB
            USB_Logger();
            
            /**
            Since gravity acts down on the MMA7660 at a force of .98G's, and the sensitivity of the accelerometer is 1.5g's, if you were to detect vibration
            there's only a range of .5g's up that you can measure gravity. So for this solution I decided to only measure going above the threshold on the Z Axis
            between +0.5G to 1.5G's, Giving an approximate sensitivity of .5G's on either side of the baseline reading of .98G.
            **/
            if((fZaxis_p >= fPosVibThresh && fZaxis_n <= fNegVibThresh) && (fPosVibThresh > fGRAVITY && fNegVibThresh < -fGRAVITY)){
                cAxisID = 'Z';
            }
            
            else if(fYaxis_p >= fPosVibThresh && fZaxis_n <= fNegVibThresh){
                cAxisID = 'Y';
            }
            
            else if(fXaxis_p >= fPosVibThresh && fXaxis_n <= fNegVibThresh){
                cAxisID = 'X';
            }
            
            else{
                cAxisID = NULL;
            }
            
            //Upon vibration threshold being reached
            switch(cAxisID){
                case 'Z':
                case 'Y':
                case 'X':
                    fVibrationThresholdReached = 1;
                    Vibration_Threshold_Reached();
                    break;
                case NULL:// Do nothing
                    break;
            }
        }
    } 
}
/**
* USB_Logger logs the current X,Y,Z axis onto an available USB Mass Storage Device that is partitioned with FAT. 
**/
void USB_Logger(){
    
    //Opens a file, creates it if it doesn't exist, and appends the information to the end of the file
    FILE *fp = fopen("/fs/test.csv","a"); // a is used instead of w as it appends (w was giving me trouble)
    
    fprintf(fp,"X: %.2f Y: %.2f Z: %.2f\r\n", fXaxis_p, fYaxis_p, fZaxis_p);
    
    //Closes the file
    fclose(fp);
    
}
/**
*Displays the Vibration Threshold to 2 decimal places.
**/
void LCD_Stat_Update(float fPosVibThresh){
    LCD_Init();
    lcd.printf("Vibration Threshold: %.2f", fPosVibThresh);
    wait(0.2);
}

/**
* Displays the temperature threshold and current temperature.
**/
void LCD_Temp_Update(float fTempThreshold, float fCurrentTemp){
    LCD_Init();
    lcd.printf("Axis Reached: %c\nTemp Threshold: %.2f\nTemperature:%.2f ->", cAxisID, fTempThreshold, fCurrentTemp);
    wait(0.2);
}

/**
* Triggered upon reaching the end times.
* You may not use this program to aid in the development, design, manufacture or production
* of nuclear, missile, chemical, or biological weapons
**/
void LCD_Armageddon_Update(){
    lcd.cls();
    lcd.locate(10,7);
    lcd.printf("   !!Gloom and Doom!!");
}

/**
* Used to reduce code length
**/
void LCD_Init(){
    lcd.cls();
    lcd.locate(0,3);
}

/**
* Function is Reached upon Vibration Threshold exceeded.
* Handles temperature sensing & Initiates LED/Spkr Control.
**/
void Vibration_Threshold_Reached(){
    int iFlashController;

    float fTempThreshold;
    
    bool bIsBlueSet = false; //Boolean to ensure that the blue ticker cannot be repeatedly set. 
    
    MMA.setActive(false); //Accelerometer is disabled as the I2C bus needs to be used by the LM75B.
    while(1){
        
        fCurrentTemp = sensor.read(); //Reads the latest temperature from the LM75B
        fTempThreshold = tempPot * fTEMP_MULTIPLIER; // Multiplies the temperature by fTEMP_MULTIPLIER to map the potentiometer to 50 degrees celcius
        
        //Greater than or equal to temperature threshold
        if(fCurrentTemp >= fTempThreshold){
            iFlashController = 2;
            Led_Control(iFlashController);
            LCD_Armageddon_Update();
            break;
        }
        //Under temperature threshold
        else{
            if(!bIsBlueSet){
                iFlashController = 1;
                Led_Control(iFlashController);
                bIsBlueSet = true;
            }
        }
        
        LCD_Temp_Update(fTempThreshold, fCurrentTemp);
    }   
}
/**
* Function to control which LED is flashing at a given time
**/
void Led_Control(int iFlashController){
    switch(iFlashController){
        case 1:// Blue LED Flashing @1Hz, Red LED off 
            blue_ticker.attach(&Blue_Toggle, 1.0);
            break;
        case 2://Red LED Flashing @5Hz Blue LED off W/ Activated Speaker
            blue_led = 1;
            blue_ticker.detach(); 
            
            
            //Determines the pin_to_high response time
            timer.start();
            
            red_ticker.attach(&Red_Toggle, 0.2);
            spkr_ticker.attach(&Speaker_On, 0.1); //Ticker is used to effectively poll speaker every tenth of a second.
            break;
        }
    }
    
/**
* Speaker frequency is calculated by getting the average of all three axis
* Mutltiplied by the temperature of the LM75B
* Multiplied by the value of the temperature potentiometer to control volume
**/
float Calculate_Speaker_Frequency(){
    float fAverage = (MMA.z() + MMA.x() + MMA.y()) / 3;
    fAverage *= sensor.read();
    
    fAverage *= iVOLUME;
    
    fAverage *= tempPot;
    
    return fAverage;
}

/**
* Speaker started with frequency retrieved from Calculate_Speaker_Frequency
* Decided to use this calculated frequency instead of a MIDI file as it is quite an ominous tune
**/
void Speaker_On(){
    
    spkr.period(1.0 / Calculate_Speaker_Frequency());
    
    if(spkr != 0.5)
        spkr = 0.5;
    
}
/**
* Toggles the red led on or off
* ISR For the red_ticker
**/
void Red_Toggle(){
    red_led = !red_led;
    pin_to_high = 1; //Should this actually be = 0?
    timer.stop();
    fPinResponseTime = timer.read();
    pc.printf("%f", fPinResponseTime);
}
/**
* Toggles the blue led on or off
* ISR For the blue_ticker
**/
void Blue_Toggle(){
    blue_led = !blue_led;
    
}