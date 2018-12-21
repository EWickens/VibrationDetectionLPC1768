/**
*
* Eoin Wickens
* R00151204
* eoin.wickens@mycit.ie
*
* On off-chance device is frozen with USB device plugged in, plug it out and back in
* If red LED comes on as soon as vibration threshold is reached, increase the temperature threshold potentiometer
* If vibration threshold is reached immediately, increase the threshold using the vibration threshold potentiometer
**/

//Constants
static const float fACCEL_MULTIPLIER = 1.5;
static const float fTEMP_MULTIPLIER = 50;
static const int iVOLUME = 250;
static const float fGRAVITY = 1.0;

/**
* Toggles the red led on or off
* ISR For the red_ticker
**/
void Red_Toggle();

/**
* Toggles the blue led on or off
* ISR For the blue_ticker
**/
void Blue_Toggle();

/**
* Function to control which LED is flashing at a given time
**/
void Led_Control(int iFlashController);

/**
* Function is Reached upon Vibration Threshold exceeded.
* Handles temperature sensing & Initiates LED/Spkr Control.
**/
void Vibration_Threshold_Reached();

/**
*Displays the Vibration Threshold to 2 decimal places.
**/
void LCD_Stat_Update(float fPosVibThresh);

/**
* Displays the temperature threshold and current temperature.
**/
void LCD_Temp_Update(float fTempThreshold, float fCurrentTemp);

/**
* Triggered upon reaching the end times.
* You may not use this program to aid in the development, design, manufacture or production
* of nuclear, missile, chemical, or biological weapons
**/
void LCD_Armageddon_Update();

/**
* Speaker started with frequency retrieved from Calculate_Speaker_Frequency
* Decided to use this calculated frequency instead of a MIDI file as it is quite an ominous tune
**/
void Speaker_On();

/**
* Speaker frequency is calculated by getting the average of all three axis
* Mutltiplied by the temperature of the LM75B
* Multiplied by the value of the temperature potentiometer to control volume
**/
float Calculate_Speaker_Frequency();

/**
* Used to reduce code length
**/
void LCD_Init();

/**
* USB_Logger logs the current X,Y,Z axis onto an available USB Mass Storage Device that is partitioned with FAT. 
**/
void USB_Logger();