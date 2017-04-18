/**
 * Created by Olavi Kamppari on 3/6/2017.
 */

/**
 *  File: GP2Y0A21.cpp
 *
 *  Sharp Optical sensor is intended for distances from 100 to 800 mm
 *  The manual is available at 
 *  http://www.sharp-world.com/products/device/lineup/data/pdf/datasheet/gp2y0a21yk_e.pdf
 *
 *  On page 5/9 there is a calibration curve that shows how the output voltage
 *  - is peaked at 3.2 V for distance of 80 mm
 *
 *  Here are the expected voltages for some distances in mm
 *      100: 2.25 V
 *      200: 1.30 V
 *      300: 0.90 V
 *      400: 0.75 V
 *      500: 0.60 V
 *      600: 0.50 V
 *      700: 0.45 V
 *      800: 0.40 V
 *
 *  I had calibrated an ultrasound sensor against a meausrement tape and
 *  observed that it was not very precise, but it was quite accurate.
 *  I did my own calibration of the ODS using the USS as a reference.
 *  I could observe that the ODS sensor had reading for the range
 *      from 65 mm to 4000 mm.
 *
 *  Under 65 mm the reading are non-monotonic and unusable.
 *  For distances over 1000 mm, the reading are not precise nor accurate,
 *  but still usable for object detection.
 *
 *  The inverse curve shown in Sharp document is not very smooth.  By using
 *  reasonable large sample set, and evaluating 1st, 2nd, and 3rd derivatives
 *  I created a graph to map the Arduino Mega 2560 ADC numbers directly to
 *  distance in mm.
 *
 *  In the odsCurve table, there is a distance in mm for 70 evenly distributed
 *  ADC values representing the range from 0 to 700.  Simple interpolation is 
 *  used to find the distance for values that are not there.
 *
 */
 
#include <GP2Y0A21.h>

#define ARRSIZE 70
#define ARRSIZE_1 (ARRSIZE - 1)

int16_t odsCurve[ARRSIZE] = {
    4227, 3362, 2672, 2132, 1717, 1402, 1162, 977, 832, 716, 623, 548, 488, 440,
    402, 372, 348, 328, 310, 293, 278, 264, 251, 239, 228, 218, 209, 201,
    194, 187, 180, 174, 168, 162, 156, 151, 146, 141, 137, 133, 129, 125,
    122, 119, 116, 113, 110, 107, 104, 101, 98, 95, 92, 90, 88, 86,
    84, 82, 80, 78, 76, 74, 72, 71, 70, 69, 68, 67, 66, 65};

int16_t GP2Y0A21distance(int16_t sensorReading) {
    int16_t    major = sensorReading / 10;
    int16_t    minor = sensorReading % 10;
    
    if (major + 1 > ARRSIZE_1) return odsCurve[ARRSIZE_1];
    if (major < 1) return odsCurve[0];

    int16_t    v1 = odsCurve[major];
    int16_t    v2 = odsCurve[major +1];

    return v1 - minor*(v1 - v2) / 10;
}