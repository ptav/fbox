/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Math utilities
*/

#include "math.h"
#include "error.h"

namespace fbox {
namespace math {


const double TINY(1e-16);


float round(const float& _number,const int _digits)
{
    float doComplete5i, doComplete5(_number * powf(10.0f, (float) (_digits + 1)));
    
    if(_number < 0.0f)
        doComplete5 -= 5.0f;
    else
        doComplete5 += 5.0f;
    
    doComplete5 /= 10.0f;
    modff(doComplete5, &doComplete5i);
    
    return doComplete5i / powf(10.0f, (float) _digits);
}


double round(double _value,int _digits)
{
    static const double doBase = 10.0;
    double doComplete5, doComplete5i;
    
    doComplete5 = _value * pow(doBase, (double) (_digits + 1));
    
    if(_value < 0.0)
        doComplete5 -= 5.0;
    else
        doComplete5 += 5.0;
    
    doComplete5 /= doBase;
    modf(doComplete5, &doComplete5i);
    
    return doComplete5i / pow(doBase, (double) _digits);
}


float sig_digits(float _value, int _digits)
{
    if(_digits < 1) throw error("sig_digits called with invalid number of digits");

    // log10f(0) returns NaN
    if(_value == 0.0f)
        return _value;
    
    int Sign;
    if(_value < 0.0f)
        Sign = -1;
    else
        Sign = 1;

    _value = fabsf(_value);
    float Powers = powf(10.0f, floorf(log10f(_value)) + 1.0f);

    return Sign * round(_value / Powers, _digits) * Powers;
}


void histogram(
	const std::vector<double>& bins,
	const std::vector<double>& series,
	std::vector<double>& out,
	const std::vector<double>& weights)
{
	int nbins = bins.size();
	bool usew = weights.size() > 0;

	// construct histogram
	out.resize(0);
	out.resize(nbins+1,0.0);
	for(size_type i = 0; i < series.size(); i++)
	{
		int idx = sorted_find(bins,series[i]);
		if (idx < 0 || series[i] > bins[idx] + TINY) ++idx; // catch values on the boundary
		
		if (usew)
			out[idx] += weights[i];
		else
			out[idx]++;
	}

	// normalise
	if (!usew)
		for(size_type i = 0; i < out.size(); ++i) out[i] /= series.size();
}

} // namespace math
} // namespace fbox

