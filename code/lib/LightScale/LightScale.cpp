#include "LightScale.h"

extern "C" {
    #include"fs_math.h"
}

const static int STEP_BITS = 5;

LightScale::LightScale()
{
    // No correction required
    m_corrections.resize(0);
}


LightScale::LightScale( unsigned int maxIn, unsigned int maxOut, bool invertOut, float gamma )
{
    if ( (gamma == 1.0f) && (maxIn == maxOut) && !invertOut )
    {
        // No correction required
        m_corrections.resize(0);
    }
    else
    {
        // Step size of 32
        int step = 1 << STEP_BITS;
        if (gamma == 1.0f)
        {
            // Gamma = 1, so just a single linear equations is required
            step = maxIn;
        }

        float previous = calculateOutput(0, maxIn, maxOut, invertOut, gamma);
        for (int intensity = step; intensity <= maxIn; intensity += step )
        {
            float next = calculateOutput(intensity, maxIn, maxOut, invertOut, gamma);

            // calculate equation that passes through both points
            float gradient = (next - previous) / step;
            float intercept = -gradient / intensity;

            // floating point gradient and intercept converted to fixed point integers with 8 fractional bits
            m_corrections.push_back( LinearCorrection { (gradient * 256.0f) +0.5f, (intercept * 256.0f) +0.5f } );
        }
    }
}


LightScale::~LightScale()
{
}



/*
Get the required output value for the given light intensity
*/
unsigned int LightScale::getValue(unsigned int intensity) const
{
    int corrected = intensity;
    if (m_corrections.size() == 0)
    {
        // No correction
    }
    else if (m_corrections.size() == 1)
    {
        corrected = m_corrections.front().calculate(intensity);
    }
    else
    {
        // Assume 8 bit intensity, with top 3 bits selecting index 
        unsigned int index = (intensity >> STEP_BITS);
        if (m_corrections.size() < index)
        {
            corrected = m_corrections[index].calculate(intensity);
        }
        else
        {
            // Just use the last equation
            corrected = m_corrections.back().calculate(intensity);
        }
    }
    
    // Constrain to 0..MaxOut?

    return corrected;
}



/*
Calculate the required output value for the given light intensity. 
This function is slow but accurate. It is used to generate the table of equations that the
quick and approximate 'getValue' function uses.
*/
float LightScale::calculateOutput(unsigned int value, unsigned int maxIn, unsigned int maxOut, bool invertOut, float gamma) const
{
    // Normalised intensity value
    float fValue = value / (float) maxIn;
    if (gamma != 1.0)
        fValue = fs_pow(fValue, gamma);

    // Scale for output
    fValue *= maxOut;

    if (invertOut)
        fValue = maxOut - fValue;

    return value;
}



unsigned int LightScale::LinearCorrection::calculate(unsigned int value) const
{
    // Apply Fixed point linear scaling
    long corrected = ((long)value * scale) + offset;

    // Convert to nearest integer and divide by 256
    return (corrected + 0x80) >> 8;
}