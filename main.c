
#include <msp430.h> 
#include <stdint.h>

#include "nrf24.h"
#include "rsa.h"

char dely[16];
int b = 0;

#define MOSI BIT0  //   P2.0
#define MISO BIT1  //   P2.1
#define SCLK BIT4  //   P2.4
#define CE BIT5    //   P2.5
#define CSN BIT3   //   P2.3
int i;
int j;
int k;
int x;
int pd; //payload func index 0-15
int pd_i;  //payload func i index 0-7
int pd_x;

unsigned char status_reg;
unsigned char read_reg[5];

unsigned char read_reg_CONFIG[1];
unsigned char read_reg_EN_AA[1];
unsigned char read_reg_EN_RXADDR[1];
unsigned char read_reg_SETUP_AW[1];
unsigned char read_reg_SETUP_RETR[1];
unsigned char read_reg_RF_CH[1];
unsigned char read_reg_RF_SETUP[1];
unsigned char read_reg_STATUS[1];
unsigned char read_reg_OBSERVE_TX[1];
unsigned char read_reg_CD[1];
unsigned char read_reg_RX_ADDR_P0[5];  //5 BYTES
unsigned char read_reg_RX_ADDR_P1[5];  //5 BYTES
unsigned char read_reg_RX_ADDR_P2[1];
unsigned char read_reg_RX_ADDR_P3[1];
unsigned char read_reg_RX_ADDR_P4[1];
unsigned char read_reg_RX_ADDR_P5[1];
unsigned char read_reg_TX_ADDR[5];  //5 BYTES
unsigned char read_reg_RX_PW_P0[1];
unsigned char read_reg_RX_PW_P1[1];
unsigned char read_reg_RX_PW_P2[1];
unsigned char read_reg_RX_PW_P3[1];
unsigned char read_reg_RX_PW_P4[1];
unsigned char read_reg_RX_PW_P5[1];
unsigned char read_reg_FIFO_STATUS[1];

unsigned char clr_status[1]={0x70};  

unsigned char rf_setupregister[1]={0b00000001};    
unsigned char configregister[1]={0b00001110};  
unsigned char rf_chanregister[1]={0b01001100};   
unsigned char address[6]="00001";   
unsigned char setup_retr_register[1]={0b01011111};   
unsigned char en_aa_register[1]={0b00111111};
unsigned char rx_pw_register[1]={0b00100000};   

void SCLK_Pulse (void);   
void Send_Bit (unsigned int value);      
void CE_On (void);  //Chip enable
void CE_Off (void);  //Chip disable
void CSN_On (void);     //CSN On
void CSN_Off (void);    //CSN Off
void Write_Byte (int content);
void Instruction_Byte_MSB_First (int content);
void Read_Byte_MSB_First(int index, unsigned char regname[]);
void Write_Byte_MSB_First(unsigned char content[], int index2);
void Write_Payload_MSB_First(int pyld[], int index3);
int musa = 9;
int payintt = 7;
int payload[8] = {-31,-99,-75,-56,-12,-60,60,60};
int aratemp;

