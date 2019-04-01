#ifndef LightScale_h
#define LightScale_h

#include <vector>

/*
This class is used to scale an integer light intensity to an output value.
It can optionaly apply gamma correction to make changes in perceived intensity linear.

Loopup tables are very common for for gama correction, because they are very quick.
However, they normally only contain 8 bit values to minimise the amount of RAM they consume.
The consequence of this is that at there are only two steps in brightness between 12.5% and 0%

This class solves the problem by creating a set of linear (y=mx+c) equations
that approximate to the required curve. 
When applying the conversion, a lookup is performed to select the correct equation.

8 lines give a good aproximation, and only need 32 bytes of RAM to hold the constants.
*/
class LightScale
{
public:
    LightScale( unsigned int maxIn, unsigned int maxOut, bool invertOut, float gamma );
    ~LightScale();


    unsigned int getValue(unsigned int intensity) const;

private:
    struct LinearCorrection
    {
        short scale;      // fixed point multiplier 0x100 = 1.0
        short offset;     // fixed point offset     0x100 = 1.0

        unsigned int calculate(unsigned int value) const;
    };


    std::vector<LinearCorrection> m_corrections;    // collection of linear correction cooefecients

    float calculateOutput(unsigned int value, unsigned int maxIn, unsigned int maxOut, bool invertOut, float gamma) const;

};







#endif   // LightScale_h

