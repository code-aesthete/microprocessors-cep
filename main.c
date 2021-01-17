/*
 * MPSI_CEP.c
 *
 * Created: 12/12/2020 03:00:00 AM
 * Authors: 1234, disturbedone
 */

// MPSI COMPLEX ENGINEERING PROBLEM
//
// GROUP MEMBERS
// Haseeb Ahmad		FA18-BEE-050-5A
// Farrukh Mumtaz	FA18-BEE-037-5A
// Abdullah Bin Tahir	FA18-BEE-009-5A

// INSTRUCTORS
// Sir Dr Irfan Ullah
// Ma'am Asma Ramay


#include <avr/io.h>
#include <avr/interrupt.h>
#define F_CPU 16000000UL // XTAL Frequency
#include <util/delay.h>


//      FOR PUSH BUTTONS

#define SWITCHED_OFF 0x02 // 0b00000010
#define START_OFF 0x04    // 0b00000100
#define MICRO 0x01        // 0b00000001
#define GRILL 0x02        // 0b00000010
#define TEMPERATURE 0x08  // 0b00001000


//        FOR LCD

#define RS PB0 //RS pin is connected to PORTD Pin 0
#define RW PB1 //RW pin is connected to PORTD Pin 1
#define EN PB2 //En pin is connected to PORTD Pin 2

int LCD_init(void);
void LCD_Send_Command(unsigned char); // send commands to LCD
void LCD_Send_Data(unsigned char);	// send data to LCD
void LCD_gotoxy(unsigned char,unsigned char); // set cursor position LCD
int LCD_Send_Array(char * ptr); // send string to LCD



//     FOR TIME DELAY

void oneSecondDelay(); // time delay of 1 second using Timer1 CTC mode prescaler of 1024


//	   CONFIGURATION

void portConfig(); // configuration of pins as input/ output
void extInterruptConfig(); // configuration of external interrupt


//    MICROWAVE MAIN OPERATION

void microwaveStart(int time); // function for microwave operation

//    FOR MICROWAVE PERIPHERALS

void buzzer(); // function to make the buzzer ring
void clockwMotorON(); // function to turn motor ON in clockwise direction
void clockwMotorOFF(); // function to turn motor OFF in clockwise direction
void anticlockwMotorON(); // function to turn motor ON in anticlockwise direction
void anticlockwMotorOFF(); // function to turn motor OFF in anticlockwise direction
void lightON(); // function to turn ON light
void lightOFF(); // function to turn OFF light
void emitterON(); // function to turn ON emitter
void emitterOFF(); // function to turn OFF emitter



