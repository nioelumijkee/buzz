**buzz~** - subctractive synthesis without filters 


################################ buzz~ #########################################

buzz~ generates a signal from a set of harmonically related partials. 
Amplitude and number of these partials are controllable. 

inlet frequency:
Sets the fundamental frequency in Hz.

inlet a: 
The amplitudes of the partials are controlled by second inlet. Each sine
component of the signal is scaled by pow(a, k).

phase:
Sets the phase of the internal phasor object.

h: 
The highest partial is controlled by a value called "h" which sets the highest
generated (integer) multiple of the fundamental frequency. 

################################################################################

CREDITS

The theory behind buzz~ was taken from:
John Lazzaro and John Wawrzynek: "Subtractive Synthesis without Filters"
This document is available at http://www.cs.berkeley.edu/~lazzaro/sa/pubs/

Most of the code is copyrighted by Miller Puckette as it is the code for the
phasor~ generator in PD found at http://www-crca.ucsd.edu/~msp/software.html

Making this code work as a buzz~ object was done by me, Frank Barknecht
<frank.barknecht@netcologne.de>

As most of buzz~ is taken from code by Miller Puckette, buzz~ has the same
copyright as PD. This license can be found in the file COPYRIGHT.

################################################################################
