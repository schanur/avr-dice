#include <avr/io.h>
#include <stdlib.h>
#define F_CPU 8000000UL
#define baud 9600

struct pin_address {
    volatile unsigned char* port;
    unsigned char           pin;
};

typedef struct pin_address numeric_display[7];

numeric_display dice[2] = {{{&PORTD, 6},
                            {&PORTD, 7},
                            {&PORTB, 0},
                            {&PORTB, 5},
                            {&PORTC, 3},
                            {&PORTB, 3},  // B1
                            {&PORTD, 3}}, // B2,
                           {{&PORTB, 1},
                            {&PORTD, 2},
                            {&PORTD, 5},
                            {&PORTB, 2},
                            {&PORTC, 0},
                            {&PORTB, 4},
                            {&PORTD, 4}}};


                                                                                                //off  dash
unsigned char leds_of_number[12] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x00, 0x40};
//unsigned char leds_of_number[10] = {0x0f, 0x0c, 0xf6, 0x7c, 0x2d, 0x79, 0x7B, 0, 0, 0};

typedef volatile unsigned char* port;
port ports[3] = {&PORTB, &PORTC, &PORTD};

#define __HIGH   0
#define __LOW    1
#define __SWITCH 2

void set_pin(struct pin_address pin, unsigned char value)
{
    switch (value) {
	case __SWITCH: *(pin.port) ^= 1 << pin.pin; break;
	case __HIGH:   *(pin.port) |= 1 << pin.pin; break;
	case __LOW:    *(pin.port) &= ~(1 << pin.pin); break;
    }
}

unsigned short get_seed()
{
    unsigned short seed = 0;
    unsigned short *p = (unsigned short*) (RAMEND+1);
    extern unsigned short __heap_start;
    while (p >= &__heap_start + 1) seed ^= * (--p);
    return seed;
}

void wait_ms (uint16_t ms)
{
    for (; ms > 0; ms--) {
	uint16_t __c = 4000;
	__asm__ volatile (
	    "1: sbiw %0,1" "\n\t"
	    "brne 1b"
	    : "=w" (__c)
	    : "0" (__c)
	);
    }
}

void rs232_init(unsigned long cpu_freq, unsigned long baud_rate)
{
    long temp = cpu_freq/(16UL*baud_rate)-1;  //set uart baud rate register
    UBRR0H = (temp >> 8);
    UBRR0L = (temp & 0xFF);
    UCSR0B= (1 <<RXEN0 | 1 << TXEN0 ); // enable RX and TX 
    UCSR0C = (3<<UCSZ00);  // 8 bits, no parity, 1 stop
}

void rs232_char(char c)
{
    while((UCSR0A & (1 <<UDRE0)) == 0);
    UDR0 = c;
}

void rs232_string(char* cp)
{
    do {
	rs232_char(*cp);
    } while (*(++cp));
}