void main(void)
{
    _enable_interrupt();
    _BIS_SR(GIE);
    __bis_SR_register(GIE);
    P1SEL = BIT1 | BIT2 ;
    P1SEL2 = BIT1 | BIT2;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    /* Place UCA0 in Reset to be configured */
    UCA0CTL1 = UCSWRST;

    /* Configure */
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 104; // 1MHz 9600
    UCA0BR1 = 0; // 1MHz 9600
    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1

    /* Take UCA0 out of reset */
    UCA0CTL1 &= ~UCSWRST;

    /* Enable USCI_A0 RX interrupt */
    IE2 |= UCA0RXIE;

    __delay_cycles(100); //power on reset

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    //P2DIR &= 0x00 ;
    P2OUT &= 0x00;
    P2DIR |= MOSI + SCLK + CE + CSN ;  //Output Pins
    P2DIR &= ~MISO;

    CE_Off();
    CSN_On();
    /************************
     **CONFIGURING REGISTERS**
     *************************/
    //EN_AA  -- enabling AA in all pipes

    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | EN_AA);
    Write_Byte_MSB_First(en_aa_register,1);
    CSN_On();
    //RF_SETUP
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RF_SETUP);
    Write_Byte_MSB_First(rf_setupregister,1);
    CSN_On();
    //RX_ADDR_P0
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_ADDR_P0);
    Write_Byte_MSB_First(address,5); // write 5 bytes address
    CSN_On();
    //TX_ADDR
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | TX_ADDR);
    Write_Byte_MSB_First(address,5); // write 5 bytes address
    CSN_On();
    //RF_CH
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RF_CH);
    Write_Byte_MSB_First(rf_chanregister,1);
    CSN_On();
    //SETUP_RETR
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | SETUP_RETR);
    Write_Byte_MSB_First(setup_retr_register,1);
    CSN_On();
    //RX_PW0
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P0);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW1
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P1);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW2
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P2);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW3
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P3);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW4
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P4);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //RX_PW4
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P5);
    Write_Byte_MSB_First(rx_pw_register,1);
    CSN_On();
    //CONFIG
    CSN_Off();
    Instruction_Byte_MSB_First(W_REGISTER | CONFIG);
    Write_Byte_MSB_First(configregister,1);
    CSN_On();
    /****************************
     **END CONFIGURING REGISTERS**
     *****************************/
    __delay_cycles(2000);  //start_up 1.5 ms
    while(1){
        lettertonumber();
        for(musa; musa > 0 ; musa--){
            aratemp = payload[payintt];
            encrypt(aratemp);
            payintt--;
        }
        run();
        __delay_cycles(5000);

        //        //STDBY-I
        //        CSN_Off();
        //        Instruction_Byte_MSB_First(W_TX_PAYLOAD);
        //        Write_Payload_MSB_First(payload,8);
        //        Write_Payload_MSB_First(payload,8);
        //        CSN_On();
        //        CE_On();
        //        __delay_cycles(50); //min pulse >10usec
        //        CE_Off();
        //        //TX settling 130 usec
        //        __delay_cycles(150);
        //        //TX MODE
        //
        //        __delay_cycles(20000);
        //        //STDBY-I
        //        CSN_Off();
        //        Instruction_Byte_MSB_First(NOP);
        //        CSN_On();
        //        if ((status_reg & BIT4) == 0x10){
        //                CSN_Off();
        //                Instruction_Byte_MSB_First(W_REGISTER | STATUS);
        //                Write_Byte_MSB_First(clr_status,1);
        //                CSN_On();
        //                CSN_Off();
        //                Instruction_Byte_MSB_First(FLUSH_TX);
        //                CSN_On();
        //
        //        }

    }
}
void SCLK_Pulse (void)
{
    P2OUT |= SCLK;//set high with OR 1
    P2OUT ^= SCLK;//toggle with XOR 1
}
void Send_Bit (unsigned int value)
{
    if (value != 0){
        P2OUT |= MOSI;}
    else {
        P2OUT &= ~MOSI;
    }
}
void CE_On (void)
{
    P2OUT |= CE;
}

void CE_Off (void)
{
    P2OUT &= ~CE;
}
void CSN_On (void)
{
    P2OUT |= CSN;
}
void CSN_Off (void)
{
    P2OUT &= ~CSN;
}
void Write_Byte(int content)  //Not ued in this application
{

    for (j=0;j<8;j++){
        x = (content & (1 << j));  //Write to Address
        Send_Bit(x);
        SCLK_Pulse();
    }
}
void Instruction_Byte_MSB_First(int content)
{

    for (k=7;k>=0;--k){
        x = (content & (1 << k));  //Write to Address
        status_reg <<= 1;
        Send_Bit(x);

        if ((P2IN & MISO) == 0x02){


            status_reg |= 0b00000001;
        }
        else {


            status_reg  &= 0b11111110;
        }

        SCLK_Pulse();

    }

}
void Read_Byte_MSB_First(int index, unsigned char regname[])
{
    for (i=0;i<=(index-1);i++){
        for (k=0;k<8;k++){
            regname[i] <<= 1;


            if ((P2IN & MISO) == 0x02){

                //read_reg |= 0b10000000;
                regname[i] |= 0b00000001;
            }
            else {

                //read_reg  &= 0b01111111;
                regname[i]  &= 0b11111110;
            }
            SCLK_Pulse();
        }
    }
}
void Write_Byte_MSB_First(unsigned char content[], int index2)
{
    for (i=0;i<=(index2-1);i++){
        for (k=7;k>=0;--k){

            x = (content[i] & (1 << k));  //Write to Address
            Send_Bit(x);
            SCLK_Pulse();

        }

    }
}

