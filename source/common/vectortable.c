typedef void(*tIsrFunc)(void);

// External ISRs
extern interrupt void irq_isr(void);
extern interrupt void KBD_ISR();
extern interrupt void RTI_ISR();
extern interrupt void Vscirx();

interrupt void UnimplementedISR(void)
{
  for (;;);
}

// Added redirected ISR vectors when BootLoader is enabled.
// The application cannot have a reset vector (resides in BootLoader).
#ifdef BOOTLOADER_ENABLED
// Redirected ISR vectors
const tIsrFunc _vect[] @0xEFCC = {  /* Interrupt table */
#else
const tIsrFunc _vect[] @0xFFCC = {  /* Interrupt table */
#endif BOOTLOADER_ENABLED
  RTI_ISR,                /* vector 25: RT */
  UnimplementedISR,       /* vector 24: IIC */
  UnimplementedISR,       /* vector 23: ATD */
  KBD_ISR,                /* vector 22: KBI */
  UnimplementedISR,       /* vector 21: SCI2TX */
  Vscirx,                 /* vector 20: SCI2RX */
  UnimplementedISR,       /* vector 19: SCI2ER */
  UnimplementedISR,       /* vector 18: SCI1TX */
  Vscirx,                 /* vector 17: SCI1RX */
  UnimplementedISR,       /* vector 16: SCI1ER */
  UnimplementedISR,       /* vector 15: SPI */
  UnimplementedISR,       /* vector 14: TPM2OF */
  UnimplementedISR,       /* vector 13: TPM2C4 */
  UnimplementedISR,       /* vector 12: TPM2C3 */
  UnimplementedISR,       /* vector 11: TPM2C2 */
  UnimplementedISR,       /* vector 10: TPM2C1 */
  UnimplementedISR,       /* vector 09: TPM2C0 */
  UnimplementedISR,       /* vector 08: TPM1OF */
  UnimplementedISR,       /* vector 07: TPM1C2 */
  UnimplementedISR,       /* vector 06: TPM1C1 */
  UnimplementedISR,       /* vector 05: TPM1C0 */
  UnimplementedISR,       /* vector 04: ICG */
  UnimplementedISR,       /* vector 03: Low Voltage Detect */
  irq_isr,                /* vector 02: IRQ pin */
  UnimplementedISR        /* vector 01: SWI */
  /* Reset Vector written to by linker file */
  /* _Startup, see start08.c */
};