int main(void)
{
    portConfig();
    extInterruptConfig();


    int counter = 0;  // for timer

    int temp = 110;	  // for temperature

    char buffer[25];
    LCD_init();
    LCD_gotoxy(1,1);
    LCD_Send_Array("Please Select: ");
    _delay_ms(500);
    LCD_init();
    LCD_Send_Array("Micro or Grill");
    oneSecondDelay();
    while (1)
    {
        oneSecondDelay();
        if ((PIND & MICRO))
        {
            LCD_init();
            LCD_Send_Array("Micro selected");
            oneSecondDelay();
            LCD_init();
            LCD_gotoxy(1,1);
            LCD_Send_Array("Enter");
            LCD_gotoxy(1,2);
            LCD_Send_Array("Temperature");
            oneSecondDelay();
            sprintf(buffer, "Temp %d oC", temp);  // this will show default temperature, which is 110 oC
            LCD_init();
            LCD_Send_Array(buffer);
            oneSecondDelay();
            while (1)
            {

                if((PIND & TEMPERATURE))
                {
                    temp += 20;
                    if(temp > 240)
                        temp = 110;
                    char buffer[25];
                    sprintf(buffer, "Temp %d oC", temp);
                    LCD_init();
                    LCD_Send_Array(buffer);
                    oneSecondDelay();
                }
                else if (!(PINC & SWITCHED_OFF ))
                {
                    break;
                }
                else
                {
                    LCD_gotoxy(1,2);
                    LCD_Send_Array("Press Timer/Temp");
                }
            }
            _delay_ms(250);
            break;
        }
        else if((PIND & GRILL))
        {
            LCD_init();
            LCD_gotoxy(1,1);
            LCD_Send_Array("Grill selected");
            oneSecondDelay();
            LCD_init();
            LCD_gotoxy(1,1);
            LCD_Send_Array("Enter");
            LCD_gotoxy(1,2);
            LCD_Send_Array("Temperature:");
            oneSecondDelay();
            sprintf(buffer, "Temp %d oC", temp);  // this will show default temperature, which is 110 oC
            LCD_init();
            LCD_Send_Array(buffer);
            oneSecondDelay();
            while (1)
            {

                if((PIND & TEMPERATURE))
                {
                    temp += 20;
                    if(temp > 240)
                        temp = 110;

                    char buffer[25];
                    sprintf(buffer, "Temp %d oC", temp);
                    LCD_init();
                    LCD_Send_Array(buffer);
                    oneSecondDelay();
                }
                else if (!(PINC & SWITCHED_OFF ))
                {
                    break; // if timer button is pressed, jump to that portion of code
                }
                else
                {
                    LCD_gotoxy(1,2);
                    LCD_Send_Array("Press Timer/Temp");
                }
            }
            _delay_ms(250);
            break;
        }
    }


    while(1)
    {
        // we have used some push buttons with pull-up resistors enabled
        // in this case, when the button is not pressed, the logic will be HIGH or 1
        // and when the button is pressed, the logic will be LOW or 0
        // so, the functionality is INVERTED for these buttons
        // therefore, if these buttons give a logic of 0, they are actually on.

        // buttons used with pull ups enabled are:
        // START TIMER PAUSE

        // button used with normal behavior (pull down mode) are:
        // TEMPERATURE MICROWAVE MODE GRILL MODE

        // if (not switched off) send 1 ~ if switch is pressed send 1

        if(!(PINC & SWITCHED_OFF))
        {
            counter += 30;

            if(counter > 240)
                counter = 0;

            char buffer[25];
            sprintf(buffer, "Time: %d sec", counter);
            LCD_init();
            LCD_Send_Array(buffer);
            oneSecondDelay(); // 1 second delay


            if(!(PINC & START_OFF))
            {
                microwaveStart(counter);
            }
            else // start button not pressed
            {
                LCD_init();
                LCD_Send_Array("Press Timer");

            }


        }

    }
}


void portConfig()
{
    // this function configures the ports on atmega32 for input/ output functionality
    // on port c, we have light, emitter, buzzer, turn table set as outputs
    // we also have 2 push buttons on port c for which we enable the internal pull up resistors
    // port d will be set as input in extInterruptConfig function for the push buttons

    DDRC |= 0x09; // buzzer and light as output

    PORTC |= (1<<PC1); // setting up internal resistor of pin 1 (Timer) button

    PORTC |= (1<<PC2); // setting up internal resistor of pin 2 (Start) button

    DDRC |= (1<<PC4); // motor clockwise as output
    DDRC |= (1<<PC5); // motor anticlockwise as output

    DDRB |= 0b00000111; // set pb0 pb1 pb2 to output

}

void extInterruptConfig()
{
    // this function configures the external interrupts
    // we use INT0 on port d in rising edge trigger mode
    // sei function used to enable global interrupt


    DDRD = 0;			// PORTD as input
    PORTD = 0xFF;		// Make pull up high

    GICR = 1<<INT0;		       // Enable INT0
    MCUCR = 1<<ISC01 | 1<<ISC00;  // Trigger INT0 on rising edge

    sei();			// Enable Global Interrupt

}



