/* Based on CPU DB MC9S08GT60, version 2.87.019 (RegistersPrg V1.061) */
/* DataSheet : manual revision not specified */

#include "smac_MC9S08GT60.h"


/* * * * *  8-BIT REGISTERS  * * * * * * * * * * * * * * * */
volatile ATD1CSTR _ATD1C;                                  /* ATD1 Control Register */
volatile ATD1PESTR _ATD1PE;                                /* ATD1 Pin Enable */
volatile ATD1SCSTR _ATD1SC;                                /* ATD1 Status and Control Register */
volatile DBGCSTR _DBGC;                                    /* Debug Control Register */
volatile DBGCAHSTR _DBGCAH;                                /* Debug Comparator A High Register */
volatile DBGCALSTR _DBGCAL;                                /* Debug Comparator A Low Register */
volatile DBGCBHSTR _DBGCBH;                                /* Debug Comparator B High Register */
volatile DBGSSTR _DBGS;                                    /* Debug Status Register */
volatile DBGTSTR _DBGT;                                    /* Debug Trigger Register */
volatile FCDIVSTR _FCDIV;                                  /* FLASH Clock Divider Register */
volatile FCMDSTR _FCMD;                                    /* FLASH Command Register */
volatile FCNFGSTR _FCNFG;                                  /* FLASH Configuration Register */
volatile FOPTSTR _FOPT;                                    /* FLASH Options Register */
volatile FPROTSTR _FPROT;                                  /* FLASH Protection Register */
volatile FSTATSTR _FSTAT;                                  /* FLASH Status Register */
volatile ICGC1STR _ICGC1;                                  /* ICG Control Register 1 */
volatile ICGC2STR _ICGC2;                                  /* ICG Control Register 2 */
volatile ICGS1STR _ICGS1;                                  /* ICG Status Register 1 */
volatile ICGS2STR _ICGS2;                                  /* ICG Status Register 2 */
volatile ICGTRMSTR _ICGTRM;                                /* ICG Trim Register */
volatile IIC1ASTR _IIC1A;                                  /* IIC1 Address Register */
volatile IIC1CSTR _IIC1C;                                  /* IIC1 Control Register */
volatile IIC1DSTR _IIC1D;                                  /* IIC1 Data I/O Register */
volatile IIC1FSTR _IIC1F;                                  /* IIC1 Frequency Divider Register */
volatile IIC1SSTR _IIC1S;                                  /* IIC1 Status Register */
volatile IRQSCSTR _IRQSC;                                  /* Interrupt Request Status and Control Register */
volatile KBI1PESTR _KBI1PE;                                /* KBI1 Pin Enable Register */
volatile KBI1SCSTR _KBI1SC;                                /* KBI1 Status and Control */
volatile PTADSTR _PTAD;                                    /* Port A Data Register */
volatile PTADDSTR _PTADD;                                  /* Data Direction Register A */
volatile PTAPESTR _PTAPE;                                  /* Pullup Enable for Port A */
volatile PTASESTR _PTASE;                                  /* Slew Rate Control Enable for Port A */
volatile PTBDSTR _PTBD;                                    /* Port B Data Register */
volatile PTBDDSTR _PTBDD;                                  /* Data Direction Register B */
volatile PTBPESTR _PTBPE;                                  /* Pullup Enable for Port B */
volatile PTBSESTR _PTBSE;                                  /* Slew Rate Control Enable for Port B */
volatile PTCDSTR _PTCD;                                    /* Port C Data Register */
volatile PTCDDSTR _PTCDD;                                  /* Data Direction Register C */
volatile PTCPESTR _PTCPE;                                  /* Pullup Enable for Port C */
volatile PTCSESTR _PTCSE;                                  /* Slew Rate Control Enable for Port C */
volatile PTDDSTR _PTDD;                                    /* Port D Data Register */
volatile PTDDDSTR _PTDDD;                                  /* Data Direction Register D */
volatile PTDPESTR _PTDPE;                                  /* Pullup Enable for Port D */
volatile PTDSESTR _PTDSE;                                  /* Slew Rate Control Enable for Port D */
volatile PTEDSTR _PTED;                                    /* Port E Data Register */
volatile PTEDDSTR _PTEDD;                                  /* Data Direction Register E */
volatile PTEPESTR _PTEPE;                                  /* Pullup Enable for Port E */
volatile PTESESTR _PTESE;                                  /* Slew Rate Control Enable for Port E */
volatile PTGDSTR _PTGD;                                    /* Port G Data Register */
volatile PTGDDSTR _PTGDD;                                  /* Data Direction Register G */
volatile PTGPESTR _PTGPE;                                  /* Pullup Enable for Port G */
volatile PTGSESTR _PTGSE;                                  /* Slew Rate Control Enable for Port G */
volatile SBDFRSTR _SBDFR;                                  /* System Background Debug Force Reset Register */
volatile SCI1C1STR _SCI1C1;                                /* SCI1 Control Register 1 */
volatile SCI1C2STR _SCI1C2;                                /* SCI1 Control Register 2 */
volatile SCI1C3STR _SCI1C3;                                /* SCI1 Control Register 3 */
volatile SCI1DSTR _SCI1D;                                  /* SCI1 Data Register */
volatile SCI1S1STR _SCI1S1;                                /* SCI1 Status Register 1 */
volatile SCI1S2STR _SCI1S2;                                /* SCI1 Status Register 2 */
volatile SCI2C1STR _SCI2C1;                                /* SCI1 Control Register 1 */
volatile SCI2C2STR _SCI2C2;                                /* SCI2 Control Register 2 */
volatile SCI2C3STR _SCI2C3;                                /* SCI2 Control Register 3 */
volatile SCI2DSTR _SCI2D;                                  /* SCI2 Data Register */
volatile SCI2S1STR _SCI2S1;                                /* SCI2 Status Register 1 */
volatile SCI2S2STR _SCI2S2;                                /* SCI2 Status Register 2 */
volatile SOPTSTR _SOPT;                                    /* System Options Register */
volatile SPI1BRSTR _SPI1BR;                                /* SPI1 Baud Rate Register */
volatile SPI1C1STR _SPI1C1;                                /* SPI1 Control Register 1 */
volatile SPI1C2STR _SPI1C2;                                /* SPI1 Control Register 2 */
volatile SPI1DSTR _SPI1D;                                  /* SPI1 Data Register */
volatile SPI1SSTR _SPI1S;                                  /* SPI1 Status Register */
volatile SPMSC1STR _SPMSC1;                                /* PM Status and Control 1 Register */
volatile SPMSC2STR _SPMSC2;                                /* PM Status and Control 2 Register */
volatile SRSSTR _SRS;                                      /* System Reset Status */
volatile SRTISCSTR _SRTISC;                                /* System RTI Status and Control Register */
volatile TPM1C0SCSTR _TPM1C0SC;                            /* TPM 1 Timer Channel 0 Status and Control Register */
volatile TPM1C1SCSTR _TPM1C1SC;                            /* TPM 1 Timer Channel 1 Status and Control Register */
volatile TPM1SCSTR _TPM1SC;                                /* TPM 1 Status and Control Register */
volatile TPM2C0SCSTR _TPM2C0SC;                            /* TPM 2 Timer Channel 0 Status and Control Register */
volatile TPM2C1SCSTR _TPM2C1SC;                            /* TPM 2 Timer Channel 1 Status and Control Register */
volatile TPM2SCSTR _TPM2SC;                                /* TPM 2 Status and Control Register */


