/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Instrument agents
*/

#include "instruments.h"
#include "../core/option.h"


namespace fbox {
namespace simulate {
namespace instruments{


//////////////////////////////////////////////////////
// instrument_state
//////////////////////////////////////////////////////

instrument_state& instrument_state::operator=(const instrument_state& _state)
{
	if (&_state != this)
	{
		value = _state.value;
		flow = _state.flow;
		matured = _state.matured;
	}
	return *this;
}


instrument_state& instrument_state::operator+=(const instrument_state& _state)
{
	value += _state.value;
	flow += _state.flow;
	matured = matured && _state.matured;
	return *this;
}


instrument_state& instrument_state::operator*=(double _weight)
{
	value *= _weight;
	flow *= _weight;
	return *this;
}


instrument_state& instrument_state::operator/=(double _weight)
{
	value /= _weight;
	flow /= _weight;
	return *this;
}


instrument_state operator+(const instrument_state& _state0,const instrument_state& _state1)
{
	instrument_state s(_state0);
	s += _state1;
	return s;
}


instrument_state operator*(const instrument_state& _state,double _weight)
{
	instrument_state s(_state);
	s *= _weight;
	return s;
}


instrument_state operator/(const instrument_state& _state,double _weight)
{
	instrument_state s(_state);
	s /= _weight;
	return s;
}


//////////////////////////////////////////////////////
// basic_instrument
//////////////////////////////////////////////////////


void basic_instrument::setup(
	double_agent_ptr _leg,
	double_agent_ptr _value,
	boolean_agent_ptr _matured)
{
	mp_matured.reset();
	clear_connected();
	connect(mp_flows = _leg);
	connect(mp_value = _value);
	if (_matured) connect(mp_matured = _matured);
}


void basic_instrument::reset_impl()
{
	update_impl();
}


bool basic_instrument::update_impl()
{
	m_state.flow = mp_flows->state();
	m_state.value = mp_value->state();
	if (mp_matured) m_state.matured = mp_matured->state();

	return true;
}



//////////////////////////////////////////////////////
// cash
//////////////////////////////////////////////////////


void cash::setup(
	double_agent_ptr _rate,
	double _initial_value,
	double _loan_spread,
	double _deposit_spread)
{
	m_state.clear();
	m_state.value = _initial_value;
	m_loan = _loan_spread;
	m_depo = _deposit_spread;

	clear_connected();
	connect(mp_rate = _rate);
}


void cash::transaction(double _value)
{
	m_local_flow -= _value;
	m_state.value += _value;
}


void cash::connect_flow(double_agent_ptr _agent)
{
	connect(_agent);
	m_flows.push_back(_agent);
}


void cash::clear_flows()
{
	m_flows.clear();
	clear_connected();
	connect(mp_rate); // reconnect rate after taking out the flows
}


void cash::init_impl()
{
	if(!mp_rate) throw error("cash_account agent not set correctly");

	m_initial = m_state.value;
	m_last_rate = get_rate();
	m_local_flow = 0.0;
}


void cash::reset_impl()
{
	m_last_rate = get_rate();
	m_local_flow = 0.0;
	update_impl();
}


bool cash::update_impl()
{
	// Note: m_local_flows is designed to allow for allow for synchronous (loop below) and
	// asynchronous (through flow_connector) setup of flows 
	if (m_flows.size())
	{
		std::vector<double_agent_ptr>::iterator itr = m_flows.begin();
		std::vector<double_agent_ptr>::iterator end = m_flows.end();
		for (; itr != end; ++itr) transaction((*itr)->state());
	}

	double dt = time_interval() / 365.0;
	double r = get_rate();

	m_state.value *= (std::exp(r * dt) + std::exp(m_last_rate * dt)) / 2.0;
	m_last_rate = r;

	m_state.flow = m_local_flow;
	m_local_flow = 0.0;

	return true;
}


double cash::get_rate() const
{
	return mp_rate->state() + (m_state.value >= 0? m_depo : m_loan);
}


void cash::flow_connector::setup(shared_ptr<cash> _account)
{
	mp_account = _account; // _account isn't connected through standard mechanism to allow assynchronous relation
	m_state = 0.0;
}


void cash::flow_connector::reset_impl()
{
	update_impl();
}


bool cash::flow_connector::update_impl()
{
	linked_policy::linked_list_type::iterator itr = m_linked_policy.begin();
	linked_policy::linked_list_type::iterator end = m_linked_policy.end();
	for (; itr != end; ++itr) mp_account->transaction((*itr)->state());

	if (mp_account->time() == m_time) // if account already up to date, then need to fix state.flow
	{
		mp_account->m_state.flow += mp_account->m_local_flow;
		mp_account->m_local_flow = 0;
	}

	return true;
}



//////////////////////////////////////////////////////
// portfolio
//////////////////////////////////////////////////////


void portfolio::clear()
{
	m_names.clear();
	mp_cash.reset();
}


void portfolio::set_cash_account(
	double_agent_ptr _rate,
	double _initial_value,
	double _loan_spread,
	double _deposit_spread)
{
	// !!!! WARNING !!!!
	// this function should seek through the connected agents list and delete any previously connected mp_cash
	// !!!! WARNING !!!!

	mp_cash.reset( new fbox::simulate::instruments::cash );
	mp_cash->setup(_rate,_initial_value,_loan_spread,_deposit_spread);
	connect(mp_cash);
}


void portfolio::add_instrument(
	instrument_agent_ptr _instrument,
	double _weight)
{
	double_agent_ptr w( new constant<double>(_weight) );
	add_instrument(_instrument,w);
}


void portfolio::add_instrument(
	instrument_agent_ptr _instrument,
	double_agent_ptr _weight)
{
	m_names.push_back( item_type(_instrument,_weight) );
	
	connect(_instrument);
	connect(_weight);
}


void portfolio::reset_impl()
{
	update_impl();
}


bool portfolio::update_impl()
{
	m_state.clear();
	vec_type::iterator itr = m_names.begin();
	vec_type::iterator end = m_names.end();
	for(; itr != end; ++itr) m_state += itr->first->state() * itr->second->state();

	if (mp_cash)
	{
		mp_cash->transaction(m_state.flow);
		m_state.flow = 0.0; // instrument leg are shown on the cash ledger

		m_state.value += mp_cash->state().value;
	}

	return !m_state.matured;
}



//////////////////////////////////////////////////////
// fixed_leg
//////////////////////////////////////////////////////

void fixed_leg::setup(
	yield_curve_ptr _yc,
	const time_type& _start,
	const time_type& _end,
	const duration_type& _period,
	double _amount,
	double _payout)
{
	clear_connected();
	connect(mp_yc = _yc);

	if (_period >= 1.) set_schedule(_start,_end,_period,_amount,_payout);
}


void fixed_leg::set_schedule(
	const time_type& _start,
	const time_type& _end,
	const duration_type& _period,
	double _amount,
	double _payout)
{
	if (_period < 1.0) throw error("Period must be 1 day or longer");
	if (_end <= _start) throw error("Schedule end date occurs after start date");

	m_flows.clear();
	for(time_type t = _start + _period; t < _end - .1; t += _period) m_flows.push_back( flow_type(t,_amount) );
	m_flows.push_back( flow_type(_end,_amount) );
	m_flows.back().amount += _payout;
}


size_type fixed_leg::size()
{
	return m_flows.size();
}


void fixed_leg::clear()
{
	m_flows.clear();
}


void fixed_leg::add(const time_type& _time,double _amount)
{
	m_flows.push_back( flow_type(_time,_amount) );
}


void fixed_leg::init_impl()
{
	if(!mp_yc) throw error("leg agent not set correctly");

	// set starting iterator
	m_itr = m_flows.begin();
	seek(m_start);
	m_itr0 = m_itr;

	m_state.value = m_state.flow = 0.0;
	m_state.matured = (m_itr == m_flows.end());
}


void fixed_leg::reset_impl()
{
	m_itr = m_itr0;
	update_impl();
}


bool fixed_leg::update_impl()
{
	if (m_state.matured)
	{
		m_state.flow = m_state.value = 0.0;
		return false;
	}

	m_state.value = 0.0;
	m_state.flow = seek(time()); // leg since last update
	
	flows_vec_type::const_iterator end = m_flows.end();
	if (m_itr == end) 
	{
		m_state.matured = true;
	}
	else
	{
		flows_vec_type::const_iterator itr = m_itr; // points to the next flow
		for(; itr != end; ++itr)
			m_state.value += itr->amount * mp_yc->discount(itr->time);
	}

	return true;
}


double fixed_leg::seek(const time_type& _time)
{
	double v = 0.0;
	flows_vec_type::const_iterator end = m_flows.end();

	while(m_itr != end && m_itr->time <= _time + .1)
	{
		v += m_itr->amount;
		++m_itr;
	}

	return v;
}


/*
//////////////////////////////////////////////////////
// float_leg
//////////////////////////////////////////////////////

void float_leg::setup(
	yield_curve_ptr _yc,
	const time_type& _start,
	const time_type& _end,
	const duration_type& _period,
	double _multiplier,
	double _rate,
	double _payout,
	double _dcc);
{
	clear_connected();
	connect(mp_yc = _yc);

	if (_period >= 1.) set_schedule(_start,_end,_period,_multiplier,_rate,_payout,_dcc);
}


void float_leg::set_schedule(
	const time_type& _start,
	const time_type& _end,
	const duration_type& _period,
	double _multiplier,
	double _rate,
	double _payout,
	double _dcc);
{
	if (_period < 1.0) throw error("Period must be 1 day or longer");
	if (_end <= _start) throw error("Schedule end date occurs after start date");

	m_multiplier = _multiplier;
	m_rate = _rate;
	m_final = _payout - m_multiplier; // final payment adjustment

	m_flows.clear();
	time_type last = _start;
	for(time_type t = _start + _period; t <= _end + .01; t += _period)
	{
		yf = (t - last).days() / _dcc;
		m_flows.push_back( flow_type(t,yf) );

		last = t;
	}
}


bool float_leg::update_impl()
{
	if (m_state.matured)
	{
		m_state.flow = m_state.value = 0.0;
		return false;
	}

	m_state.value = 0.0;
	m_state.flow = seek(time()); // leg since last update
	
	flows_vec_type::const_iterator end = m_flows.end();
	if (m_itr == end) 
	{
		m_state.matured = true;
	}
	else
	{
		flows_vec_type::const_iterator itr = m_itr; // points to the next flow
		for(; itr != end; ++itr)
			m_state.value += itr->amount * mp_yc->discount(itr->time);
	}

	return true;
}


double fixed_leg::seek(const time_type& _time)
{
	double v = 0.0;
	flows_vec_type::const_iterator end = m_flows.end();

	while(m_itr != end && m_itr->time <= _time + .1)
	{
		v += m_itr->amount;
		++m_itr;
	}

	return v;
}
*/


//////////////////////////////////////////////////////
// risky_leg
//////////////////////////////////////////////////////

void risky_leg::setup(
	yield_curve_ptr _yc,
	yield_curve_ptr _sc,
	double _recovery)
{
	fixed_leg::setup(_yc);
	connect(mp_sc = _sc);
	m_recovery = _recovery;
}


bool risky_leg::update_impl()
{
	if (m_state.matured)
	{
		m_state.flow = m_state.value = 0.0;
		return false;
	}

	m_state.value = 0.0;
	m_state.flow = seek(time()); // leg since last update
	
	// check normal termination
	flows_vec_type::const_iterator end = m_flows.end();
	if (m_itr == end)
	{
		m_state.matured = true;
	}
	else
	{
		// check event driven termination
		double h = -std::log(mp_sc->discount(m_time+1)) * 365.0;
		double dt = time_interval() / 365.0;
		double p = std::exp(-h * dt);
		if (rnd() > p)
		{
			m_state.flow += m_recovery;
			m_state.matured = true;
		}
		else // if still live calculate present value
		{
			flows_vec_type::const_iterator itr = m_itr; // points to the next flow
			for(; itr != end; ++itr)
				m_state.value += itr->amount * mp_yc->discount(itr->time) * mp_sc->discount(itr->time);
		}
	}

	return true;
}



//////////////////////////////////////////////////////
// portfolio_fixed_leg
//////////////////////////////////////////////////////

void portfolio_fixed_leg::setup(
	yield_curve_ptr _yc,
	yield_curve_ptr _sc,
	portfolio_events::counter_ptr _counter,
	double _recovery_rate)
{
	fixed_leg::setup(_yc);
	connect(mp_sc = _sc);
	connect(mp_counter = _counter);
	m_recovery_rate = _recovery_rate;
}


void portfolio_fixed_leg::init_impl()
{
	if(!mp_yc || !mp_sc || !mp_counter) throw error("portfolio_leg agent not set correctly");
}


void portfolio_fixed_leg::reset_impl()
{
	fixed_leg::reset();
	m_last_pv = m_state.value;
	m_last_size = mp_counter->state();
}


bool portfolio_fixed_leg::update_impl()
{
	size_type sz = mp_counter->state();

	if (m_state.matured || sz == 0)
	{
		m_state.matured = true;
		m_state.flow = m_state.value = 0.0;
		return false;
	}

	m_state.value = 0.0;
	m_state.flow = seek(time()); // leg since last update
	m_state.flow *= (sz + m_last_size) / 2.0; // assume leg are paid on the averate of members now and in the last period	
	
	flows_vec_type::const_iterator end = m_flows.end();
	if (m_itr == end) 
	{
		m_state.matured = true;
	}
	else
	{
		flows_vec_type::const_iterator itr = m_itr; // points to the next flow
		for(; itr != end; ++itr)
			m_state.value += itr->amount * mp_yc->discount(itr->time) * mp_sc->discount(itr->time);

		// apply recovery to terminated events on the average value at this and the last node
		m_state.flow += (m_state.value + m_last_pv) / 2.0 * m_recovery_rate * (m_last_size - sz);

		// scale value by size of portfolio
		m_state.value *= sz;
	}

	m_last_size = sz;
	m_last_pv = m_state.value;

	return true;
}



//////////////////////////////////////////////////////
// portfolio_event_leg
//////////////////////////////////////////////////////

void portfolio_event_leg::setup(
	yield_curve_ptr _yc,
	yield_curve_ptr _sc,
	shared_ptr<portfolio_events> _events,
	portfolio_events::counter_ptr _counter,
	const time_type& _maturity,
	double _payout)
{
	clear_connected();
	connect(mp_yc = _yc);
	connect(mp_sc = _sc);
	connect(mp_counter = _counter);
	connect(mp_events = _events);
	m_maturity = _maturity;
	m_payout = _payout;

	m_istep = 30; // approximately monthly NPV integration steps
}


void portfolio_event_leg::init_impl()
{
	if(!mp_yc || !mp_sc || !mp_counter || !mp_events) throw error("portfolio_event_leg agent not set correctly");
}


bool portfolio_event_leg::update_impl()
{
	m_state.flow = m_state.value = 0.0;
	size_type sz = mp_counter->state();

	if (m_state.matured || sz == 0)
	{
		m_state.matured = true;
		return false;
	}

	if (time() >= m_maturity) 
	{
		m_state.matured = true;
	}
	else
	{
		size_type events = mp_events->state();
		m_state.flow = events * m_payout;

		double s0(1.0),s1(1.0);
		for(time_type t = time(); t < m_maturity; t += m_istep)
		{
			s1 = mp_sc->discount(t);
			m_state.value += mp_yc->discount(t) * (s0 - s1);
			s0 = s1;
		}
		m_state.value *= m_payout * sz;
	}

	return true;
}


//////////////////////////////////////////////////////
// forward
//////////////////////////////////////////////////////


void forward::setup(
	yield_curve_ptr _yc,
	double_agent_ptr _security,
	double_agent_ptr _notional,
	double_agent_ptr _strike,
	const time_type& _start,
	const time_type& _expiry)
{
	clear_connected();
	connect(mp_yc = _yc);
	connect(mp_sec = _security);
	connect(mp_notional = _notional);
	connect(mp_strike = _strike);
	m_start = _start;
	m_expiry = _expiry;
	m_has_started = false;
}


void forward::reset_impl()
{
	update_impl();
}


bool forward::update_impl()
{
	if (m_state.matured)
	{
		m_state.value = m_state.flow = 0.0;
		return false;
	}

	if (!m_has_started & (m_time >= m_start))
	{
		m_actual_strike = mp_strike->state();
		m_actual_notional = mp_notional->state();
		m_has_started = true;
	}

	double x = mp_sec->state();
	
	if (m_time >= m_expiry)
	{
		m_state.value = 0.0;
		m_state.flow = m_actual_notional * (x - m_actual_strike);
		m_state.matured = true;
	}
	else
	{
		double df = mp_yc->discount(m_expiry);
		m_state.value = m_actual_notional * (x - m_actual_strike) * df;
		m_state.flow = 0.0;
	}

	return true;
}


//////////////////////////////////////////////////////
// option
//////////////////////////////////////////////////////


void option::setup(
	yield_curve_ptr _yc,
	double_agent_ptr _vol,
	double_agent_ptr _security,
	double_agent_ptr _notional,
	char _call_put,
	double_agent_ptr _strike,
	const time_type& _start,
	const time_type& _expiry,
	model_type _model)
{
	forward::setup(_yc,_security,_notional,_strike,_start,_expiry);
	connect(mp_vol = _vol);

	m_cp = _call_put;
	m_model = _model;
}


void option::reset_impl()
{
	option::update_impl();
}


bool option::update_impl()
{
	if (m_state.matured)
	{
		m_state.value = m_state.flow = 0.0;
		return false;
	}

	if (!m_has_started & (m_time >= m_start))
	{
		m_actual_strike = mp_strike->state();
		m_actual_notional = mp_notional->state();
		m_has_started = true;
	}

	double x = mp_sec->state();

	if (m_time >= m_expiry)
	{
		m_state.value = 0.0;
		m_state.flow = m_actual_notional * fbox::finance::option_intrinsic(m_cp,m_actual_strike,x,1.0);
		m_state.matured = true;
	}
	else
	{
		double t = (m_expiry - m_time) / 365.0;
		double df = mp_yc->discount(m_expiry);
		double v = mp_vol->state();

		switch (m_model)
		{
			case NORMAL:
			m_state.value = m_actual_notional * fbox::finance::normal_black_scholes(m_cp,m_actual_strike,t,x/df,v,df);
			break;

			case BLACK_SCHOLES:
			m_state.value = m_actual_notional * fbox::finance::black_scholes(m_cp,m_actual_strike,t,x/df,v,df);
			break;

			default:
			throw error("Invalid option type (BLACK_SCHOLES/NORMAL)");
		}

		m_state.flow = 0.0;
	}

	return true;
}



} // namespace instruments
} // namespace simulate
} // namespace fbox

