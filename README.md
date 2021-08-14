# mbed-ZSC31014

Driver and initial attempt at a calibration setup for the [Renesas/IDT ZSC31014 strain gauge amplifier IC](https://www.renesas.com/us/en/document/dst/zsc31014-datasheet). Currently set up for running under [mBed OS](https://os.mbed.com/mbed-os/), but you really only need to be able to toggle a GPIO and read/write to an I2C bus (The IC doesn't support using SPI for this step). 

*Note*: You probably don't need a driver for simply reading values from the IC. It's a normal I2C read operation from the device's address, and depending on the number of bytes you read you can get just strain gauge information (send the address, read 2 bytes) or also temperature (read 1 or 2 additional bytes). See section 3.2 of the aforelinked datasheet. This driver is intended to help with setting up and calibrating the IC. 

To enter command mode (which allows you to actually read and write from/to the configuration/calibration registers), you'll need to send a command within a few hundred microseconds of powering the device. If you don't have much else on the same power supply you can probably just use a GPIO pin to power the IC, which is what this code assumes. 

## The Calibration Process

The calibration process is described in section 3.7 of the datasheet in much more detail than I'm going to go over here, so these are more tips and tricks and TL;DR overview. 

1. I would recommend running the `dumpEEPROM()` method and copying the result somewhere safe so that if you (or this driver) accidentally overwrites something unintended, you can recover the initial configuration (unless you accidentally run the lock EEPROM command, in which case your device can be bricked). The temperature sensing is factory-calibrated and evidently kind of a pain to recreate. 
2. The first hoop to jump through is actually getting the device in command mode. You need to send the `Start_CM` command within 1.5 milliseconds of applying power to the device, but after the device has had a chance to finish powering up. The delays in `startCommandMode` are what worked for me, but YMMV. 
3. The main things to set up initially are:

   - The I2C address (or set it up to use SPI mode), and whether to listen only on that address (`lockAddress`)
   - Whether to detect bridge faults (highly recommended for safety-critical applications)
   - The settings for the amplifier. I used the highest pre-amp gain, a low pre-amp offset (since I'm looking at a mostly unidirectional signal), long integration, and configured for a full bridge powered from the IC's `Bsink` pin. 
   - You need to start normal operation mode for the changes to take effect, and insert some delays beforehand for the writes to complete. 
   - You especially don't want to set the lock EEPROM bit and then not start normal operation mode, as that will skip writing the checksum on the EEPROM data but keep the EEPROM locked, bricking the IC along with whatever it's soldered to. 

4. Now you need to set the gain and offset. You probably want to set the gain first, and the offset second, because changing the gain changes the offset. 

   1. I start the device in normal operation mode and take a few readings to average.
   2. I then apply a known force to it, see how the reading changes, and adjust the gain to compensate. 
   3. I usually have to repeat this a few times in a sort of binary search pattern. 
   4. Finally you should be able to choose the offset you want and set the offset number accordingly.
   5. This doesn't always work quite how I would expect so you may need to hone in on the exact solution.
   6. You can also set adjustments for temperature compensation, but I haven't gotten that fancy yet. 

7. Now when you power on the device normally, you should be able to read from the I2C address you set (or SPI, I didn't try that) and get bridge data in the first two bytes (the first two bits are error indicators that should be zero, the remaining 14 bits are the reading, MSB first), and temperature data in the following bytes. 

## Things That Tripped Me Up

1. mBed uses 8-bit I2C addresses, and normal right-thinking people use 7-bit I2C addresses. This driver attempts to compensate for that discrepancy. 
2. Adding helpful `printf`s at the wrong place will make e.g. the `startCommandMode` method fail because they're sufficiently slow to throw off the timing. 
3. As described above, it's helpful to dump the contents of the EEPROM in case you screw something up (or in case I screwed something up in this code). 
4. I'm not a C++ programmer, so this might not be especially idiomatic C++ (to the extent that such a thing exists). 



