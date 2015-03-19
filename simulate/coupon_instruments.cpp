/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Coupon instrument agents
*/

#include "coupon_instruments.h"


namespace fbox {
namespace simulate {
namespace instruments {


//////////////////////////////////////////////////////
// coupon_impl
//////////////////////////////////////////////////////

void coupon_impl::setup(
	const time_type& _fix,
	const time_type& _start,
	const time_type& _end,
	const time_type& _pay,
	double _year_fraction)
{
	if (_pay < _fix) throw fbox::error("Payment date before fixing");
	if (_end < _start) throw fbox::error("Accrual end date before start");

	m_fix = _fix;
	m_accrual_start = _start;
	m_accrual_end = _end;
	m_pay = _pay;

	m_year_fraction = _year_fraction;
}



//////////////////////////////////////////////////////
// fixed_payment
//////////////////////////////////////////////////////

void fixed_payment::setup(
	yield_curve_ptr _yc,
	const time_type& _pay,
	double _amount)
{
	clear_connected();
	connect(mp_yc = _yc);

	m_amount = _amount;
	
	time_type t;
	coupon_impl::setup(t,t,t,_pay,1.0);
}


void fixed_payment::reset_impl()
{
	update_impl();
}


bool fixed_payment::update_impl()
{
	if (m_state.matured == true)
	{
		m_state.flow = m_state.value = 0.0;
		return false;
	}

	if (m_time >= m_pay)
	{
		m_state.flow = m_amount;
		m_state.value = 0.0;
		m_state.matured = true;
	}
	else
	{
		m_state.value = m_amount * mp_yc->discount(m_pay);
	}

	return true;
}



//////////////////////////////////////////////////////
// fixed_coupon
//////////////////////////////////////////////////////

void fixed_coupon::setup(
	yield_curve_ptr _yc,
	const time_type& _start,
	const time_type& _end,
	const time_type& _pay,
	double _rate,
	double _year_fraction)
{
	clear_connected();
	connect(mp_yc = _yc);

	m_amount = _rate * _year_fraction;

	time_type t;
	coupon_impl::setup(t,_start,_end,_pay,_year_fraction);
}



//////////////////////////////////////////////////////
// vanilla_float_coupon
//////////////////////////////////////////////////////

void vanilla_float_coupon::setup(
	yield_curve_ptr _yc,
	const time_type& _fix,
	const time_type& _start,
	const time_type& _end,
	const time_type& _pay,
	double _multiplier,
	double _year_fraction)
{
	clear_connected();
	connect(mp_yc = _yc);
	
	m_multiplier = _multiplier;

	coupon_impl::setup(_fix,_start,_end,_pay,_year_fraction);
}


void vanilla_float_coupon::init_impl()
{
	if (m_fix < m_start) throw fbox::error("fixing date set before simulation start date in float_coupon");
}


void vanilla_float_coupon::reset_impl()
{
	m_fixed = false;
	update_impl();
}


bool vanilla_float_coupon::update_impl()
{
	if (m_state.matured == true)
	{
		m_state.flow = m_state.value = 0.0;
		return false;
	}

	if (m_time + 0.1 >= m_pay)
	{
		m_state.flow = m_amount;
		m_state.value = 0.0;
		m_state.matured = true;
		return true;
	}

	if (!m_fixed)
	{
		m_amount = rate() * m_year_fraction;
		if (m_time + 0.1 >= m_fix) m_fixed = true;
	}

	m_state.value = m_amount * mp_yc->discount(m_pay);

	return true;
}


double vanilla_float_coupon::rate()
{
	return m_multiplier * (mp_yc->discount(m_accrual_start) / mp_yc->discount(m_accrual_end) - 1.0);
}


//////////////////////////////////////////////////////
// float_coupon
//////////////////////////////////////////////////////

void float_coupon::setup(
	yield_curve_ptr _yc,
	const time_type& _fix,
	const time_type& _start,
	const time_type& _end,
	const time_type& _pay,
	double_agent_ptr _index,
	double _multiplier,
	double _year_fraction)
{
	vanilla_float_coupon::setup(_yc,_fix,_start,_end,_pay,_multiplier,_year_fraction);
	connect(mp_index = _index);
}


bool float_coupon::update_impl()
{
	if (m_state.matured == true)
	{
		m_state.flow = m_state.value = 0.0;
		return false;
	}

	if (m_time + 0.1 >= m_pay)
	{
		m_state.flow = m_amount;
		m_state.value = 0.0;
		m_state.matured = true;
		return true;
	}

	if (!m_fixed)
	{
		m_amount = mp_index->state() * m_multiplier * m_year_fraction;
		if (m_time + 0.1 >= m_fix) m_fixed = true;
	}

	m_state.value = m_amount * mp_yc->discount(m_pay);

	return true;
}


} // namespace instruments
} // namespace simulate
} // namespace fbox

