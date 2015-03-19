/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Interpolation utilities
*/

#include "main.h"
#include "error.h"
#include "interpolator.h"

namespace fbox {
namespace math {


////////////////////////////////////////
// linear_interpolator
////////////////////////////////////////

double linear_interpolator::operator() (double _x) const
{
	int i0,i1;
	double v;

	// check bounds
	if (_x <= mr_table.front().first)
		v = mr_table.front().second;
	else if (_x >= mr_table.back().first) 
		v = mr_table.back().second;
	else
	{
		sorted_find_adaptor adapt(mr_table);
		i0 = sorted_find(adapt,_x);
		i1 = i0 + 1;
		v = interp1(_x,mr_table[i0].first,mr_table[i1].first,mr_table[i0].second,mr_table[i1].second);
	}

	return v;
}


double linear_interpolator::interp1(double x,double x0,double x1,double y0,double y1) const
{
	double dif(x1-x0);
	if(dif == 0.) return y0;
	return (x-x0) * (y1-y0)/dif + y0;
}



////////////////////////////////////////
// cubic_spline_interpolator
////////////////////////////////////////

cubic_spline_interpolator::cubic_spline_interpolator(
	const table_type& _table,
	double _start,
	double _end)
  :	table_interpolator_impl(_table),
	m_start(_start),
	m_end(_end) 
{
	if (mr_table.size() > 1) make_spline();
}


void cubic_spline_interpolator::check_table()
{
	if (mr_table.size() <= 1) throw error("Table has 1 or less elements");

	// check axis is sorted
	table_type::const_iterator itr = mr_table.begin();
	table_type::const_iterator end = mr_table.end();
	
	double v1,v0 = itr->first;
	for (++itr; itr != end; ++itr)
	{
		v1 = itr->first;
		if (v1 + TINY < v0) throw error("Table is not sorted");
		v0 = v1;
	}
}


void cubic_spline_interpolator::make_spline()
{
	check_table();

	size_type sz = mr_table.size();

	m_deriv2.resize(sz);
	
	double p,qn,sig,un;
	int n = int(sz);
	std::vector<double> u(n);

	if (m_start > 0.99e30)
	{
		m_deriv2[0] = 0.0;
		u[0] = 0.0; // natural spline
	}
	else 
	{
		m_deriv2[0] = -0.5;
		
		u[0] = ( 3.0 / (mr_table[1].first - mr_table[0].first) ) * 
			( (mr_table[1].second - mr_table[0].second) / (mr_table[1].first - mr_table[0].first) - m_start );
	}

	for(int i=1; i <= n-2; i++) 
	{
		sig = (mr_table[i].first - mr_table[i-1].first) / (mr_table[i+1].first - mr_table[i-1].first);
		p = sig * m_deriv2[i-1] + 2.0;
		m_deriv2[i] = (sig-1.0) / p;
		
		u[i] = (mr_table[i+1].second - mr_table[i].second) / (mr_table[i+1].first - mr_table[i].first) -
			(mr_table[i].second - mr_table[i-1].second) / (mr_table[i].first - mr_table[i-1].first);
		
		u[i] = (6.0 * u[i] / (mr_table[i+1].first - mr_table[i-1].first) - sig * u[i-1]) / p;
	}

	if (m_end > 0.99e30)
	{
		qn = 0.0;
		un = 0.0; // natural spline
	}
	else 
	{
		qn = 0.5;
		un = ( 3.0 / (mr_table[n-1].first - mr_table[n-2].first) ) * 
			( m_end - (mr_table[n-1].second - mr_table[n-2].second) / (mr_table[n-1].first - mr_table[n-2].first) );
	}

	m_deriv2[n-1] = (un - qn * u[n-2]) / (qn * m_deriv2[n-2] + 1.0);

	for (int k = n-2; k >= 0; k--)
	{
		m_deriv2[k] = m_deriv2[k] * m_deriv2[k+1] + u[k];
	}

}


double cubic_spline_interpolator::operator() (double _x) const 
{	
	if (!m_deriv2.size()) throw error("cubic-spline table is not valid");
	
	double a,b,h;

	int klo = 0;
	int khi = int(mr_table.size() - 1);
	int k;

	if(_x > mr_table[khi].first)		
	{
		if( m_end > 0.99e30 )
		{
			klo = khi - 1;
			h = mr_table[khi].first - mr_table[klo].first;
			a = (mr_table[khi].first - _x) / h;
			b = (_x - mr_table[klo].first) / h;
			return (a * mr_table[klo].second + b * mr_table[khi].second - a * m_deriv2[klo] * (h*h) / 6.0);
	
		}
		else 
		{
			return ( mr_table[khi].second + m_end * (_x - mr_table[khi].first) );
		}

	}

	else if(_x < mr_table[klo].first)	
	{
		if ( m_start > 0.99e30 )
		{
			khi = 1;
			h = mr_table[khi].first - mr_table[klo].first;
			a = (mr_table[khi].first - _x) / h;
			b = (_x - mr_table[klo].first) / h;
			return (a * mr_table[klo].second + b * mr_table[khi].second - a * m_deriv2[khi] * (h*h) / 6.0);
		}
		else
		{
			return ( mr_table[klo].second + m_start * (_x - mr_table[klo].first) );
		}
	}

	while ( khi-klo > 1 ) 
	{
		k = (khi + klo) >> 1;
		if (mr_table[k].first > _x) 
		{
			khi = k;
		}
		else 
		{
			klo = k;
		}
	}

	h = mr_table[khi].first - mr_table[klo].first;
	a = (mr_table[khi].first - _x) / h;
	b = (_x - mr_table[klo].first) / h;

	return ( a * mr_table[klo].second + b * mr_table[khi].second + ((a*a*a-a) * m_deriv2[klo] + (b*b*b-b) * m_deriv2[khi]) * (h*h) / 6.0 );
}

} // namespace math
} // namespace fbox

