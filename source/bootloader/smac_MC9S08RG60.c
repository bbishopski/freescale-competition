/* Based on CPU DB MC9S08RG60, version 2.87.009 (RegistersPrg V1.061) */
/* DataSheet : MC9S08RG60/D Rev. 1.07, 2/2004 */

#include "smac_MC9S08RG60.h"


/* * * * *  8-BIT REGISTERS  * * * * * * * * * * * * * * * */
volatile ACMP1SCSTR _ACMP1SC;                              /* Analog Comparator Status and Control Register */
volatile CMTMSCSTR _CMTMSC;                                /* CMT Modulator Status and Control Register */
volatile CMTOCSTR _CMTOC;                                  /* CMT Output Control Register */
volatile DBGCSTR _DBGC;                                    /* Debug Control Register */
volatile DBGCAHSTR _DBGCAH;                                /* Debug Comparator A High Register */
volatile DBGCALSTR _DBGCAL;                                /* Debug Comparator A Low Register */
volatile DBGCBHSTR _DBGCBH;                                /* Debug Comparator B High Register */
volatile DBGCBLSTR _DBGCBL;                                /* Debug Comparator B Low Register */
volatile DBGSSTR _DBGS;                                    /* Debug Status Register */
volatile DBGTSTR _DBGT;                                    /* Debug Trigger Register */
volatile FCDIVSTR _FCDIV;                                  /* FLASH Clock Divider Register */
volatile FCMDSTR _FCMD;                                    /* FLASH Command Register */
volatile FCNFGSTR _FCNFG;                                  /* FLASH Configuration Register */
volatile FOPTSTR _FOPT;                                    /* FLASH Options Register */
volatile FPROTSTR _FPROT;                                  /* FLASH Protection Register */
volatile FSTATSTR _FSTAT;                                  /* FLASH Status Register */
volatile IRQSCSTR _IRQSC;                                  /* Interrupt Request Status and Control Register */
volatile KBI1PESTR _KBI1PE;                                /* KBI1 Pin Enable Register */
volatile KBI1SCSTR _KBI1SC;                                /* KBI1 Status and Control */
volatile KBI2PESTR _KBI2PE;                                /* KBI2 Pin Enable Register */
volatile KBI2SCSTR _KBI2SC;                                /* KBI2 Status and Control */
volatile PTADSTR _PTAD;                                    /* Port A Data Register */
volatile PTADDSTR _PTADD;                                  /* Data Direction Register A */
volatile PTAPESTR _PTAPE;                                  /* Pullup Enable for Port */
volatile PTBDSTR _PTBD;                                    /* Port B Data Register */
volatile PTBDDSTR _PTBDD;                                  /* Data Direction Register B */
volatile PTBPESTR _PTBPE;                                  /* Pullup Enable for Port B */
volatile PTCDSTR _PTCD;                                    /* Port C Data Register */
volatile PTCDDSTR _PTCDD;                                  /* Data Direction Register C */
volatile PTCPESTR _PTCPE;                                  /* Pullup Enable for Port C */
volatile PTDDSTR _PTDD;                                    /* Port D Data Register */
volatile PTDDDSTR _PTDDD;                                  /* Data Direction Register D */
volatile PTDPESTR _PTDPE;                                  /* Pullup Enable for Port D */
volatile PTEDSTR _PTED;                                    /* Port E Data Register */
volatile PTEDDSTR _PTEDD;                                  /* Data Direction Register E */
volatile PTEPESTR _PTEPE;                                  /* Pullup Enable for Port E */
volatile SBDFRSTR _SBDFR;                                  /* System Background Debug Force Reset Register */
volatile SCI1C1STR _SCI1C1;                                /* SCI1 Control Register 1 */
volatile SCI1C2STR _SCI1C2;                                /* SCI1 Control Register 2 */
volatile SCI1C3STR _SCI1C3;                                /* SCI1 Control Register 3 */
volatile SCI1DRLSTR _SCI1DRL;                              /* SCI1 Data Register */
volatile SCI1S1STR _SCI1S1;                                /* SCI1 Status Register 1 */
volatile SCI1S2STR _SCI1S2;                                /* SCI1 Status Register 2 */
volatile SOPTSTR _SOPT;                                    /* System Integration Module Options Register */
volatile SPI1BRSTR _SPI1BR;                                /* SPI1 Baud Rate Register */
volatile SPI1C1STR _SPI1C1;                                /* SPI1 Control Register 1 */
volatile SPI1C2STR _SPI1C2;                                /* SPI1 Control Register 2 */
volatile SPI1DSTR _SPI1D;                                  /* SPI1 Data Register */
volatile SPI1SSTR _SPI1S;                                  /* SPI1 Status Register */
volatile SPMSC1STR _SPMSC1;                                /* System Power Management Status and Control 1 Register */
volatile SPMSC2STR _SPMSC2;                                /* System Power Management Status and Control 2 Register */
volatile SRSSTR _SRS;                                      /* System Reset Status Register */
volatile SRTISCSTR _SRTISC;                                /* System RTI Status and Control Register */
volatile TPM1C0SCSTR _TPM1C0SC;                            /* TPM1 Timer Channel 0 Status and Control Register */
volatile TPM1C1SCSTR _TPM1C1SC;                            /* TPM1 Timer Channel 1 Status and Control Register */
volatile TPM1SCSTR _TPM1SC;                                /* TPM1 Timer Status and Control Register */


/* * * * *  16-BIT REGISTERS  * * * * * * * * * * * * * * * */
volatile CMTCG1STR _CMTCG1;                                /* Carrier Generator Data Register 1 */
volatile CMTCG2STR _CMTCG2;                                /* Carrier Generator Data Register 2 */
volatile CMTCMD12STR _CMTCMD12;                            /* CMT Modulator Data Register 12 */
volatile CMTCMD34STR _CMTCMD34;                            /* CMT Modulator Data Register 34 */
volatile DBGFSTR _DBGF;                                    /* Debug FIFO Register */
volatile SCI1BDSTR _SCI1BD;                                /* SCI1 Baud Rate Register */
volatile SDIDSTR _SDID;                                    /* System Device Identification Register */
volatile TPM1C0VSTR _TPM1C0V;                              /* TPM1 Timer Channel 0 Value Register */
volatile TPM1C1VSTR _TPM1C1V;                              /* TPM1 Timer Channel 1 Value Register */
volatile TPM1CNTSTR _TPM1CNT;                              /* TPM1 Counter Register */
volatile TPM1MODSTR _TPM1MOD;                              /* TPM1 Timer Counter Modulo Register */

/* EOF */
