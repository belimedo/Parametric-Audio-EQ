/********************************************************************************************
**
**  File:   initPLL.c
**  Date:   12-15-2011
**  Author: MM
**  Use:    Initialize the DSP PLL for the required CCLK and HCLK rates.
**  Note:   CLKIN will be 25 MHz from an external oscillator.  The PLL is programmed
**          to generate a core clock (CCLK) of 400 MHz and SDCLK of 160MHz
**
**
*********************************************************************************************/


#include <platform_include.h>      /* System and IOP register bit and address definitions. */
#include <processor_include.h>
#include <builtins.h>  /* Get definitions of compiler builtin functions */


void initPLL(){

/********************************************************************************************/

  int temp,i;

 //Step 1 - change the PLLD to 4
  temp=*pPMCTL;
  temp&=~PLLD16 ;
  temp|=(PLLD4 | DIVEN) ;
  *pPMCTL = temp;

 //Step 2 - wait for dividers to stabilize
 for(i=0;i<16;i++);

 //Step 3 - set INDIV bit now to bring down the VCO speed and enter the bypass mode
 temp&=~DIVEN;
 temp|=(INDIV | PLLBP);
 *pPMCTL = temp;

 //Step 4 - wait for the PLL to lock
 for(i=0;i<4096;i++);

 //Step 5 - come out of the bypass mode
 temp=*pPMCTL;
 temp&=~PLLBP;
 *pPMCTL = temp;

 //Step 6 - wait for dividers to stabilize
 for(i=0;i<16;i++);

 //Step 7 - set the required PLLM and INDIV values here  and enter the bypass mode
//PLLM=16, INDIV=0,  fVCO=2*PLLM*CLKIN = 2*16*25 = 800 MHz
 temp = *pPMCTL;
 temp&=~ (INDIV | PLLM63);
 temp|= (PLLM16| PLLBP);
 *pPMCTL = temp;

 //Step 8 - wait for the PLL to lock
 for(i=0;i<4096;i++);

 //Step 9 - come out of the bypass mode
 temp = *pPMCTL;
 temp&=~PLLBP;
 *pPMCTL=temp;

 //Step 10 - wait for dividers to stabilize
 for(i=0;i<16;i++);

 //Step 11 - set the required values of PLLD(=2) and SDCKR (=2.5) here
// fCCLK = fVCO/PLLD = 800/2 = 400 MHz, fSDCLK = fCCLK/SDCKR = 400/2.5 = 160 MHz
 temp=*pPMCTL;
 temp&=~(PLLD16 | 0x1C0000 );
 temp|= (SDCKR2_5 | PLLD2 | DIVEN);
 *pPMCTL=temp;

 //Step 12 - wait for the dividers to stabilize
 for(i=0;i<16;i++);
}

void initExternalMemory(void)
{

// SDRAM memory on EZ-Board - MT48LC16M16A2-6A
// 5.4ns @ CL = 3 ,167 MHz speed
// Parameters
// Config - 64M x 16(16M x 16 x 4)
// Speed -        167 MHz
// CAS Latency -   3
// Row addressing - 8K(A0-A12)
// Column addressing - 512(A0-A8)
// No of Banks - 4

//tRAS - 42ns
//tRCD - 18ns
//tRP - 18ns
//tRC - 60ns( tRP + tRAS >= tRC)
//tWR - (1CLK + 6ns)

//For the 160 MHz case, tSDCLK = 1/160 MHz = 6.25ns
// CAS Latency = 3
// tRCD = 18 / 6.25 = 3(2.88)
// tRP = 18 / 6.25 = 3(2.88)
// tRC = 60 / 6.25 = 10
// tRAS = 42/ 6.25 = 7(6.72)
// tWR = (6.25 + 6)/6.25 = 2(1.96)
// fSDCLK = 160 MHz
// tREF=  64ms
// NRA = 8192

// RDIV = ((f SDCLK X t REF)/NRA) - (t RAS + t RP)
// RDIV = (((160 X 10^6 x 64 x 10^-3)/8192) - (7 + 3))
// RDIV = 1240 = 0x4D8


    *pSYSCTL |= MSEN; // Enables FLAG2 and 3 (IRQ2 and TIMEXP) as MS2 and 3


     // Mapping Bank 0 to SDRAM
    *pEPCTL |=B0SD;
    *pEPCTL &= ~(B1SD|B2SD|B3SD);


    //Programming the SDCTL register
     *pSDCTL= SDCL3|SDPSS|SDCAW9|SDRAW13|SDTRAS7|SDTRP3|SDTWR2|SDTRCD3|X16DE;



    // Change this value to optimize the performance for quazi-sequential accesses (step > 1)

    #define SDMODIFY 1                    // Setting the Modify to 1

    *pSDRRC= (0x4D8)|(SDMODIFY<<17)|SDROPT;



    *pSYSCTL |= EPDATA32; // Data [31:0] pins are configured as data lines


    // Flash is connected on Bank 1
	// Programming maximum waitstates

	*pAMICTL1 = AMIEN | BW8 | WS31 ;


	// SRAM is connected on Bank 3
	// SRAM part used - IS61WV102416BLL
	// As per datasheet access time is 10 ns, 8ns
	// Programming waitstates = 4


	*pAMICTL3 =  AMIEN | BW16 | WS4;








}








