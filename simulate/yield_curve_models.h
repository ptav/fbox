#ifndef __FBOX_SIMULATE_YIELD_CURVE_MODELS_H__
#define __FBOX_SIMULATE_YIELD_CURVE_MODELS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Yield curve models
*/

#include <fbox/main.h>
#include <fbox/line.h>
#include "agent_impl.h"
#include "cached_agent.h"
#include "models.h"

namespace fbox {
namespace simulate {

//! Discount factor and interest rate interface for yield curve models
class basic_yield_curve
:	public multi_agent_impl<double>
{
public:
	//! Rolling discount bond (cash numeraire)
	virtual double discount() = 0;

	//! Discount bond maturing at _time
	virtual double discount(const time_type& _time) = 0;

public:
	//! Term/rolling bond interface and common interface
	class basic_bond
	:	public single_agent_impl<double>
	{
	public:
		void setup(shared_ptr<basic_yield_curve> _yc,const time_type& _end=time_type());
		void set_curve(shared_ptr<basic_yield_curve> _yc); //! Underlying yield curve
		void set_end(const time_type& _end); //! Maturity of bond

	protected:
		shared_ptr<basic_yield_curve> mp_yc;
		time_type m_mat;
	};

	//! Fixed maturity bond implementation
	class term_bond : public basic_bond
	{
	public:
		void set_redemption(bool _redemption); //! True (default) if state at maturity is unity (bond is redempted), zero otherwise. No effect for rolling bonds.

	protected:
		bool m_redemption;

		void reset_impl() { m_state = mp_yc->discount(m_mat); }
		
		bool update_impl() 
		{
			if (m_mat > m_time) 
			{
				m_state = mp_yc->discount(m_mat);
				return true;
			}
			else if (m_redemption && m_mat == m_time) 
			{
				m_state = 1.0;
				return true;
			}
			else
			{
				m_state = 0.0;
				return false;
			}
		}

		void dump(std::ostream& _strm) const;
	};

	//! rolling maturity bond implementation
	class rolling_bond : public basic_bond
	{
	protected:
		void reset_impl() { m_state = mp_yc->discount(m_time + m_mat); }
		bool update_impl() { m_state = mp_yc->discount(m_time + m_mat); return true; }
		void dump(std::ostream& _strm) const;
	};

	//! spot bond (cash account) implementation
	class spot_bond : public basic_bond
	{
	protected:
		void reset_impl() { m_state = mp_yc->discount(); }
		bool update_impl() { m_state = mp_yc->discount(); return true; }
		void dump(std::ostream& _strm) const;
	};

protected:
	fbox::simulate::year_fraction<double> year_fraction;
};


typedef shared_ptr<basic_yield_curve> yield_curve_ptr;

//! Constant rate yield curve
class constant_rate_yield_curve
:	public basic_yield_curve
{
public:
	void setup(double _rate);

	virtual double discount();
	virtual double discount(const time_type& _time);

protected:
	double m_rate;

	void reset_impl() {}
	bool update_impl() { return false; }
};


//! Static (non-stochastic) yield curve
class static_yield_curve
:	public basic_yield_curve
{
public:
	void setup(shared_ptr<math::line> _discount_factors);

	virtual double discount();
	virtual double discount(const time_type& _time);

protected:
	shared_ptr<math::line> mp_df;

	void reset_impl();
	bool update_impl();
};


//! Muti-factor LIBOR market yield curve model (N.B. this model does not obey arbitrage considerations by 
//! construction - the correct drift must be encoded in the underlying rate processes)
/*!
	Add each forward rate by providing a rate process and the tenor step. For example, the following code
	excerpt creates a LIBOR model curve with 4 underlying rate factors (r1, through r43) that respectively
	model the spot 3m, 3-6m and 6m-9m and 9m-inf rates.

	<code>
	libor_yield_curve yc;
	yc.set_tenor(91);
	yc.add_rate(r1);
	yc.add_rate(r2);
	yc.add_rate(r3);
	yc.add_rate(r4);
	</code>
*/	
class libor_yield_curve
:	public basic_yield_curve
{
public:
	void clear();

	void add_rate(double_agent_ptr _rate);
	void set_tenor(duration_type const& _tenor);

	virtual double discount();
	virtual double discount(const time_type& _time);
	virtual void dump(std::ostream& _strm) const;

protected:
	double m_spot_df;
	std::vector<double> m_df;
	std::vector<double_agent_ptr> m_rates;
	duration_type m_tenor;

	void reset_impl();
	bool update_impl();
};


//! Muti-factor swap rate market model (N.B. this model does not obey arbitrage considerations by 
//! construction - the correct drift must be encoded in the underlying rate processes)
/*!
	Add each swap rate rate in turn by providing a rate process and the maturity.

	<code>
	swap_yield_curve yc;
	yc.add_rate(r3m,91);
	yc.add_rate(r6m,182);
	yc.add_rate(r1y,365);
	yc.add_rate(r5y,1826);
	yc.add_rate(r10y,3652);
	</code>
*/	
class swap_yield_curve
:	public basic_yield_curve
{
public:
	void clear();

	void add_rate(double_agent_ptr _rate,duration_type const& _tenor);

	virtual double discount();
	virtual double discount(const time_type& _time);
	virtual void dump(std::ostream& _strm) const;

protected:
	struct rate_type
	{
		rate_type(double_agent_ptr _rate,duration_type const& _tenor) : rate(_rate),tenor(_tenor) {}

		double_agent_ptr rate;
		duration_type tenor;
	};

	typedef std::vector<rate_type> rate_vector_type;

	double m_spot_df;
	rate_vector_type m_rates;

	void reset_impl();
	bool update_impl();
};


//! Hull-White (aka extended Vasicek) short rate yield curve model
class hw_yield_curve
:	public basic_yield_curve
{
public:
	void setup(
		double_agent_ptr _rnd,			//!< Driving generator
		shared_ptr<math::line> _bonds,	//!< Discount factors
		double _reversion_speed,		//!< Mean reversion speed
		double _volatility);			//!< Volatility

	virtual double discount();
	virtual double discount(const time_type& _time);
	virtual void dump(std::ostream& _strm) const;

protected:
	struct cstate { double b,f,mu; };
	struct calibrator : public standalone_cached_agent_impl<cstate>
	{
		fbox::simulate::year_fraction<double> year_fraction;
		double mrs,vol,a,a0;
		shared_ptr<math::line> bonds;
		void setup(shared_ptr<math::line> _bonds,double _mrs,double _vol);
		virtual void reset_impl();
		virtual bool update_impl();
	};

	double_agent_ptr mp_rnd;
	shared_ptr<calibrator> mp_calibrator;
	double m_df;

	virtual void reset_impl();
	virtual bool update_impl();
};


//! Apply additive or multiplicative term structure shifts. State exposes current rolled-up discount factor.
class shifted_yield_curve
:	public basic_yield_curve
{
public:
	enum shift_type
	{
		additive = 'a',
		product = 'p'
	};

	void setup(
		shared_ptr<basic_yield_curve> _yc,	//!< Base yield curve
		shared_ptr<math::line> _shift,		//!< Shift function
		shift_type _type);					//!< Type of shift

	virtual double discount();
	virtual double discount(const time_type& _time);

protected:
	shared_ptr<basic_yield_curve> mp_yc;
	shared_ptr<math::line> mp_shift;
	shift_type m_type;

	void reset_impl();
	bool update_impl();
};


//! Apply additive or multiplicative term structure shifts
class combined_yield_curve
:	public basic_yield_curve
{
public:
	void setup(
		shared_ptr<basic_yield_curve> _yc1,		//!< First yield curve
		shared_ptr<basic_yield_curve> _yc2);	//!< Second yield curve

	virtual double discount();
	virtual double discount(const time_type& _time);

protected:
	shared_ptr<basic_yield_curve> mp_yc1,mp_yc2;

	void reset_impl();
	bool update_impl();
};



} // namespace simulate
} // namespace fbox

#endif
