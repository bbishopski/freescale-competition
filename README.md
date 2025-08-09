# freescale-competition
Prize winning project for Freescale international competition

The Magic Wah is a wireless expression pedal for musical instruments.  The system is based on the popular wah-wah pedal that has been around since the early 1960’s.  It produces a distinctive tone that many musicians rely use to add expression to their playing.   The traditional wah pedal requires that the musician manually step on the wah pedal to make the “wah” sound.  The wireless wah pedal frees the musician from the need to stand directly at the pedal and offers many other advantages over the traditional wah pedal.

Traditional wah pedals are simple analog circuits.  The wah effect is accomplished by sweeping the frequencies of a bandpass filter as the user steps on the pedal.  The act of stepping on the pedal turns a potentiometer that changes the bandpass filter frequencies making the “wah” sound.  The Magic Wah uses the same type of analog circuitry to achieve the “wah” sound, but rather than step on a pedal to control the sound, a sensor, worn on the foot, controls the “wah” effect.  The analog potentiometer is replaced with a digital potentiometer that is adjusted by a microcontroller.

The Magic Wah system consists of two physical devices that communicate with each other point to point using the 802.15.4 PHY layer.  The transmitter is worn on the foot of the musician, and the receiver is plugged into the musical instrument signal path.   The transmitter constantly monitors the foot angle of the musician and transmits the data to the receiver.  The receiver translates the foot angle into a digital potentiometer setting that changes the frequency of the analog bandpass filter, thus creating a “wah” effect as the foot is moved up and down.

The Magic Wah was developed using a pair of Freescale 13192 SARD (sensor applications reference design) boards.  The boards include MCS908GT microcontroller, MC13192 2.4GHz RF transceiver and two accelerometers: MMA6261 (X, Y axis) and MMA1260D (Z axis).   The analog “wah” circuit is controlled by the software using a  microcontroller output port that adjusts the value of the digital potentiometer (Dallas Semiconductor DS1804 100Kohm).

The MMA6261 and MMA1260D accelerometers are used to measure overall movement, detect specific gestures, and to measure tilt.  The system is turned off and on by simple foot gestures.  To turn the wah pedal on, the user simply plants their heel down with toe pointing up.  To turn off, the foot is moved to the side laterally.  When no movement is detected for an extended period of time, the software will transition into a low power mode to extend battery life.
 

