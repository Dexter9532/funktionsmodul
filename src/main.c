#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdint.h>


#define MOSI 3
#define SCK 5
#define SS 2


/*void execute(uint8_t cmd, uint8_t data){

    PORTB &= ~(1<<SS);

    SPDR = cmd;
    while(!(SPSR & (1<<SPIF)));

    SPDR = data;
    while(!(SPSR & (1<<SPIF)));

    PORTB |= (1<<SS);
}*/

void execute(uint8_t cmd,uint8_t data)
  {
      PORTB &= ~(1<<SS);
  
      SPDR = cmd;
      while(!(SPSR & (1<<SPIF)));
  
      SPDR = data;
      while(!(SPSR & (1<<SPIF)));
  
      PORTB |= (1<<SS);
  }

  int main(void)
  {
      DDRB = (1<< MOSI)|(1<<SCK)|(1<<SS);
      SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
      DDRD = 0;
  
      execute(0x09,0x00); //decode=0
  
      execute(0x0B,0x07);  //nmr of displays=8
      execute(0x0C,0x01); // turn on screans
      execute(0x0A,0x05); //intensity
    while (1) 
    { 
    execute(0x01, 0x07);
    
    }
}