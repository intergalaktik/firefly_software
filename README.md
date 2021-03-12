# FireFly

### Sender

    [x] Using onboard DIP SW to set sender to passtrue data or to send messages alone
    [x] Implementing few test messages on sender
    
### Receiver    
    
    [x] WDT on receiver - if message is not received in 1 second reset board
    [x] Set panel ID with DIP SW
    [x] Light blink on reboot

### Messages

    [x] Set range or single LED on range or single panel to WHITE value - 0 value will set it to no light
    [x] Set range or single LED on range or single panel to RGB value - 0 on all values will set it to no light
    [x] On range or single panel set all LEDS at once (sending all RGB values in one message)
    [x] Set range or single LED on range or single panel to one of 16 grops
    [x] Send group color change message RGB or WHITE