void Write_Payload_MSB_First(int pyld[], int index3)
{
    for (pd_i=0;pd_i<=(index3-1);pd_i++){
        for (pd=7;pd>=0;--pd){

            pd_x = (pyld[pd_i] & (1 << pd));  //Write to Address
            Send_Bit(pd_x);
            SCLK_Pulse();

        }
        for (pd=15;pd>=8;--pd){

            pd_x = (pyld[pd_i] & (1 << pd));  //Write to Address
            Send_Bit(pd_x);
            SCLK_Pulse();

        }

    }
}


void run(){
    P2DIR &= 0x00 ;
       P2OUT &= 0x00;
       P2DIR |= MOSI + SCLK + CE + CSN ;  //Output Pins
       P2DIR &= ~MISO;

       CE_Off();
       CSN_On();
       /************************
       **CONFIGURING REGISTERS**
       *************************/
       //EN_AA  -- enabling AA in all pipes

       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | EN_AA);
       Write_Byte_MSB_First(en_aa_register,1);
       CSN_On();
       //RF_SETUP
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RF_SETUP);
       Write_Byte_MSB_First(rf_setupregister,1);
       CSN_On();
       //RX_ADDR_P0
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RX_ADDR_P0);
       Write_Byte_MSB_First(address,5); // write 5 bytes address
       CSN_On();
       //TX_ADDR
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | TX_ADDR);
       Write_Byte_MSB_First(address,5); // write 5 bytes address
       CSN_On();
       //RF_CH
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RF_CH);
       Write_Byte_MSB_First(rf_chanregister,1);
       CSN_On();
       //SETUP_RETR
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | SETUP_RETR);
       Write_Byte_MSB_First(setup_retr_register,1);
       CSN_On();
       //RX_PW0
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P0);
       Write_Byte_MSB_First(rx_pw_register,1);
       CSN_On();
       //RX_PW1
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P1);
       Write_Byte_MSB_First(rx_pw_register,1);
       CSN_On();
       //RX_PW2
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P2);
       Write_Byte_MSB_First(rx_pw_register,1);
       CSN_On();
       //RX_PW3
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P3);
       Write_Byte_MSB_First(rx_pw_register,1);
       CSN_On();
       //RX_PW4
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P4);
       Write_Byte_MSB_First(rx_pw_register,1);
       CSN_On();
       //RX_PW4
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | RX_PW_P5);
       Write_Byte_MSB_First(rx_pw_register,1);
       CSN_On();
       //CONFIG
       CSN_Off();
       Instruction_Byte_MSB_First(W_REGISTER | CONFIG);
       Write_Byte_MSB_First(configregister,1);
       CSN_On();
    /////////////////////////
    P2DIR &= 0x00 ;
    P2OUT &= 0x00;
    P2DIR |= MOSI + SCLK + CE + CSN ;  //Output Pins
    P2DIR &= ~MISO;

    CE_Off();
    CSN_On();

    //STDBY-I
    CSN_Off();
    Instruction_Byte_MSB_First(W_TX_PAYLOAD);
    Write_Payload_MSB_First(payload,8);
    Write_Payload_MSB_First(payload,8);
    CSN_On();
    CE_On();
    __delay_cycles(50); //min pulse >10usec
    CE_Off();
    //TX settling 130 usec
    __delay_cycles(150);
    //TX MODE

    __delay_cycles(20000);
    //STDBY-I
    CSN_Off();
    Instruction_Byte_MSB_First(NOP);
    CSN_On();
    if ((status_reg & BIT4) == 0x10){
        CSN_Off();
        Instruction_Byte_MSB_First(W_REGISTER | STATUS);
        Write_Byte_MSB_First(clr_status,1);
        CSN_On();
        CSN_Off();
        Instruction_Byte_MSB_First(FLUSH_TX);
        CSN_On();

    }
    P1SEL = BIT1 | BIT2 ;
    P1SEL2 = BIT1 | BIT2;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    /* Place UCA0 in Reset to be configured */
    UCA0CTL1 = UCSWRST;

    /* Configure */
    UCA0CTL1 |= UCSSEL_2; // SMCLK
    UCA0BR0 = 104; // 1MHz 9600
    UCA0BR1 = 0; // 1MHz 9600
    UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1

    /* Take UCA0 out of reset */
    UCA0CTL1 &= ~UCSWRST;