int main(void)
{
    srandom(get_seed());
    rs232_init(F_CPU, baud);
    //DDRD=0xff;
    
    DDRC = ~(1 << PC1);
    //PORTC = 1 << PC1;
    //DDRC &= ~(1 << PC1); // PD0 als Ausgang festlegen
    //PORTC |= (1 << PC1); // Pullup für PD0 aktivieren
    //DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB5);
    //DDRC |= (1 << PC2) | (1 << PC3);
    //DDRD |= (1 << PD6) | (1 << PD7);
    DDRB = 0xff;
    DDRD = 0xff;
    PORTB = 0xff;
    PORTC = 0xff;
    PORTD = 0xff;
    
    /*unsigned char port, pin;
    struct pin_address pin_no;
    while (1) {
    for (port = 0; port < 3; port++) {
	for (pin = 0; pin < 8; pin++) {
	    if ((port == 1) && (pin == 1)) continue;
	    if ((port == 2) && (pin < 2)) continue;
	    while ((PINC & (1 << PC1)) != 0);
	    wait_ms(150);
	    rs232_string("\nport: ");
	    rs232_char('0' + port);
	    rs232_string("\npin:  ");
	    rs232_char('0' + pin);
	    pin_no.port = ports[port];
	    pin_no.pin = pin;
	    set_pin(pin_no, __SWITCH);
	}
    }
    }*/
    
    unsigned char contact = 0;
    unsigned char show_result = 0;
    unsigned long push_cnt = 0;
    unsigned long release_cnt = 0;
    //unsigned long prng;
    unsigned long four_byte_rand;
    unsigned char* a;
    unsigned long three_bit_rand[2];
    unsigned long i;
    unsigned char last_value = 0;
    unsigned int de_jitter1;
    unsigned int de_jitter2;
    char int_buffer[65];
    
    for (i = 0; i < 7; ++i) {
	set_pin(dice[1][i], __LOW);
	set_pin(dice[0][i], __LOW);
	wait_ms(1000);
    }
    
    while (1) {
	if (show_result == 0) {
	    //show_result = 1;
	    de_jitter1 = 0;
	    de_jitter2 = 0;
	    if (contact == 0) {
		for (i = 0; i < 10000; i++) {
		    if ((PINC & (1 << PC1)) == 0) ++de_jitter1;
		    else de_jitter1 = 0;
		    
		    if (de_jitter1 > de_jitter2) de_jitter2 = de_jitter1;
		}
		if (de_jitter2 >= 1000) {
		    contact = 1;
	        } else {
		    release_cnt++;
	        }
	    } else {
		for (i = 0; i < 10000; i++) {
		    if ((PINC & (1 << PC1)) != 0) ++de_jitter1;
		    else de_jitter1 = 0;
		    if (de_jitter1 > de_jitter2) de_jitter2 = de_jitter1;
		}
		if (de_jitter2 >= 1000) {
		    contact = 0;
		    show_result = 1;
		} else {
		    push_cnt++;
		}
	    }
	} else {
	    //wait_ms (200);
	    show_result = 0;
	    four_byte_rand = push_cnt ^ release_cnt;
	    
	    do {
		three_bit_rand[0] = 0;
		four_byte_rand ^= random();
		a = (unsigned char*) &four_byte_rand;
		for (i = 0; i < 4; i++) {
		    three_bit_rand[0] ^= a[i];
		}
		a = (unsigned char*) &de_jitter2;
		for (i = 0; i < 4; i++) {
		    three_bit_rand[0] ^= a[i];
		}
		three_bit_rand[0] = (three_bit_rand[0] & 0x0f)
			     ^ ((three_bit_rand[0] & 0xf0) >> 4) 
			     ^ (last_value);
	    
	    } while (three_bit_rand[0] > 5);
	    do {
		three_bit_rand[1] = (unsigned char) (random() % 6);
	    } while (three_bit_rand[1] > 5);
	    last_value = three_bit_rand[0];
	    ultoa (four_byte_rand, int_buffer, 10);
	    rs232_string("\n\nfour_byte_rand: ");
	    rs232_string(int_buffer);
	    ultoa (de_jitter1, int_buffer, 10);
	    rs232_string("\nde_jitter1: ");
	    rs232_string(int_buffer);
	    ultoa (push_cnt, int_buffer, 10);
	    rs232_string("\npush_cnt: ");
	    rs232_string(int_buffer);
	    ultoa (release_cnt, int_buffer, 10);
	    rs232_string("\nrelease_cnt: ");
	    rs232_string(int_buffer);
	    rs232_string("\noutput value 1: ");
	    rs232_char('1' + three_bit_rand[0]);
	    rs232_string("\noutput value 2: ");
	    rs232_char('1' + three_bit_rand[1]);
	
	    for (i = 0; i < 7; ++i) {
		    set_pin(dice[0][i], !!(leds_of_number[three_bit_rand[0] + 1] & (1 << i)));
		    set_pin(dice[1][i], !!(leds_of_number[three_bit_rand[1] + 1] & (1 << i)));
	    }
	
	}
    }
    return (0);
}