int LCD_init()
{
    // this function will initalized the LCD, it selects the mode (8 bit/ 4 bit),
    // visiblity of cursor, display shift, backlight etc

    DDRA = 0xFF; // set port a to output

    DDRB |= 0b00000111; // set pb0 pb1 pb2 to output

    _delay_ms(100);

    PORTA |= (1 << EN); // keep en high while preserving all other bits

    LCD_Send_Command(0x38); // LCD mode 8-bit, 2 line
    _delay_ms(2);

    LCD_Send_Command(0xC); // display on, cursor off
    _delay_ms(2);

    LCD_Send_Command(0x01); // clear display
    _delay_ms(2);

    LCD_Send_Command(0x16); // increment cursor, no display shift
    _delay_ms(2);

    return (0);
}


void LCD_Send_Command(unsigned char comm)
{
    // this function will configure the LCD when we want to send a command
    // first the LCD is configured properly for command mode
    // and then command is sent on port a
    // this function is utilized in LCD_init function

    PORTB &= ~(1 << RS); // clear bit RS
    PORTB &= ~(1 << RW); // clear bit RW
    PORTA = comm; // send command to port B
    PORTB &= ~(1 << EN); // low to high
    _delay_ms(1);
    PORTB |= (1 << EN);
}


void LCD_Send_Data(unsigned char data)
{
    // this function will configure the LCD when we want to send data
    // first the LCD is configured properly for data mode
    // and then data (string specified by us) is displayed on LCD

    PORTB |= (1 << RS); // RS high for data

    PORTB &= ~(1 << RW); // RW low for writing

    PORTA = data; // send data on port a

    //  send a short pulse on EN
    PORTB &= ~(1 << EN); // low to high
    _delay_ms(2);
    PORTB |= (1 << EN);

}


void LCD_gotoxy(unsigned char x, unsigned char y)
{
    // this function will be used to define the position of cursor on LCD
    // the first argument (x) defines the position of cursor horizontally for example
    // if x = 7, the cursor will be at position 7 from horizontal axis
    // the second argument (y) defines the line number,
    // in our case we have a 16x2 LCD, so line number can be either 1 or 2

    unsigned char firstCharAdr[] = {0x80, 0xC0, 0x94, 0xD4}; // from book table 12. 4 //program 12.5
    LCD_Send_Command(firstCharAdr[y-1] + x-1);
    _delay_us(100);
}

int LCD_Send_Array(char * ptr)
{
    // this function will send a string on LCD display

    while ( * ptr != '\0')

    {
        LCD_Send_Data( * ptr);

        ptr++;
    }
}


//		INTERRUPT SERVICE ROUTINE FOR PAUSE

ISR(INT0_vect)
{
    // 	when an external interrupt occurs, program will jump here and
    // 	-> all devices will be stopped (by sending 0 to that port)
    // 	-> LCD will display a pause message
    //	-> program will go in an infinite while loop to wait for exit condition (Start/ Stop button)
    // 	-> program will wait for user to press the Start button
    //
    // 	when the Start button is pressed, normal functionality is resumed
    //
    // 	if Stop button is pressed, microwave will reset

    lightOFF();
    anticlockwMotorOFF();
    clockwMotorOFF();
    emitterOFF();

    LCD_init();
    LCD_gotoxy(6,1);
    LCD_Send_Array("PAUSED!");
    LCD_gotoxy(1,2);
    LCD_Send_Array("Press Start/Stop");

    while(1)

    {
        if(!(PINC & START_OFF))  // start  = pin no. at which start button is connected
            break;
    }
}

void oneSecondDelay()
{
    // this function will give a delay of 1 second
    // time 1
    // CTC Mode
    // prescaler of 1024

    OCR1AH = 0x3D;
    OCR1AL = 0x08;

    TCNT1H = 0;
    TCNT1L = 0;

    TCCR1A = 0;
    TCCR1B = 0b00001101; // CTC mode prescaler 1024

    while((TIFR&(1<<OCF1A))==0); // while loop will terminate after one second

    TIFR = 1 << OCF1A; // clear ocf1a bit

    TCCR1A = 0;
    TCCR1B = 0;

}

