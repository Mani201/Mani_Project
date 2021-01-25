import spidev
import time
import sys, os
spi_0 = spidev.SpiDev()
spi_0.open(0, 0)

#this fucntion can be used to find out the ADC value on ADC 0
def readadc_0(adcnum_0):
    if adcnum_0 > 7 or adcnum_0 < 0:
        return -1
x
    r_0 = spi_0.xfer2([1, 8 + adcnum_0 << 4, 0])
    adcout_0 = ((r_0[1] & 3) << 8) + r_0[2]
    return adcout_0

#first ADC setup on SPI port 1
spi_1 = spidev.SpiDev()
spi_1.open(0, 1)

#this fucntion can be used to find out the ADC value on ADC 1
def readadc_1(adcnum_1):


    if adcnum_1 > 7 or adcnum_1 < 0:
        return -1
    r_1 = spi_1.xfer2([1, 8 + adcnum_1 << 4, 0])
    adcout_1 = ((r_1[1] & 3) << 8) + r_1[2]
    return adcout_1

while 1:
    print "Port 0  | "  , "adc_0: " , str(readadc_0(1)).zfill(4) , " | " 
    time.sleep(1)



