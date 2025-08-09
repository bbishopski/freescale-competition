void sard_board_init(void);

	#define LED1					PTDD_PTDD0
	#define LED1DIR				PTDDD_PTDDD0
	#define LED2					PTDD_PTDD1
	#define LED2DIR				PTDDD_PTDDD1
	#define LED3					PTDD_PTDD3
	#define LED3DIR				PTDDD_PTDDD3
	#define LED4					PTDD_PTDD4
	#define LED4DIR				PTDDD_PTDDD4
	#define LEDPORT 			PTDD
	#define PB0						PTAD_PTAD2
	#define PB0PU					PTAPE_PTAPE2
	#define PB0DIR 				PTADD_PTADD2
	#define PB1 					PTAD_PTAD3
	#define PB1PU 				PTAPE_PTAPE3
	#define PB1DIR 				PTADD_PTADD3
	#define PB2 					PTAD_PTAD4
	#define PB2PU 				PTAPE_PTAPE4
	#define PB2DIR 				PTADD_PTADD4
	#define PB3 					PTAD_PTAD5
	#define PB3PU 				PTAPE_PTAPE5
	#define PB3DIR 				PTADD_PTADD5
	#define PB0IE 				KBIPE_KBIPE2
	#define PB1IE 				KBIPE_KBIPE3
	#define PB2IE 				KBIPE_KBIPE5
	#define PB3IE 				KBIPE_KBIPE4

	#define PUSH_BUTTON1 PTAD_PTAD2
	#define PUSH_BUTTON2 PTAD_PTAD3
	#define PUSH_BUTTON3 PTAD_PTAD4
	#define PUSH_BUTTON4 PTAD_PTAD5
	
	#define LED_OFF 1
	#define LED_ON 0
	#define PRESSED	0


#ifdef old
#define LED1					PTDD_PTDD0
#define LED1DIR					PTDDD_PTDDD0
#define LED2					PTBD_PTBD1
#define LED2DIR					PTBDD_PTBDD1
#define LED3					PTBD_PTBD3
#define LED3DIR					PTBDD_PTBDD3
#define LED4					PTBD_PTBD4
#define LED4DIR					PTBDD_PTBDD4
#define LEDPORT PTBD
#define PB0 PTAD_PTAD3
#define PB0PU PTAPE_PTAPE3
#define PB0DIR PTADD_PTADD3
#define PB1 PTAD_PTAD2
#define PB1PU PTAPE_PTAPE2
#define PB1DIR PTADD_PTADD2
#define PB2 PTAD_PTAD0
#define PB2PU PTAPE_PTAPE0
#define PB2DIR PTADD_PTADD0
#define PB3 PTAD_PTAD1
#define PB3PU PTAPE_PTAPE1
#define PB3DIR PTADD_PTADD1
#define PB0IE KBIPE_KBIPE3
#define PB1IE KBIPE_KBIPE2
#define PB2IE KBIPE_KBIPE1
#define PB3IE KBIPE_KBIPE0
#endif