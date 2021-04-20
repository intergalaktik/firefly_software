#include <avr/io.h>
#include <stdint.h>            // has to be added to use uint8_t

#include <avr/interrupt.h>    // Needed to use interrupts    

volatile uint8_t portbhistory = 0xFF;     // default is high because the pull-up

int main(void)
{
  // put your setup code here, to run once:
  //Set Pin for output
  DDRD |= (1 << PD1);
  //Set Pin for input
  DDRB &= ~(1 << PD4);
  //Set internal pull-up resistor (after setting pin for input)
  PORTB |= (1 << PD4);

  cli();
  PCICR |= (1 << PCIE0);      // Set PCIE0 to enable PCMSK0 scan.
  PCMSK0 |= (1 << PCINT4);    // Set PCINT0 (digital input 12) to trigger an interrupt on state change
  sei();                      // Set enable interrupts - we're ready to go

  while (1)
  {
    /*main program loop here */
  }
}

ISR(PCINT0_vect) {
  cli();
  uint8_t changedbits;

  changedbits = PINB ^ portbhistory;
  portbhistory = PINB;

  if (changedbits & (1 << PINB4))
  {
    /* PCINT4 changed */
    if (PINB & (1 << PINB4)) {
      PORTD |= (1 << PD1);
    }
    else {
      PORTD &= ~(1 << PD1);
    }
  }
  sei();
}
