#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/eeprom.h>

#define F_CPU 16000000UL 
#define BAUD 9600
#define UBRR_VALUE ((F_CPU / 16 / BAUD) - 1)



#define pb3 3
#define pb5 5
#define pb2 2 


uint16_t counter = 0;
uint16_t seconds = 0;
bool timer_paused = false;
bool timer_running = false; // 0 = timer av, 1 = timer på
uint16_t mapping_range [10]= {2, 5, 10, 25, 50, 100, 500, 1000, 5000, 9999};
uint8_t buttons_pressed, function, pind_value, last_function;
uint8_t setting = 0;

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

void save_setting_to_eeprom() {
    eeprom_update_byte((uint8_t*)0, setting);  // Spara `setting` på adress 0
}
void load_setting_from_eeprom() {
    setting = eeprom_read_byte((uint8_t*)0);  // Läs `setting` från adress 0
    if (setting > 9) setting = 0; // 🔹 Om EEPROM innehåller skräp, återställ till 0
}


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

    _delay_ms(100); // Enkel debounce

    uint8_t buttons_pressed = ~PIND; // Läs PIND
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
void display_number(uint16_t number) {
    uint8_t thousands = (number >= 1000) ? (number / 1000) % 10 : 0;
    uint8_t hundreds = (number >= 100) ? (number / 100) % 10 : 0;
    uint8_t tens = (number >= 10) ? (number / 10) % 10 : 0;
    uint8_t ones = number % 10;

    // Stäng av oanvända displayer genom att skicka 0x00 (inget visat)
    if (number < 1000) {
        execute(0x04, 0x00);  // Display 4 avstängd
    } else {
        execute(0x04, thousands);
    }

    if (number < 100) {
        execute(0x03, 0x00);  // Display 3 avstängd
    } else {
        execute(0x03, hundreds);
    }

    if (number < 10) {
        execute(0x02, 0x00);  // Display 2 avstängd
    } else {
        execute(0x02, tens);
    }

    execute(0x01, ones);  // Display 1 visar alltid ental
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

    return voltage;
}


uint16_t get_random_decimal() {
    uint16_t adc1 = ADC_read();  // Läs första ADC-värdet (0-1023)
    uint16_t adc2 = ADC_read();  // Läs andra ADC-värdet (0-1023)

    // Kombinera och förstora värdet för bättre spridning
    uint32_t combined_adc = ((uint32_t)adc1 * adc2) ^ counter; 

    // Mappa värdet till önskat intervall
    uint16_t random_value = (combined_adc % mapping_range[setting]) + 1;  

    // Skriver endast ut det slumpmässiga talet och funktionen
    printf("Random number: %d\n", random_value);

    return random_value;
}

volatile uint32_t timer_count = 0; // Variabel för timer-värdet
volatile uint8_t count_direction = 1; // 1 = upp, 0 = ner

void timer1_init() {
    TCCR1B |= (1 << WGM12); // CTC-läge
    TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler 64
    OCR1A = 249; // 1 ms intervall (16MHz / 64 / 1000 - 1)
    TIMSK1 |= (1 << OCIE1A); // Aktivera Timer1 Compare Match A Interrupt
    sei(); // Globala interrupts på
}

// Timer interrupt körs var 1 ms
ISR(TIMER1_COMPA_vect) {
    if (count_direction) {
        if (timer_count < 99990000) timer_count++; // Räkna upp
    } else {
        if (timer_count > 0) timer_count--; // Räkna ner
    }
   seconds = timer_count / 1000; // Beräkna sekunder
}

void start_timer() {
    timer_count = 0; // Återställ
    count_direction = 1; // Räkna upp
}
void resume_timer() {
    TCCR1B |= (1 << WGM12); // CTC-läge
    TCCR1B |= (1 << CS11) | (1 << CS10); // 🔹 Starta timern igen med prescaler 64
    TIMSK1 |= (1 << OCIE1A); // 🔹 Säkerställ att Timer1 Compare Match Interrupt är aktiverad
    timer_running = true;
}


void count_down() {
    count_direction = 0; // Räkna ner
}
void stop_timer() {
    TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10)); // 🔹 Stoppa timern genom att nollställa prescalern
    TIMSK1 &= ~(1 << OCIE1A); // 🔹 Inaktivera Timer1 Compare Match Interrupt
    timer_running = false; // Markera att timern är stoppad
}


