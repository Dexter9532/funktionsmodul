#include <Arduino.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>

#define CS 2   // PB2 (CS/LOAD)
#define PB0 0
#define PB3 3
#define PB5 5


uint8_t buttons_pressed, function;

ISR(PCINT2_vect) {

  buttons_pressed = ~PIND; // Läs in knapptryckningar

  switch (buttons_pressed) {
      case 0b00000001: function = 1; break;
      case 0b00000010: function = 2; break;
      case 0b00000100: function = 3; break;
      case 0b00001000: function = 4; break;
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
uint8_t get_random_decimal() {
    uint16_t adc_value = ADC_read(); // Läs ADC (0 - 1023)
    
    int last_digit = (adc_value % 4) + 1; // Mappa till 1–4

    return last_digit;
}
void spi_init(){
    
  PORTB |= (1 << CS); // CS börjar HIGH (inaktiv)
  // SPI inställningar: Enable SPI, Master mode, SCK = F_CPU/16
  SPCR = (1 << SPE) | (1 << MSTR); // SPI på, master, stigande flank

  execute(0x09, 0b00111111); // Aktiverar decode för DIG0 - DIG5, ingen decode på DIG6-7
  execute(0x0B, 0x07); // hur många används
  execute(0x0A, 0x05); // ljusstyrka
  execute(0x0C, 0x01); // Slå på MAX7221
}
void setup_interrupts() {
  cli();
  PCICR |= (1 << PCIE2);
  PCMSK2 |= 0xFF;
  sei();
}
void execute(uint8_t cmd, uint8_t data)
{
    PORTB &= ~(1 << CS); // CS LOW (starta överföring)

    SPDR = cmd;  // Skicka kommando
    while (!(SPSR & (1 << SPIF))); // Vänta på överföring klar

    SPDR = data; // Skicka data
    while (!(SPSR & (1 << SPIF))); // Vänta på överföring klar

    PORTB |= (1 << CS); // CS HIGH (slut på överföring)
}

void main ()
{
  DDRD = 0;
  PORTD = 255;
  DDRB = (1 << PB3) | (1 << PB5) | (1 << CS);

  spi_init();
  ADC_init();
  setup_interrupts();

  while (1)
  {
    //state settings
    while(function == 0){

    }
    //randomizer
    while (function == 1)
    {
      get_random_decimal();
    }
    //counter
    while (function == 2)
    {
      /* code */
    }
    //setting
    while (function == 3)
    {
      /* code */
    }
    //reset
    while (function == 4)
    {
      function = 0;
    }

  }
}

