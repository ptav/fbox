/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Simulation implementation for process framework
*/

#include "observer.h"
#include "../core/math.h"

namespace fbox {
namespace simulate {


//////////////////////////////////////////////////////
// Expectation
//////////////////////////////////////////////////////

void expectation::init()
{
	if (!mp_agent) throw error("Observer's target agent is not set");
	m_value = m_weight = 0.0;
}


void expectation::observe(double _weight)
{
	m_value += mp_agent->state() * _weight;
	m_weight += _weight;
}


void expectation::end()
{
	m_value /= m_weight;
}



//////////////////////////////////////////////////////
// Bounds
//////////////////////////////////////////////////////

void bounds::init()
{
	if (!mp_agent) throw error("Observer's target agent is not set");
	m_min = std::numeric_limits<double>::max();
	m_max = std::numeric_limits<double>::min();
}


void bounds::observe(double _weight)
{
	double v = mp_agent->state();
	m_min = std::min(m_min,v);
	m_max = std::max(m_max,v);
}



//////////////////////////////////////////////////////
// statistics 
//////////////////////////////////////////////////////


statistics::statistics (const statistics & _e)
:	observer(_e),
	m_expectation(_e.m_expectation),
	m_variance(_e.m_variance),
	m_stderr(_e.m_stderr),
	m_cum_weight(_e.m_cum_weight),
	m_max(_e.m_max),
	m_min(_e.m_min)
{}


void statistics::init()
{
	if (!mp_agent) throw error("Observer's target agent is not set");
	m_expectation = m_variance = m_stderr = m_cum_weight = 0.0;
	m_min = std::numeric_limits<double>::max();
	m_max = std::numeric_limits<double>::min();
}


void statistics::observe(double _weight)
{
	double v = mp_agent->state();

	m_expectation += v * _weight;
	m_variance += v * v * _weight;
	m_cum_weight += _weight;

	m_min = std::min(m_min,v);
	m_max = std::max(m_max,v);

	++m_stderr; // counts samples only
}


void statistics::end()
{
	m_expectation /= m_cum_weight;
	
	m_variance /= m_cum_weight;
	m_variance -= m_expectation * m_expectation;

	m_stderr = std::sqrt(m_variance / m_stderr);
}


double statistics ::standard_deviation() const
{
	if (math::is_zero(m_variance))
		return 0.0;
	else
		return std::sqrt(m_variance);
}



//////////////////////////////////////////////////////
// cross_moments
//////////////////////////////////////////////////////


cross_moments::cross_moments(const cross_moments& _o)
:	m_agents(_o.m_agents),
	m_expectation(_o.m_expectation),
	m_covar(_o.m_covar),
	m_samples(_o.m_samples),
	m_weight(_o.m_weight)
{}


void cross_moments::init()
{
	size_type sz = m_agents.size();
	if (sz < 1) throw error("cross_moment observer's target agents are not set");
	
	m_weight = 0.0;
	m_samples = 0;

	m_expectation.clear();
	m_expectation.resize(sz,0.0);
	
	m_covar.clear();
	m_covar.resize(sz,std::vector<double>(sz,0.0));
}


void cross_moments::observe(double _weight)
{
	typedef std::vector<double> dvec;
	typedef std::vector<dvec> dvec2;
	typedef std::vector<double_agent_ptr> avec;

	avec::iterator itr = m_agents.begin();
	avec::iterator end = m_agents.end();

	dvec::iterator e = m_expectation.begin();
	dvec2::iterator i0 = m_covar.begin();
	dvec2::iterator i = i0;

	double v;
	dvec::iterator j;
	avec::iterator a;
	for(; itr != end; ++itr, ++e, ++i)
	{
		v = (*itr)->state();
		*e += v * _weight;

		for(a = m_agents.begin(), j = i->begin(); a != itr; ++a,++j)
		{
			*j += v * (*a)->state() * _weight;
		}
		*j += v * v * _weight;
		
		m_weight += _weight;
		++m_samples;
	}
}


void cross_moments::end()
{
	size_type sz = m_agents.size();
	for(size_type i = 0; i < sz; ++i)
	{
		m_expectation[i] /= m_weight;
	}

	for(size_type i = 0; i < sz; ++i)
	{
		for(size_type j = 0; j <= i; ++j)
		{
			m_covar[i][j] /= m_weight;
			m_covar[i][j] -= m_expectation[i] * m_expectation[j];
		}
	}
}



double cross_moments::standard_error(size_type _i) const
{
	return std::sqrt(m_covar[_i][_i] / m_samples);
}

	
//////////////////////////////////////////////////////
// scenarios
//////////////////////////////////////////////////////


scenarios::scenarios(const scenarios& _e)
:	observer(_e),
	m_outcomes(_e.m_outcomes),
	m_weights(_e.m_weights)
{}


void scenarios::init()
{
	if (!mp_agent) throw error("Observer's target agent is not set");
	m_outcomes.clear();
	m_weights.clear();

}


void scenarios::observe(double _weight)
{
	m_outcomes.push_back( mp_agent->state() );
	m_weights.push_back( _weight );
}



//////////////////////////////////////////////////////
// histogram
//////////////////////////////////////////////////////

histogram::histogram(
	double_agent_ptr _agent,
	double _min,
	double _max,
	double _step,
	size_type _order,
	bool _pdf)
:	observer(_agent),
	m_min(_min),
	m_max(_max),
	m_step(_step),
	m_order(_order),
	m_pdf(_pdf),
	m_cum_weight(0.0)
{}


histogram::histogram(const histogram& _h)
:	observer(_h.mp_agent),
	m_min(_h.m_min),
	m_max(_h.m_max),
	m_step(_h.m_step),
	m_order(_h.m_order),
	m_pdf(_h.m_pdf),
	m_cum_weight(_h.m_cum_weight)
{}


void histogram::init()
{
	if (m_max <= m_min + math::TINY) throw error("Histogram interval is too small");

	m_bins.clear();
	m_probs.clear();
	for(double v = m_min; v <= m_max + fbox::math::TINY; v += m_step)
	{
		m_bins.push_back(v);
		m_probs.push_back(0.0);
	}

	m_cum_weight = 0.0;
}


void histogram::observe(double _weight)
{
	double v = mp_agent->state();

	size_type index;
	double remainder; 
	pos(v,index,remainder);
	
	size_type sz = m_probs.size();

	m_cum_weight += _weight;

	if (m_order == 1)
	{
		if (index >= sz)	m_probs[sz - 1] += _weight;
		else				m_probs[index] += _weight;
	}
	else if (m_order == 2)
	{
		double prob1 =	_weight * (1.0	- remainder);
		double prob2 =	_weight * remainder;
		
		if (index >= sz)	m_probs[sz - 1] += prob1;
		else				m_probs[index]+= prob1;

		if (index+1 >= sz)	m_probs[sz - 1] += prob2;
		else				m_probs[index + 1]+= prob2;
	}
	else
		throw error("Unsupported distribution fitting order");
}


//#define FOR_EACH(iterator,end_iterator,operation) \
//	for(; iterator != end_iterator; ++iterator) operation;
void histogram::end()
{
	double w = m_cum_weight;
	if (m_pdf) w *= m_step;

	std::vector<double>::iterator itr = m_probs.begin();
	std::vector<double>::iterator end = m_probs.end();
	for(;itr != end; ++itr) *itr /= w;
}


void histogram::pos(double _value,size_type& _grid,double& _remainder)
{
	_value = (_value - m_min) / m_step;
	if (_value >= 0)
	{
		_grid = static_cast<size_type>(_value);
		_remainder = _value - _grid;
	}
	else
	{
		_grid = 0;
		_remainder = 0.0;
	}
}


} // namespace simulate
} // namespace fbox