uint8_t count_minute2, count_minute = 0;
void display_time (uint16_t time) {

    uint8_t second_dig2 = (time >= 10) ? (time / 10) % 10 : 0;
    uint8_t second_dig1 =  time % 10;
    uint8_t minute_dig3 = count_minute;
    uint8_t minute_dig4 = count_minute2;
    if (time < 10) {
        execute(0x02, 0x00);  // Display 2 avstängd
    } 
    else {
        execute(0x02, second_dig2);
    }
    execute(0x01, second_dig1);  // Display 1 visar alltid ental
    if(time == 59){
        timer_count = 0;
        count_minute++;
    }
    execute(0x03, minute_dig3 | 0b10000000); // Display 3 visar minuter i ental

    if (minute_dig3 == 10) {
        count_minute = 0;
        count_minute2 ++;
    }
    execute(0x04, minute_dig4); // Display 4 visar minuter i tiotal
    if (minute_dig4 == 10) {
        stop_timer();
        count_minute2 = 0;
        count_minute = 0;
        timer_count = 0;
}
}
// SPI-initiering för MAX7221
void spi_init() {

    DDRB = (1<<pb3)|(1<<pb5)|(1<<pb2);
    SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);

    execute(0x09,0b01111111); //decode
    execute(0x0B,0x07);  //nmr of displays
    execute(0x0C,0x01); // turn on screans
    execute(0x0A,0x05); //intensity
    execute(0x01, 0);
    execute(0x02, 0);
    execute(0x03, 0);
    execute(0x04, 0);
}
void setup_interrupts() {
        cli(); // Stäng av globala avbrott under konfiguration

        PCICR |= (1 << PCIE2);   // Aktivera Pin Change Interrupt för PORTD
        PCMSK2 |= 0xF0;          // Aktivera PCINT för PD0-PD3 (dina knappar)

        sei(); // Slå på globala avbrott
}

void update_display() {
        execute(0x08, 0b01000111); // Segment 8: "F"
        execute(0x07, last_function); // Segment 7: "E"
        execute(0x06, 5); // Segment 6: "S"
        execute(0x05, setting); // Segment 5: "T"
}

int main(void) {

    DDRD = 0; // PD0–PD3 som ingångar
    PORTD |= 0xF0; // Aktivera pullups på PD4–PD7




    uart_init();  // Initiera UART
    stdout = &uart_output; // Koppla printf() till UART
    load_setting_from_eeprom();  //  Läs inställning från EEPROM

    // Initiera timer
    timer1_init();
    spi_init();
    ADC_init();
    setup_interrupts();

    while (1) {
        
        counter++;
        update_display();
        printf("Timer: %d\n", seconds);

        if (function == 1) { 
            stop_timer();     // Stäng av timern
            timer_count = 0;  // Nollställ räknare
            count_minute = 0; // Återställ minuter
            count_minute2 = 0;
            timer_running = true; // Markera att timern är på
            timer_paused = false; // Se till att pausläget är falskt
            timer1_init();  //  Starta timern direkt igen
            display_time(timer_count); // Uppdatera display
            last_function = function;
            function = 0;
        }
        
        if(timer_running == true) {
            display_time(seconds); //  Skicka värdet till displayen
            _delay_ms(100); // 100 ms delay
        }
        if (timer_count == 9999) {
            stop_timer(); //  Stoppa timern
        }
        else if (function == 2) { 
            if (!timer_paused) {  
                stop_timer(); // 🔹 PAUSA timern
                timer_paused = true;
                printf("Timer paused!\n");
                last_function = function;
            } else { 
                resume_timer(); //  ÅTERSTARTA timern
                timer_paused = false;
                printf("Timer resumed!\n");
                last_function = 1;
            }
            function = 0;
        }
        
        
        else if(function == 3) {
            stop_timer();
            uint16_t random_value = get_random_decimal(); //  Läs en gång och spara
            display_number(random_value); //  Skicka värdet till displayen
            last_function = function; //  Nollställ funktionen
            function = 0; //  Nollställ funktionen
        }
        else if(function == 4) {
            stop_timer();
            setting ++;
            save_setting_to_eeprom();
            if(setting == 10) {
                setting = 0;
            }
            last_function = function;
            function = 0;
        }
    }
}