/* * * * *  16-BIT REGISTERS  * * * * * * * * * * * * * * * */
volatile ATD1RSTR _ATD1R;                                  /* ATD1 Result Data */
volatile DBGFSTR _DBGF;                                    /* Debug FIFO Register */
volatile ICGFLTSTR _ICGFLT;                                /* ICG Filter Register */
volatile SCI1BDSTR _SCI1BD;                                /* SCI1 Baud Rate Register */
volatile SCI2BDSTR _SCI2BD;                                /* SCI2 Baud Rate Register */
volatile SIMIDSTR _SIMID;                                  /* System Integration Module Part ID Register */
volatile TPM1C0VSTR _TPM1C0V;                              /* TPM 1 Timer Channel 0 Value Register */
volatile TPM1C1VSTR _TPM1C1V;                              /* TPM 1 Timer Channel 1 Value Register */
volatile TPM1CNTSTR _TPM1CNT;                              /* TPM 1 Counter Register */
volatile TPM1MODSTR _TPM1MOD;                              /* TPM 1 Timer Counter Modulo Register */
volatile TPM2C0VSTR _TPM2C0V;                              /* TPM 2 Timer Channel 0 Value Register */
volatile TPM2C1VSTR _TPM2C1V;                              /* TPM 2 Timer Channel 1 Value Register */
volatile TPM2CNTSTR _TPM2CNT;                              /* TPM 2 Counter Register */
volatile TPM2MODSTR _TPM2MOD;                              /* TPM 2 Timer Counter Modulo Register */

/* EOF */
