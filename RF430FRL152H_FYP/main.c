/*
 * main.c
 *
 * RF430FRL152H NFC Only Example Project
 *
 * Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#include "NDEF.h"
#include "types.h"
#include "patch.h"
#include <rf430frl152h.h>

//*****************************FUNCTION PROTOTYPES********************************/
void DeviceInit(void);
//********************************************************************************/

/*********************** SUMMARY **************************************************************************************************
 * This project *only* utilizes the RF stack (ISO15693) on the ROM of the RF430FRL15xH. This setup allows the user to make a
 * custom application that is run from FRAM.  Only the RF13M vector that runs the RF stack needs to be pointing to its
 * ROM location.
 */

 /**************************************************************************************************************************************************
*   Code Space
***************************************************************************************************************************************************
*
*  Please check the lnk_rf430frl152h_NFC_Only.cmd file to customize how much code space is used for NDEF message ("FRAM_NDEF")
*  or for code ("FRAM_CODE").  
*
**************************************************************************************************************************************************/

/**************************************************************************************************************************************************
*   Main
***************************************************************************************************************************************************
*
* Brief :		Program entry.
*
* Param[in] :   None
*
*
* Param[out]:   None
*
* Return :
*
**************************************************************************************************************************************************/

void main()
{
	WDTCTL = WDTPW + WDTHOLD;                   // Stop watchdog

    // ROM RF13M module setup ** The following three lines are needed for proper RF stack operation
    DS = 1; 									// ROM variable needs to be initialized here
    asm ( " CALL #0x5CDA "); 					// Call ROM function ( Initialize function pointers)
    asm ( " CALL #0x5CAC "); 					// Call ROM function ( Check part configuration)


	initISO15693(CLEAR_BLOCK_LOCKS);  // clear all block locks
	//initISO15693(0);					// leave block locks as they are set in FRAM

	//JTAG is set to be disabled in this function call
	DeviceInit();
	
	// Configure Port Pins


         P1DIR  &= ~BIT0;                          // P1.0/TA0.1 Input Capture
         P1SEL0 |=  BIT0;                          // TA0.1 option select
         P1SEL1 |=  BIT0;                          // TA0.1 option select
         // Configure the TA0CCR1 to do input capture
         TA0CCTL1 = CAP + CM_1 + CCIE + CCIS_0;

                                                     // TA0CCR1 Capture mode; CCI1A; Both
                                                     // Rising and Falling Edge; interrupt enable

         TA0CTL |= TASSEL_2 + MC_2 + TACLR + TAIE;

         Count = 0;
	
	while(1)
	{
        // Enter application code here...
		__bis_SR_register(LPM3_bits + GIE);
		if (TA0CCTL1 & COV)
		         // Check for Capture Overflow
		         Period = REdge2 - REdge1;     // Calculate Period
			NFC_NDEF_Message[21] = {
			}; // Put in measurements
		
				
	}
}

/**************************************************************************************************************************************************
*  DeviceInit
***************************************************************************************************************************************************
*
* Brief : Initialize the clock system and other settings
*         Patchable function
*
* Param[in] :   parameters:  has these independent options
*                            INITIALIZE_DEVICE_CLOCK_SYSTEM - initializes the clock system
*                            POPULATE_INTERRUPT_VECTOR_IN_INITIALIZATION - populate the default interrupt vectors and recalculate their CRC
*
* Param[out]:  None
*
* Return  None
*
* Patchable :   Yes
**************************************************************************************************************************************************/
void DeviceInit(void)
{
//	P1SEL0 = 0xF0; //keep JTAG
//	P1SEL1 = 0xF0; //keep JTAG
	P1SEL0 = 0x00; //no JTAG
	P1SEL1 = 0x00; //no JTAG

    P1DIR &= ~0xEF;
    P1REN = 0;

    CCSCTL0 = CCSKEY;                        // Unlock CCS

    CCSCTL1 = 0;                             // do not half the clock speed
    CCSCTL4 = SELA_1 + SELM_0 + SELS_0;      // Select VLO for ACLK and select HFCLK/DCO for MCLK, and SMCLK
    CCSCTL5 = DIVA_2 + DIVM_1 + DIVS_1;      // Set the Dividers for ACLK (4), MCLK, and SMCLK to 1
    CCSCTL6 = XTOFF;                         // Turns of the crystal if it is not being used
    CCSCTL8 = ACLKREQEN + MCLKREQEN + SMCLKREQEN; //disable clocks if they are not being used

    CCSCTL0_H |= 0xFF;                       // Lock CCS

  return;
}



//#pragma vector = RFPMM_VECTOR
//__interrupt void RFPMM_ISR(void)
//{
//}
//
//#pragma vector = PORT1_VECTOR
//__interrupt void PORT1_ISR(void)
//{
//}
//
//#pragma vector = SD_ADC_VECTOR
//__interrupt void SD_ADC_ISR(void)
//{
//}
//
//#pragma vector = USCI_B0_VECTOR
//__interrupt void USCI_B0_ISR(void)
//{
//}
//#pragma CODE_SECTION(RF13M_ISR, ".fram_driver_code")  // comment this line for using ROM's RF13M ISR, uncomment next one, see .cmd file for details
#pragma CODE_SECTION(RF13M_ISR, ".rf13m_rom_isr") 		// comment this line for creating a custom RF13M ISR that will exist in FRAM, bypassing ROM's, uncomment previous
#pragma vector = RF13M_VECTOR
__interrupt void RF13M_ISR(void)
{
	// Right now this vector is pointing to the ROMs firmware location that runs the RF stack. 
    // Entering code here will, without changing the CODE_SECTION lines
	// above, will cause an error.
    // Changing the code section above will cause the ROM RF stack to be bypassed.  New handler will need to be created.
}

//#pragma vector = WDT_VECTOR
//__interrupt void WDT_ISR(void)
//{
//}
//
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TimerA1_ISR(void)
{
  switch(__even_in_range(TA0IV,0x0E))
  {
      case  TA0IV_TACCR1:                   // Vector  2:  TACCR1 CCIFG

          if (TA0CCTL1 & CCI){                // Capture Input Pin Status
            if (Count==0){                    // Rising Edge was captured
                REdge1 = TA0CCR1;
                Count++;
            }
            else{


                REdge2 = TA0CCR1;
                Count=0;
                TA1R=0;
                __bic_SR_register_on_exit(LPM0_bits + GIE);  // Exit LPM0 on return to main
            }

        }
        else{}
        break;
      case TA0IV_TACCR1: break;             // Vector  4:  TACCR2 CCIFG
      case TA0IV_TACCR2: break;             // Vector  4:  TACCR2 CCIFG
      default:  break;
  }
}

//#pragma vector = TIMER0_A0_VECTOR
//__interrupt void TimerA0_ISR(void)
//{
//}
//
//#pragma vector = UNMI_VECTOR
//__interrupt void UNMI_ISR(void)
//{
//}
//
//#pragma vector = SYSNMI_VECTOR
//__interrupt void SysNMI_ISR(void)
//{
//}

