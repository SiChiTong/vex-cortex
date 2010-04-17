/* Documetaion of the VEX interfaces */

#define __packed __attribute__((packed))
struct oi_data {
	// right stick
	u8 axis_1;
	u8 axis_2;
	
	// left stick
	u8 axis_3;
	u8 axis_4;
	
	// accelerometer
	u8 accel_x;
	u8 accel_y;
	u8 accel_z;
	
	// trigger buttons ( comment data given is clearly incorrect)
	/* 
	byte 7 = Group 5 & 6   //Buttons  
		bit0 = Left Down       *** Group 5
		bit1 = Left Up
		bit2 = Right Down      *** Group 6
		bit3 = Left Down
	*/
	u8 g5_u:1;
	u8 g5_d:1;
	u8 g6_u:1;
	u8 g6_d:1;
	u8 reserved1:4; // not mentioned.
	
	u8 g8_down:1;
	u8 g8_left:1;
	u8 g8_up:1;
	u8 g8_right:1;
	
	u8 g7_down:1;
	u8 g7_left:1;
	u8 g7_up:1;
	u8 g7_right:1;
	
	u8 reserved2[3]; // noted as "spare"
} __packed;

struct state_pack {
	u8 iack:1;
	u8 config:1;
	u8 initializing:1; // data is not ready.
	u8 valid:1; // data is valid
	u8 reserved:4;
} __packed;

/* SPI (uses SPI1)
*Initialization Process:
PE0 set high ("SPI1_INT")
read 8 bytes of junk data.
wait for PE3 && PE4 to be low.

*Tranfers
Sync set to SYNC_MAGIC.
State set to 2.
version set to 1.
SystemsFlags = 1 to induce auton, 0 otherwise.

Transfers are triggered every 20ms.
32 bytes are transmitted for every transfer.
PA11 set high, called "RTS". ("RTS high Used to ensure 1st 4 bytes").
For each byte:
	PE1 set low; appears to be slave select. ("Use as SSL0")
	Reads one byte.
	Writes one byte.
	PE1 set high.
	Claims to wait 15us (uses "for(i=0;i<150;i++)")
	if the byte number is a multiple of 4 (this is what the code does.
	  comments, however, indicate that this should only occour after the 4th byte):
		They "Make a gap" (sersiously?)
		delay 210us ("for(i=0;i<1000;i++)")
		PA11 ("RTS") set low.
packet num (in the slave packet) is incremented following each transfer.
*/

#define SYNC_MAGIC 0xC917

//Data From Master
typedef struct { 
	u16 sync; // Should always be SYNC_MAGIC
	union {
		u8  a;
		struct state_pack b;
	} state;
	union {
		u8 a;
		struct {
			u8 tx1_active:1;
			u8 tx2_active:1;
			u8 spare:1;
			u8 competition_mode:1; //XXX: what does this imply?
			u8 reset_slave:1; //XXX: noted as "(Reserved)" but has a name.
			u8 joystick_mode:1; //XXX: wtf is joystick mode?
			u8 autonomus:1;
			u8 disable:1;
		} __packed b;
	} SystemFlags;
	u8  mainBatteryVoltage; // mult by 0.0591 for something readable.      
	u8  backupBatteryVoltage;
	union {
		u8  a[12];
		struct oi_data b;
	} joystick[2];
	u8  version;
	u8  packetNum;
} __packed spi_packet_m2u;

//Data To Master
typedef struct { 
	u16 sync; // should always be SYNC_MAGIC
	union {
		u8 a;
		struct state_pack b;
	} state;
	union {
		u8 a;
		struct {
			u8 auton:1;
			u8 crystal_mode:1;
			u8 disable:1;
			u8 brake:1; //XXX: what does this mean?
			u8 enable_printfs:1; //XXX: noted as "Reserved for Master"
			u8 enable_display:1; //XXX: noted as "Reserved for Master"
			u8 reserved:2; // unmentioned.
		} __packed b;
	} SystemFlags; //XXX: "Reserved for Slave (TBD)"
	u8  DigitalByte1;   //Digital bits 1-8      
	u8  DigitalByte2;   //Digital bits 9-12, 13-16 (spare)   
	u8  Motor[8];       //PWM values 0-255
	u8  MotorStatus[8]; //XXX: "PWM motor states (TBD)"
	u8  Analog[8];      //Analog port (1-8)
	u8  version;
	u8  packetNum;
} __packed spi_packet_u2m;


