#include <xc.h>
#include <pic18f4520.h>

#pragma config OSC = INTIO67 // Oscillator Selection bits
#pragma config WDT = OFF     // Watchdog Timer Enable bit 
#pragma config PWRT = OFF    // Power-up Enable bit
#pragma config BOREN = ON   // Brown-out Reset Enable bit
#pragma config PBADEN = OFF     // Watchdog Timer Enable bit 
#pragma config LVP = OFF     // Low Voltage (single -supply) In-Circute Serial Pragramming Enable bit
#pragma config CPD = OFF     // Data EEPROM?Memory Code Protection bit (Data EEPROM code protection off)

#define _XTAL_FREQ 200000

int door = 0; // 0:close; 1:open
int full_close = 0;

void __interrupt(low_priority) ISRlow(void)
{
    PORTDbits.RD0 = 1;
    
            if(CCPR1L == 0b00001101 && CCP1CONbits.DC1B == 0b11)
            {
                full_close = 0;
                while (1)
                {   
                    if(CCPR1L == 0b00000011 && CCP1CONbits.DC1B == 0b11)
                        break;
                    if(CCP1CONbits.DC1B == 0b11) 
                        CCPR1L--;
                    CCP1CONbits.DC1B--;
                    __delay_ms(30);
                }
//            door = 1;
            }
        INTCON3bits.INT1IF = 0b0;
        return; 
}

void __interrupt(high_priority) ISR(void)
{   
        if(full_close == 0){
            PORTDbits.RD0 = 1;
            
            while (1) //open door after detecting object()
            {   
                if(CCPR1L == 0b00000011 && CCP1CONbits.DC1B == 0b11)
                    break;
                if(CCP1CONbits.DC1B == 0b11) 
                    CCPR1L--;
                CCP1CONbits.DC1B--;
                __delay_ms(30);
            }
            door = 1;
        }
        INTCONbits.INT0IF = 0b0;
    
    return;   
}

void main(void) {
    // interrupt ?? -> ?? RB0 ? interrupt service routine
    RCONbits.IPEN = 0b1;
    INTCONbits.GIEH = 0b1;
    INTCONbits.GIEL = 0b1;   
    
    INTCONbits.INT0IE = 0b1;
    INTCONbits.INT0IF = 0b0;
    
    // RB1
    INTCON3bits.INT1IP = 0;
    INTCON3bits.INT1IE = 0b1;
    INTCON3bits.INT1IF = 0b0;
    
    IPR1bits.TMR1IP = 0;
    PIE1bits.TMR1IE = 1;

    // Timer2 -> On, prescaler -> 4
    T2CON = 0b01111101;
    // Internal Oscillator Frequency, Fosc = 125 kHz, Tosc = 8 탎
    OSCCONbits.IRCF = 0b001;  
    // PWM mode, P1A, P1C active-high; P1B, P1D active-high
    CCP1CONbits.CCP1M = 0b1100;
    // CCP1/RC2 -> Output
    TRISC = 0;
    LATC = 0;
    // RB0 ?????
    TRISB=0xFF;
    LATB=0xFF;
    // LATD = 0;
    TRISDbits.RD0 = 0;
    TRISDbits.RD2 = 1;
    
 
    //PWM period = (PR2+1)*4*Tosc*(TMR2 prescaler) = (0x9b + 1) * 4 * 8탎 * 4 = 0.019968s ~= 20ms
    PR2 = 0x9b;
    /**
     * Duty cycle
     * = (CCPR1L:CCP1CON<5:4>) * Tosc * (TMR2 prescaler)
     * = (0x0b*4 + 0b01) * 8탎 * 4
     * = 0.00144s ~= 1450탎
     */
    // -90 degree
    CCPR1L = 0b00000011;
    CCP1CONbits.DC1B = 0b11;

//    while(1);
    while(1){
        PORTDbits.RD0 = 0;
        
         if(PORTDbits.RD2 == 0){
            if(CCPR1L == 0b00001101 && CCP1CONbits.DC1B == 0b11){   //close -> open
                INTCONbits.INT0IF = 0b0;
                full_close = 0;
                while(1){
                    if(CCP1CONbits.DC1B == 0b11) 
                        CCPR1L--;
                    CCP1CONbits.DC1B--;
                    __delay_ms(30);
                    if(CCPR1L == 0b00000011 && CCP1CONbits.DC1B == 0b11)
                        break;
                }
            }
            else if(CCPR1L == 0b0000011 && CCP1CONbits.DC1B == 0b11){ //open->close
                while(1){
                    if(door == 1)
                        break;
                    if(CCP1CONbits.DC1B == 0b11) 
                        CCPR1L++;
                    CCP1CONbits.DC1B++;
                    __delay_ms(30);
                    if(CCPR1L == 0b00001101 && CCP1CONbits.DC1B == 0b11){
                        full_close = 1;
                        break;
                    }
                }
            }
            PORTDbits.RD0 = 0;
            door = 0;
        }
    }
    return;
}

// duty cycle = (CCPR1L:CCP1CON<5:4>)*Tosc*(TMR2 prescaler) = (0x0b*4 + 0b01) * 8탎 * 4 = 0.00144s ~= 1450탎
//-90 degree -> 500 탎 -> ~15 -> 0b00_0000_1111
// CCPR1L = 0b0000_0011;
// CCP1CONbits.DC1B = 0b11;

//0 degree -> 1450 탎 -> ~45 -> 0b00_0010_1101
// CCPR1L = 0b0000_1011;
// CCP1CONbits.DC1B = 0b01;

//90 degree -> 2400 탎  -> ~75 -> 0b00_0100_1011
// CCPR1L = 0b0001_0010;
// CCP1CONbits.DC1B = 0b11;

