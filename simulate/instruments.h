#ifndef __FBOX_SIMULATE_INSTRUMENTS_H__
#define __FBOX_SIMULATE_INSTRUMENTS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Instrument agents
*/

#include <boost/math/distributions/normal.hpp>
#include <fbox/main.h>
#include "agent_impl.h"
#include "basic_pricing.h"
#include "yield_curve_models.h"


namespace fbox {
namespace simulate {
namespace instruments {


//! State type for instruments we can price
struct instrument_state
{
	double value; //!< Market value
	double flow; //!< Actual cashflows since the last time step
	bool matured; //!< Becomes true when the asset terminates

	instrument_state() : value(0.0),flow(0.0),matured(false) {}
	instrument_state(const instrument_state& _state) : value(_state.value),flow(_state.flow),matured(_state.matured) {}

	instrument_state& operator=(const instrument_state& _state);
	instrument_state& operator+=(const instrument_state& _state);
	instrument_state& operator*=(double _weight);
	instrument_state& operator/=(double _weight);

	void clear()
	{
		value = flow = 0.0;
		matured = false;
	}
};


instrument_state operator+(const instrument_state& _state0,const instrument_state& _state1);
instrument_state operator*(const instrument_state& _state,double _weight);
instrument_state operator/(const instrument_state& _state,double _weight);


typedef multi_agent_impl<instrument_state> instrument_agent;
typedef shared_ptr<instrument_agent> instrument_agent_ptr;


//! User defined instrument
class basic_instrument
:	public instrument_agent
{
public:
	void setup(
		double_agent_ptr _flows,
		double_agent_ptr _value,
		boolean_agent_ptr _matured = boolean_agent_ptr());

protected:
	double_agent_ptr mp_flows,mp_value;
	boolean_agent_ptr mp_matured;

	virtual void reset_impl();
	virtual bool update_impl();
};



//! Cash account instrument
/*!
	Initial amount set by <code>setup</code> can be later adjusted through <code>deposit</code> and
	<code>withdrawal</code>. The actual starting amount is only fixed when <code>init(...)</code> is
	called.
*/
class cash
:	public instrument_agent
{
public:
	void setup(
		double_agent_ptr _rate,
		double _initial_value = 0.0,
		double _loan_spread = 0.0,
		double _deposit_spread = 0.0);

	//! Deposit or withdrawal funds
	void transaction(double _value);

	//! Connect a cashflow source/drain agent
	void connect_flow(double_agent_ptr _agent);

	//! Clear all cashflow agents
	void clear_flows();

	//! Allow transaction to be fed into a cash account from other agents, without synchronising the said account
	//! (so that dependents can feed back to cash)
	class flow_connector
	:	public multi_agent_impl<
			double,
			time_type,
			shared_ptr<basic_valued_agent<double,time_type> >,
			duration_type
		>
	{
	public:
		virtual const double& state() const { return mp_account->state().value; }
		void setup(shared_ptr<cash> _account);

	protected:
		shared_ptr<cash> mp_account;

		virtual void reset_impl();
		virtual bool update_impl();
	};
	
protected:
	double_agent_ptr mp_rate;
	double m_initial,m_depo,m_loan,m_last_rate,m_local_flow;
	std::vector<double_agent_ptr> m_flows;

	virtual void init_impl();
	virtual void reset_impl();
	virtual bool update_impl();

	double get_rate() const;
};



//! Aggregate different instruments into a portfolio
/*
	If a cash account is defined, the portfolio becomes self-financing: no in or out leg of cash from
	the instruments are	seem on the state().flow parameter and are reflected in the portfolio value 
	instead. Instead any instrument leg are absorbed by the account. To iinspect net instrument leg 
	one can look at the flow paramter of the account itself that reflects  any flow in our out of the
	account.
*/
class portfolio
:	public instrument_agent
{
public:
	void clear();

	void set_cash_account(
		double_agent_ptr _rate,
		double _initial_value = 0.0,
		double _loan_spread = 0.0,
		double _deposit_spread = 0.0);
	
	void add_instrument(instrument_agent_ptr _instrument,double _weight = 1.0);
	void add_instrument(instrument_agent_ptr _instrument,double_agent_ptr _weight);

	shared_ptr<fbox::simulate::instruments::cash> cash_account() const { return mp_cash; }
	instrument_agent_ptr instrument(size_type _i) const { return m_names[_i].first; }
	double_agent_ptr weight(size_type _i) const { return m_names[_i].second; }

protected:
	shared_ptr<fbox::simulate::instruments::cash> mp_cash;

	typedef std::pair<instrument_agent_ptr,double_agent_ptr> item_type;
	typedef std::vector<item_type> vec_type;
	vec_type m_names;

	virtual void reset_impl();
	virtual bool update_impl();
};


//! Cashflow leg pricing agent
class fixed_leg
:	public instrument_agent
{
public:
	//! Initial setup, optionally with equally spaced schedule
	void setup(
		yield_curve_ptr _yc,
		const time_type& _start = 0.0,
		const time_type& _end = 0.0,
		const duration_type& _period = 0.0,
		double _amount = 0.0,
		double _payout = 0.0);
	
	//! Create equally spaced, constant value leg, plus an optional final payout (redemption)
	void set_schedule(
		const time_type& _start,
		const time_type& _end,
		const duration_type& _period,
		double _amount,
		double _payout = 0.0);

	//! Number of flows
	size_type size();

	//! Clear leg flows
	void clear();

	//! Add single flow
	void add(const time_type& _time,double _amount);

protected:
	struct flow_type
	{
		time_type time;
		double amount;
		flow_type(const time_type& _t,double _a) : time(_t),amount(_a) {}
	};
	typedef std::vector<flow_type> flows_vec_type;

	flows_vec_type m_flows;
	yield_curve_ptr mp_yc;
	flows_vec_type::const_iterator m_itr,m_itr0;

	double seek(const time_type& _time);

	virtual void init_impl();
	virtual void reset_impl();
	virtual bool update_impl();
};


/*
//! Floating cashflow leg pricing agent
class float_leg
:	public fixed_leg
{
public:
	//! Initial setup, optionally with equally spaced schedule
	void setup(
		yield_curve_ptr _yc,
		const time_type& _start = 0.0,
		const time_type& _end = 0.0,
		const duration_type& _period = 0.0,
		double _multiplier = 0.0,
		double _rate = 0.0,
		double _payout = 0.0,
		double _dcc = 365);
	
	//! Create equally spaced, constant value leg, plus an optional final payout (redemption)
	void set_schedule(
		const time_type& _start,
		const time_type& _end,
		const duration_type& _period,
		double _multiplier,
		double _rate,
		double _payout = 0.0,
		double _dcc = 365);

protected:
	double m_multiplier,m_rate,m_final;

	virtual bool update_impl();
};
*/


//! Risky cashflow leg pricing agent
class risky_leg : public fixed_leg
{
public:
	//! Create equally spaced, constant value leg
	void setup(
		yield_curve_ptr _yc,		//!< Discount curve
		yield_curve_ptr _sc,		//!< Survival curve and short term event intensity
		double _recovery = 0.0);	//!< Amount paid when termination is event driven

protected:
	double m_recovery;
	yield_curve_ptr mp_sc;

