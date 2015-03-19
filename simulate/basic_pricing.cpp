/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Some basic pricing agents
*/

#include "basic_pricing.h"
#include "../core/option.h"

namespace fbox {
namespace simulate {


//////////////////////////////////////////////////////
// flows
//////////////////////////////////////////////////////

void flows::setup(
	double_agent_ptr _index,
	const time_type& _start,
	const time_type& _end,
	const duration_type& _period,
	double _multiplier,
	double _margin,
	double _payout,
	double _dcc_ratio)
{
	clear_connected();
	if (_index) connect(mp_index = _index);
	if (_period >= 1.) set_schedule(_start,_end,_period,_multiplier,_margin,_payout,_dcc_ratio);
}


void flows::set_schedule(
	const time_type& _start,
	const time_type& _end,
	const duration_type& _period,
	double _multiplier,
	double _fixed_amount,
	double _payout,
	double _dcc_ratio)
{
	if (_period < 1.0) throw error("Period must be 1 day or longer");
	if (_end <= _start) throw error("Schedule end date occurs after start date");

	m_leg.clear();
	
	time_type t0 = _start;
	time_type t1 = _start + _period;
	while(t1 < _end - .1)
	{
		m_leg.push_back( flow_type(t0,t0,t1,t1,_multiplier,_fixed_amount,0.0,(t1-t0)/_dcc_ratio) );

		t0 = t1;
		t1 += _period;
	}

	m_leg.push_back( flow_type(t0,t0,_end,_end,_multiplier,_fixed_amount,_payout,(t1-t0)/_dcc_ratio) );
}


size_type flows::size()
{
	return m_leg.size();
}


void flows::clear()
{
	m_leg.clear();
}


void flows::add(
	const time_type& _fix,
	const time_type& _start,
	const time_type& _end,
	const time_type& _pay,
	double _multiplier,
	double _margin,
	double _principal,
	double _yf)
{
	if (_yf < 0.0) _yf = (_end - _start) / 365.0;
	m_leg.push_back( flow_type(_fix,_start,_end,_pay,_multiplier,_margin,_principal,_yf) );
}


void flows::add(
	const time_type& _start,
	const time_type& _end,
	double _multiplier,
	double _margin,
	double _principal,
	double _yf)
{
	add(_start,_start,_end,_end,_multiplier,_margin,_principal,_yf);
}


void flows::add(
	const time_type& _pay,
	double _principal)
{
	m_leg.push_back( flow_type(_pay,_pay,_pay,_pay,0.0,0.0,_principal,0.0) );
}


void flows::dump(std::ostream& _strm) const
{
	std::string n = fbox::type_id(*this,true);
	fbox::xml::make_tag(_strm,n,false)
		.attr("index",mp_index);

	flows_vec_type::const_iterator itr = m_leg.begin();
	flows_vec_type::const_iterator end = m_leg.end();
	for(; itr != end; ++itr)
	{
		fbox::xml::make_tag(_strm,"coupon")
			.attr("fix",itr->fix)
			.attr("start",itr->start)
			.attr("end",itr->end)
			.attr("pay",itr->pay)
			.attr("multiplier",itr->multiplier)
			.attr("margin",itr->margin)
			.attr("principal",itr->principal)
			.attr("yf",itr->yf);
	}

	_strm << "</" << n << '>';
}


void flows::init_impl()
{
	// set starting iterator
	m_fitr = m_pitr = m_leg.begin();
	seek(m_start);
	m_fitr0 = m_fitr;
	m_pitr0 = m_pitr;
	m_state = 0.0;
}


void flows::reset_impl()
{
	m_matured = false;
	m_fitr = m_fitr0;
	m_pitr = m_pitr0;
	update_impl();
}


bool flows::update_impl()
{
	if (m_matured)
	{
		m_state = 0.0;
		return false;
	}

	m_state = seek(time()); // flows since last update
	if (m_pitr == m_leg.end()) m_matured = true;

	return true;
}


double flows::seek(const time_type& _time)
{
	flows_vec_type::iterator end = m_leg.end();

	if (mp_index)
	{
		double r = mp_index->state();
		while(m_fitr != end && m_fitr->fix <= _time + .1)
		{
			m_fitr->fixing = r;
			++m_fitr;
		}
	}

	double v = 0.0;
	while(m_pitr != end && m_pitr->pay <= _time + .1)
	{
		v += m_pitr->principal + (m_pitr->multiplier * m_pitr->fixing + m_pitr->margin) * m_pitr->yf;
		++m_pitr;
	}

	return v;
}



//////////////////////////////////////////////////////
// portfolio_events
//////////////////////////////////////////////////////

void portfolio_events::setup(
	portfolio_events::counter_ptr _counter,
	double_agent_ptr _intensity,
	int _impact)
{
	clear_connected();
	connect(mp_counter = _counter);
	connect(mp_h = _intensity);
	m_impact = _impact;
}


void portfolio_events::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
		.attr("impact",m_impact)
		.attr("counter",mp_counter)
		.attr("intensity",mp_h);
}


void portfolio_events::init_impl()
{
	if(!mp_counter || !mp_h) throw error("portfolio_events agent not set correctly");

	m_state = 0;
}


bool portfolio_events::update_impl()
{
	size_type n0 = mp_counter->state();
	double dt = time_interval() / 365.0;
	double h = mp_h->state();
	
	// dp		event probability of each individual name since the last update
	// p0k		probability of 0,1,... or k events (k starts at 0)
	// ln0k		logarithm of the p0k
	// pk1		probability of k+1 or more events
	
	size_type n = n0;
	double dp = 1.0 - std::exp(-h*dt);
	double ln0k = n * std::log(1.0 - dp);
	double p0k = ln0k > -20.0 ? std::exp(ln0k) : 0.0;
	double pk1 = 1. - p0k;

	// check events while there are still survivors and event probability is higher than 1e-5
	while (n && pk1 > 1e-5)
	{
		// no event, stop recursion
		if (rnd() > pk1) break;

		// probability for next recursion step
		ln0k += std::log( n * dp / (n0 - n + 1.0) / (1.0 - dp) );
		if (ln0k > -20.0) // slight improvement in efficiency from avoiding recalculation of exp
		{
			p0k += std::exp(ln0k); // WARNING!!! can't understand why += (instead of =) seems to work
			pk1 = 1.0 - p0k;
		}

		n += m_impact;
	}

	mp_counter->setup(n);
	m_state = n0 - n;

	return true;
}


//////////////////////////////////////////////////////
// black_scholes
//////////////////////////////////////////////////////


void black_scholes::setup(
	double_agent_ptr _forward,
	double_agent_ptr _volatility,
	double _notional,
	char _call_put,
	double _strike,
	const time_type& _expiry)
{
	clear_connected();
	connect(mp_fwd = _forward);
	connect(mp_vol = _volatility);

	m_notional = _notional;
	m_call_put = _call_put;
	m_strike = _strike;
	m_expiry = _expiry;
}


void black_scholes::reset_impl()
{
	update_impl();
}


bool black_scholes::update_impl()
{
	if (m_time >= m_expiry)
	{
		m_state = 0.0;
		return false;
	}

	double t = (m_expiry - m_time) / 365.0;
	double x = mp_fwd->state();
	double v = mp_vol->state();

	m_state = m_notional * fbox::finance::black_scholes(m_call_put,m_strike,t,x,v,1.0);

	return true;
}



} // namespace simulate
} // namespace fbox