void microwaveStart(int time)
{

// 	argument time = number of seconds withing the range 30-240 specified by user
//
// 	this function will start the microwave, turn on the required devices (light, emitter, motor)
// 	for proper distribution of heat to food we want the motor to rotate for 2 cycles in clockwise direction
// 	and for 1 cycle in anticlockwise direction, for this functionality we initialize a counter (motor_rotation_counter)
// 	that decides the rotation direction of motor. if the counter is less than 6, motor will rotate in clockwise direction
// 	, if it is less than 9, rotation will be in anticlockwise direction. Counter will be re-initialized to zero once its
// 	value becomes greater than 9
//
// 	once the microwave runs for the specified amount of seconds, the devices will be turned off and buzzer will ring for a short time
//
// 	special cases:
//
// 		-> if there is an external interrupt on INT0, the program control will jump to ISR and all the devices and microwave will be paused
// 		until the interrupt is ended by pressing the Start button
//
// 		-> if stop button is pressed, microwave will reset
//
    int i;
    int motor_rotation_counter = 0;

    for(i = time; i > 0; i--)
    {
        lightON();
        emitterON();

        if(motor_rotation_counter < 6)
        {
            clockwMotorON();
            oneSecondDelay();


            LCD_init();
            char buffer[25];
            sprintf(buffer, "Remaining:%d sec", i);
            LCD_gotoxy(7,1);
            LCD_Send_Array("Busy!");
            LCD_gotoxy(1,2);
            LCD_Send_Array(buffer);

            motor_rotation_counter++;
        }
        else
        {
            clockwMotorOFF();
            anticlockwMotorON();
            oneSecondDelay();

            LCD_init();
            char buffer[25];
            sprintf(buffer, "Remaining:%d sec", i);
            LCD_gotoxy(7,1);
            LCD_Send_Array("Busy!");
            LCD_gotoxy(1,2);
            LCD_Send_Array(buffer);

            motor_rotation_counter++;
        }
        if(motor_rotation_counter > 9)
        {
            anticlockwMotorOFF();
            motor_rotation_counter = 0;
        }


    }

    lightOFF();
    emitterOFF();
    clockwMotorOFF();
    anticlockwMotorOFF();
    LCD_init();
    LCD_Send_Array("Ready!");
    buzzer();
}

void clockwMotorON()
{
    /*    sending 1 on port c pin 4 to turn on motor rotation in clockwise direction*/
    PORTC |= (1<<PC4);
}

void clockwMotorOFF()
{
    /*    sending 0 on port c pin 4 to turn off motor rotation in clockwise direction*/
    PORTC &= ~(1<<PC4);
}

void anticlockwMotorON()
{
    /*    sending 1 on port c pin 5 to turn on motor rotation in anticlockwise direction*/
    PORTC |= (1<<PC5);
}

void anticlockwMotorOFF()
{
    /*    sending 0 on port c pin 5 to turn off motor rotation in anticlockwise direction*/
    PORTC &= ~(1<<PC5);
}

void lightON()
{
    /*    sending 1 on port c pin 6 to turn on light*/
    PORTC |= (1<<PC0);
}

void lightOFF()
{
    /*    sending 0 on port c pin 0 to turn off light*/
    PORTC &= ~(1<<PC0);
}

void emitterON()
{
    /*    sending 1 on port c pin 6 to turn on emitter*/
    PORTC |= (1<<PC6);
}

void emitterOFF()
{
    /*    sending 0 on port c pin 6 to turn off emitter*/
    PORTC &= ~(1<<PC6);
}

void buzzer()
{
    // 	when microwave finishes it operation, buzzer will ring for a short interval of time and then turned off
    // 	it is turned on by sending 1 on port c pin 3
    // 	if it turned off by sending 0 on port c pin 3

    PORTC |= (1<<PC3);
    _delay_ms(250);
    PORTC &= ~(1<<PC3); // using masks for preserving other bits
    PORTC |= (1<<PC3);
    _delay_ms(250);
    PORTC &= ~(1<<PC3); // using masks for preserving other bits
    PORTC |= (1<<PC3);
    _delay_ms(250);
    PORTC &= ~(1<<PC3); // using masks for preserving other bits

}