	virtual bool update_impl();
};



//! Risky portfolio leg. When events occur a fraction of the NPV is paid immediately.
class portfolio_fixed_leg : public fixed_leg
{
public:
	void setup(
		yield_curve_ptr _yc,					//!< Discount curve
		yield_curve_ptr _sc,					//!< Survival curve and short term event intensity
		portfolio_events::counter_ptr _counter,	//!< Size of outstanding portfolio
		double _recovery_rate);					//!< Fraction of outstanding value that is recovered when a termination event occurs

protected:
	yield_curve_ptr mp_sc;
	portfolio_events::counter_ptr mp_counter;
	double m_recovery_rate;

	virtual void init_impl();
	virtual void reset_impl();
	virtual bool update_impl();

private:
	double m_last_pv;
	size_type m_last_size;
};



//! Portfolio payments that occur with events
class portfolio_event_leg
:	public multi_agent_impl<instrument_state>
{
public:
	void setup(
		yield_curve_ptr _yc,					//!< Discount curve
		yield_curve_ptr _sc,					//!< Survival curve and short term event intensity
		shared_ptr<portfolio_events> _events,	//!< Portfolio events
		portfolio_events::counter_ptr _counter,	//!< Portfolio names counter
		const time_type& _maturity,				//!< Legal maturity date (after this date events no longer have an effect)
		double _payout);						//!< Payout amount

protected:
	yield_curve_ptr mp_yc,mp_sc;
	shared_ptr<portfolio_events> mp_events;
	portfolio_events::counter_ptr mp_counter;
	time_type m_maturity;
	double m_payout;

	virtual void init_impl();
	virtual bool update_impl();

private:
	time_type m_istep;
};



//! Cash account agent. Interest is reinvested.
class forward
:	public instrument_agent
{
public:
	void setup(
		yield_curve_ptr _yc,
		double_agent_ptr _security,
		double_agent_ptr _notional,
		double_agent_ptr _strike,
		const time_type& _start,
		const time_type& _expiry);

protected:
	yield_curve_ptr mp_yc;
	double_agent_ptr mp_sec;
	double_agent_ptr mp_notional;
	double_agent_ptr mp_strike;
	time_type m_start,m_expiry;
	
	bool m_has_started;
	double m_actual_strike,m_actual_notional;

	virtual void reset_impl();
	virtual bool update_impl();
};


//! Cash account agent. Interest is reinvested.
class option
:	public forward
{
public:
	enum model_type
	{
		BLACK_SCHOLES = 'b',
		NORMAL = 'n'
	};

	void setup(
		yield_curve_ptr _yc,
		double_agent_ptr _vol,
		double_agent_ptr _security,
		double_agent_ptr _notional,
		char _call_put,
		double_agent_ptr _strike,
		const time_type& _start,
		const time_type& _expiry,
		model_type _model = BLACK_SCHOLES);

protected:
	double_agent_ptr mp_vol;
	char m_cp;
	model_type m_model;

	virtual void reset_impl();
	virtual bool update_impl();
};


} // namespace instruments
} // namespace simulate
} // namespace fbox

#endif