///////////////////////////////////////
    P1SEL = BIT1 | BIT2 ;
       P1SEL2 = BIT1 | BIT2;
       DCOCTL = 0;
       BCSCTL1 = CALBC1_1MHZ;
       DCOCTL = CALDCO_1MHZ;
       /* Place UCA0 in Reset to be configured */
       UCA0CTL1 = UCSWRST;

       /* Configure */
       UCA0CTL1 |= UCSSEL_2; // SMCLK
       UCA0BR0 = 104; // 1MHz 9600
       UCA0BR1 = 0; // 1MHz 9600
       UCA0MCTL = UCBRS0; // Modulation UCBRSx = 1

       /* Take UCA0 out of reset */
       UCA0CTL1 &= ~UCSWRST;

       /* Enable USCI_A0 RX interrupt */
       IE2 |= UCA0RXIE;

       __delay_cycles(100); //power on reset

       WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
       //P2DIR &= 0x00 ;
       P2OUT &= 0x00;
       P2DIR |= MOSI + SCLK + CE + CSN ;  //Output Pins
       P2DIR &= ~MISO;

       CE_Off();
       CSN_On();
}
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{

    while(b<8){
        dely[b] = UCA0RXBUF;
        b++;
    }
    b=0;
    IFG2 &= ~UCA0RXIFG;
}


void lettertonumber(void){
    char letterr = dely[1];
    int rempo = 7;
    switch (letterr){

    case 'a':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 0;
            rempo--;
        }
        break;
    case 'b':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 10;
            rempo--;
        }
        break;
    case 'c':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 20;
            rempo--;
        }
        break;
    case 'd':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 30;
            rempo--;
        }
        break;
    case 'e':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 40;
            rempo--;
        }
        break;
    case 'f':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 50;
            rempo--;
        }
        break;
    case 'g':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 60;
            rempo--;
        }
        break;
    case 'h':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 70;
            rempo--;
        }
        break;
    case 'i':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 80;
            rempo--;
        }
        break;
    case 'j':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 90;
            rempo--;
        }
        break;
    case 'k':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 100;
            rempo--;
        }
        break;
    case 'l':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 110;
            rempo--;
        }
        break;
    case 'm':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] =120;
            rempo--;
        }
        break;
    case 'n':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 130;
            rempo--;
        }
        break;
    case 'o':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 140;
            rempo--;
        }
        break;
    case 'p':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = 150;
            rempo--;
        }
        break;
    case 'A':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -10;
            rempo--;
        }
        break;
    case 'B':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -20;
            rempo--;
        }
        break;
    case 'C':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -30;
            rempo--;
        }
        break;
    case 'D':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -40;
            rempo--;
        }
        break;
    case 'E':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -50;
            rempo--;
        }
        break;
    case 'F':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -60;
            rempo--;
        }
        break;
    case 'G':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -70;
            rempo--;
        }
        break;
    case 'H':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -80;
            rempo--;
        }
        break;
    case 'I':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -90;
            rempo--;
        }
        break;
    case 'J':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -100;
            rempo--;
        }
        break;
    case 'K':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -110;
            rempo--;
        }
        break;
    case 'L':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -120;
            rempo--;
        }
        break;
    case 'M':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -130;
            rempo--;
        }
        break;
    case 'N':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -140;
            rempo--;
        }
        break;
    case 'O':
        rempo = 7;
        while(rempo >= 0){
            payload[rempo] = -150;
            rempo--;
        }
        break;

    }
}
