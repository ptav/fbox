#ifndef __FBOX_SIMULATE_BASIC_PRICING_H__
#define __FBOX_SIMULATE_BASIC_PRICING_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Some basic pricing agents
*/

#include <boost/math/distributions/normal.hpp>
#include <fbox/main.h>
#include "agent_impl.h"
#include "basic_agents.h"
#include "models.h"


namespace fbox {
namespace simulate {


//! Cashflow leg pricing agent
class flows
:	public multi_agent_impl<double>
{
public:
	//! Initial setup, optionally with equally spaced schedule
	void setup(
		double_agent_ptr _index = double_agent_ptr(),
		const time_type& _start = 0.0,
		const time_type& _end = 0.0,
		const duration_type& _period = 0.0,
		double _multiplier = 0.0,
		double _fixed_amount = 0.0,
		double _payout = 0.0,
		double _dcc_ratio = 365.0);
	
	//! Create equally spaced, constant value flows, plus an optional final payout (redemption)
	void set_schedule(
		const time_type& _start,
		const time_type& _end,
		const duration_type& _period,
		double _multiplier = 0.0,
		double _fixed_amount = 0.0,
		double _payout = 0.0,
		double _dcc_ratio = 365.0);

	//! Number of flows
	size_type size();

	//! Clear all flows
	void clear();

	//! Add single flow
	void add(
		const time_type& _fix,
		const time_type& _start,
		const time_type& _end,
		const time_type& _pay,
		double _multiplier,
		double _margin=0.0,
		double _principal=0.0,
		double _yf=-1.0);

	//! Add single flow (simpler version)
	void add(
		const time_type& _start,
		const time_type& _end,
		double _multiplier,
		double _margin=0.0,
		double _principal=0.0,
		double _yf=-1.0);

	//! Add single principal flow
	void add(
		const time_type& _pay,
		double _principal);

	virtual void dump(std::ostream& _strm) const;

protected:
	struct flow_type
	{
		time_type fix,start,end,pay;
		double margin;
		double multiplier;
		double principal;
		double yf;

		double fixing;

		flow_type(const time_type& _fix,const time_type& _t0,const time_type& _t1,const time_type& _pay,double _m,double _a,double _p,double _yf) 
		:	fix(_fix),start(_t0),end(_t1),pay(_pay),multiplier(_m),margin(_a),principal(_p),yf(_yf),fixing(0.0) {}
	};
	typedef std::vector<flow_type> flows_vec_type;

	flows_vec_type m_leg;
	double_agent_ptr mp_index;
	flows_vec_type::iterator m_pitr,m_fitr,m_pitr0,m_fitr0;
	bool m_matured;

	double seek(const time_type& _time);

	virtual void init_impl();
	virtual void reset_impl();
	virtual bool update_impl();
};



//! Size of a portfolio of names subject to termination events (default, death, etc.) 
/*!
	Events are determined using a recursion that tests for the probability of 1 or more 
	events, then if that suceeds, for the probability of 2 or more and so on up until such 
	a test fails. It is best suited to small portfolios or very unlikely events and not 
	very efficient for large N.p1 values, where N is the size of the portfolio and p1 the 
	individual event probability.

	The agent's state carries information on how many events have accured in the last time
	step. The counting process retains the information on how many names survive at each
	point in time.
*/
class portfolio_events
:	public multi_agent_impl<size_type>
{
public:
	typedef constant<size_type> counter;
	typedef shared_ptr<counter> counter_ptr;

	void setup(
		counter_ptr _counter,			//!< Name counter
		double_agent_ptr _intensity,	//!< Event intensity
		int _impact);					//!< Impact of each event on the counter agent

	virtual void dump(std::ostream& _strm) const;

protected:
	int m_impact;
	counter_ptr mp_counter;
	double_agent_ptr mp_h;

	virtual void init_impl();
	virtual bool update_impl();
};


//! black Scholes price process
class black_scholes
:	public multi_agent_impl<double>
{
public:
	void setup(
		double_agent_ptr _forward,		//!< Forward process
		double_agent_ptr _volatility,	//!< Volatility process
		double _notional,
		char _call_put,
		double _strike,
		const time_type& _expiry);

protected:
	double_agent_ptr mp_fwd,mp_vol;
	double m_notional,m_strike;
	char m_call_put;
	time_type m_expiry;

	virtual void reset_impl();
	virtual bool update_impl();
};


} // namespace simulate
} // namespace fbox

#endif
