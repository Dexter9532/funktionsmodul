#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#define F_CPU 16000000UL  // Processorhastighet (ändra vid behov)
#define BAUD 9600
#define UBRR_VALUE ((F_CPU / 16 / BAUD) - 1)



#define pb3 3
#define pb5 5
#define pb2 2 

uint16_t counter = 0;
uint8_t mapping_range = 9; // 🔹 Standard: Mappas till 1-4
uint8_t buttons_pressed, function, pind_value;

void uart_init() {
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8); // Ställ in baudrate hög byte
    UBRR0L = (uint8_t)UBRR_VALUE;        // Ställ in baudrate låg byte
    UCSR0B = (1 << TXEN0);  // Aktivera UART-sändning
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  // 8-bit data, 1 stoppbit
}
int uart_putchar(char c, FILE *stream) {
    while (!(UCSR0A & (1 << UDRE0))); // Vänta tills buffern är tom
    UDR0 = c; // Skicka tecken
    return 0;
}
FILE uart_output = FDEV_SETUP_STREAM(uart_putchar, NULL, _FDEV_SETUP_WRITE);

void print_pind_binary() {
    printf("PIND: ");
    for (int8_t i = 7; i >= 0; i--) {  // Loopar igenom varje bit från MSB till LSB
        if (PIND & (1 << i)) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n"); // Ny rad i Serial Monitor
}
void execute(uint8_t cmd,uint8_t data)
{
    PORTB &= ~(1<<pb2);
    
    SPDR = cmd;
    while(!(SPSR & (1<<SPIF)));
    
    SPDR = data;
    while(!(SPSR & (1<<SPIF)));
    
    PORTB |= (1<<pb2);
}
// Interrupt för knappar
ISR(PCINT2_vect) {

    _delay_ms(50); // Enkel debounce

    uint8_t buttons_pressed = ~PIND; // Läs PIND

    printf("Interrupt triggat! PCIF2 = %d\n", (PCIFR & (1 << PCIF2)) ? 1 : 0);
    print_pind_binary(); // Skriv ut PIND i binär form
    


    switch (buttons_pressed & 0xF0) { // 🔹 Filtrera endast PD4–PD7
        case 0b00010000: function = 1; break; // PD4
        case 0b00100000: function = 2; break; // PD5
        case 0b01000000: function = 3; break; // PD6
        case 0b10000000: function = 4; break; // PD7
    }
    printf("Function: %d\n", function);
    PCIFR |= (1 << PCIF2); // 🔹 Rensa interrupt-flaggan (måste göras manuellt)
    
}



// Rensa displayen
void clear_display() {
    for (uint8_t i = 1; i <= 8; i++) {
        execute(i, 0x00); // Sätter alla segment till av
    }
}

void ADC_init() {
    ADMUX = (1 << REFS0); // Välj referensspänning AVCC (5V)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // Aktivera ADC och sätt prescaler till 128 (16MHz/128 = 125kHz ADC-klocka)
}

uint16_t ADC_read() {
    ADCSRA |= (1 << ADSC); // Starta ADC-konvertering
    while (ADCSRA & (1 << ADSC)); // Vänta tills konvertering är klar
    return ADC; // Returnera 10-bitars ADC-värde (0–1023)
}

float ADC_getVoltage() {
    uint16_t adc_value = ADC_read();
    float voltage = (adc_value * 5.0) / 1023.0; // Beräkna spänning i volt

    printf("ADC Voltage: %.5f V\n", voltage);
    return voltage;
}




uint8_t get_random_decimal() {
    uint16_t adc1 = ADC_read();
    uint16_t adc2 = ADC_read();

    uint8_t noise_bits = (adc1 ^ adc2 ^ counter) & 0x1F; //  XOR med counter för mer slump
    uint8_t random_value = (noise_bits % 9) + 1; //  Mappa till 1–9

    printf("ADC1: %d, ADC2: %d, Counter: %d, Noise Bits: %d, Random: %d\n",
           adc1, adc2, counter, noise_bits, random_value);

    return random_value;
}


// SPI-initiering för MAX7221
void spi_init() {

    DDRB = (1<<pb3)|(1<<pb5)|(1<<pb2);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);

    execute(0x09,0b01111111); //decode
    execute(0x0B,0x07);  //nmr of displays
    execute(0x0C,0x01); // turn on screans
    execute(0x0A,0x05); //intensity
}
void setup_interrupts() {
        cli(); // Stäng av globala avbrott under konfiguration

        PCICR |= (1 << PCIE2);   // Aktivera Pin Change Interrupt för PORTD
        PCMSK2 |= 0xF0;          // Aktivera PCINT för PD0-PD3 (dina knappar)

        sei(); // Slå på globala avbrott
}

void update_display() {
        execute(0x08, 0b01000111); // Segment 8: "F"
        execute(0x07, function); // Segment 7: "E"
        execute(0x06, 5); // Segment 6: "S"
}

int main(void) {

    DDRD = 0;
    PORTD |= 0xF0; // Aktivera pullups på PD4–PD7




    uart_init();  // Initiera UART
    stdout = &uart_output; // Koppla printf() till UART


    spi_init();
    ADC_init();
    setup_interrupts();

    while (1) {
        
        counter++;
        update_display();

        if(function == 0) {
            execute(0x01,0);
        }
        if(function == 1) {

            execute(0x01,1);
        }
        else if (function == 2) {
            uint8_t random_value = get_random_decimal(); //  Läs en gång och spara
            printf("Random number: %d\n", random_value); //  Debug print
            execute(0x02, random_value); //  Skicka värdet till displayen
            function = 0; //  Nollställ funktionen
        }
        else if(function == 3) {

            execute(0x01,3);
        }
        else if(function == 4) {

            execute(0x01,4);
        }
    }
}

