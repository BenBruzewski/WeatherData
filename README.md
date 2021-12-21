# WeatherData
This is a project from my Embedded Systems Lab course. It's made using C and intended for the MSP430FG4618 microcontroller.

Detailed Report can be found here: https://drive.google.com/file/d/1Tga_IXkrZ_jU9YsCswrhX6Pg-C61xenf/view?usp=sharing

## Detailed Information:

For my final project, I have taken a text file from https://www.visualcrossing.com (weather data →
Download data for free → Make account → Download text file).
Next, I've imported this data as a .h (header) file since the MSP430 does not have native file stream functionality.
I’ve set up the header file similarly to a lookup table that could be used to define a wave in other programs.
The .h file has 3 arrays, each with their own purpose stated below:
  ● WeatherF will be used by the showData() function to calculate the LED color
  ● WeatherC will be used by UART to display the true value on MobaXTerm (sendChar/sendString)
  ● WeatherI will be used by the LCD screen since the preset LCD functions take integer input.

My program interfaces with the 3 LEDs, the LCD screen, and the 2 switches connected to the
MSP430FG4618. Depending on the weather conditions from the text file, the MSP430 will activate one of its colored LEDs as shown below:

Temperature Data: 
- 🟩 0-40F
- 🟧 40-70F
- 🟥 70+ F

Precipitation Data: 
- 🟩 0-33% chance of rain 
- 🟧 33-66% chance of rain 
- 🟥 66%+ chance of rain

The MSP430FG4618 also displays the closest integer value to the currently selected data on its 7-segment LCD display. 
The user can toggle between temperature and precipitation being displayed on the board by using an onboard switch (SW1) 
After a period of inactivity (1 minute), the board goes into a low power state, which disables all displays. This can be retracted by pressed Switch #2 (SW2)

If the user has a serial cable, they can also establish a UART connection to their software of choice (MobaXTerm, Putty, etc.) which displays detailed weather info for both types of data through UART. 