/** GPIO INITS 
 ** Things starting with "Claim" reverence
 ** Vex/IFI comments in orig. code. 
 ** For comedic value
 **/

/* Uses SPI1: 
 * SCK, MISO, MOSI : PA{5,6,7} 
 * XXX: Vex makes them all ouputs
 */

/* "RTS" : PA11 (set as input) */

/* "RX1" (Crystals) : A10 (set as output) */

/* Noted as "Set to analog" : PC{0,1,2,3} */

/* under same heading, set as input floating:
 *   PC{6,7,8}
 */

/* "Smart Motor" Setup :
 * Call "SetMotorControl_To_Neutral"  
 * Claim to set PD{0,1} to ouput pp. (Don't)
 * PD{3,4,7,8} = output pp.
 * Then do this nice thing:
 *  GPIOD->CRH = (9 << 16) | (9 << 20) 
 *	  | (9 << 24) | ( 9 << 28) | 1;
 * Thanks VEX.
 */
 
/* PD{0,1} = input */

/* PE{4,7,8,9,10,11,12,13,14} = input
 *  Note: only claim to set PE{13,14} = input
 */
 
/* Claim to set PE{9,11,13,14} = output pp
 * Actualy set PE{0,6} to output pp
 */ 

/** END GPIO INITS **/
 
/* SPI1:
	Master Mode, bidirectional,
	16b data size, CPOL_Low,
	CPHA_2Edge, NSS_Soft,
	Prescale = 32 (Note: "highest = 16"),
	MSB First. CRC Poly = 7.
*/

/* ADC1:
Channels : {0,1,2,3,12,13,10,11}
Set to scan mode
*/

/* Interrupts used:
TIM1, TIM2, TIM3, TIM4, EXTI9_5
*/
 
/* Crystal Detection
PB10: low when RX1 is connected.
PC8 : low when RX2 is connected.
*/

/* USART:
USART1 is debug usart.
USART{2,3} expected to be connected to
pins labeled "USART{1,2}"
Baud = 115200, word = 8b, 1 stopbit,
no parity, no harware flow ctrl.
*/

/* SysTick
VeX/IFI appears to use this for a msdelay.
"reload" set to 9000, interupt enabled.
Claim the 9000 value gives a 1ms tick.
*/

/* TIM1 used to trigger an update of the
 * master processor
 */
 
/* TIM{2,3} is using capture #3 and
 * filling the pwm{1,2} array.
 * For crystal input.
*/

/* TIM4 essentially ignored */

/* EXTI9_5 is KEY_BUTTON?
 * seems to blink some LED.
 */

/* There are constant references to the
 * following things. No idea what they mean:
TARGET_BOARD
KEY_BUTTON
 */ 

/* Control Pins for Motors 1 & 10. Connected directly to the STM.
// See Set_MotorControl_Sw{1,2} for details.
// (These use timer4 in the default code)
// Active low. Names taken from original comments.
PD3 : AH1 // gpio
PD4 : BH1 // gpio
PD7 : AH2 // gpio
PD8 : BH2 // gpio
PD12: AL1
PD13: BL1
PD14: AL2 
PD15: BL2
*/

/* Digital IO
PE9 : 1
PE11: 2
PC6 : 3
PC7 : 4
PE13: 5
PE14: 6
PE8 : 7
PE10: 8
PE12: 9
PE7 : 10
PD0 : 11
PD1 : 12
*/
