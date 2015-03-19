#ifndef __FBOX_SIMULATE_COUPON_INSTRUMENTS_H__
#define __FBOX_SIMULATE_COUPON_INSTRUMENTS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Coupon instrument agents
*/

#include <fbox/main.h>
#include "instruments.h"


namespace fbox {
namespace simulate {
namespace instruments {


//! Single coupon instrument. Base for exotic coupon legs.
class coupon_impl
:	public instrument_agent
{
public:
	using instrument_agent::time_type;

	void setup(
		const time_type& _fix,
		const time_type& _start,
		const time_type& _end,
		const time_type& _pay,
		double _year_fraction);

protected:
	time_type m_fix,m_accrual_start,m_accrual_end,m_pay;
	double m_year_fraction;
};



//! Fixed coupon instrument
class fixed_payment
:	public coupon_impl
{
public:
	void setup(
		yield_curve_ptr _yc,
		const time_type& _pay,
		double _amount = 0.0);

protected:
	yield_curve_ptr mp_yc;
	double m_amount;

	virtual void reset_impl();
	virtual bool update_impl();
};



//! Fixed coupon instrument
class fixed_coupon
:	public fixed_payment
{
public:
	void setup(
		yield_curve_ptr _yc,
		const time_type& _start,
		const time_type& _end,
		const time_type& _pay,
		double _rate,
		double _year_fraction);
};


//! Vanilla floating coupon instrument
class vanilla_float_coupon
:	public coupon_impl
{
public:
	void setup(
		yield_curve_ptr _yc,
		const time_type& _fix,
		const time_type& _start,
		const time_type& _end,
		const time_type& _pay,
		double _multiplier,
		double _year_fraction);

protected:
	yield_curve_ptr mp_yc;
	double m_multiplier,m_amount;
	bool m_fixed;

	virtual void init_impl();
	virtual void reset_impl();
	virtual bool update_impl();

	double rate(); //!< Actual coupon rate (not annualised)
};


//! Floating coupon instrument that refers a generic index
//! The underling index must report the forward value of the underlying after any corrections
class float_coupon
:	public vanilla_float_coupon
{
public:
	void setup(
		yield_curve_ptr _yc,
		const time_type& _fix,
		const time_type& _start,
		const time_type& _end,
		const time_type& _pay,
		double_agent_ptr _index,
		double _multiplier,
		double _year_fraction);

protected:
	double_agent_ptr mp_index;

	virtual bool update_impl();
};



} // namespace instruments
} // namespace simulate
} // namespace fbox

#endif
