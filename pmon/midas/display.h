
/*
 * BEWARE!
 * this macro assumes that it will not cause or suffer from any side effects
 */
#define DISPLAY(code)				\
	.set	noat;				\
	li	AT,PA_TO_KVA1(LED_BASE);	\
	sb	code,0(AT);			\
	.set	at
