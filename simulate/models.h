#ifndef __FBOX_SIMULATE_MODELS_H__
#define __FBOX_SIMULATE_MODELS_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Fundamental modelling processes
*/

#include <boost/math/distributions/normal.hpp>
#include <fbox/main.h>
#include "agent_impl.h"

namespace fbox {
namespace simulate {

//! Year fraction instrumentation class. 
/*!
	The parameters are static members and so all instances of the class shared the same day count
	convention.
*/
template<typename _duration_type>
class year_fraction
{
public:
	static void set_ratio(_duration_type const& _ratio) { m_ratio = _ratio; }

	static double ratio() { return m_ratio; }
	static double yf(_duration_type const& _dt) { return _dt / m_ratio; }

protected:
	static _duration_type m_ratio;	
};


//! Diffusion process with constant parameters
class basic_diffusion
:	public single_agent_impl<double>
{
public:
	void setup(
		double_agent_ptr _rnd,
		double _drift,
		double _volatility,
		double _initial=0.0);

	virtual void dump(std::ostream& _strm) const;

protected:
	double_agent_ptr mp_rnd;
	double m_initial,m_drift,m_vol;
	fbox::simulate::year_fraction<double> year_fraction;

	virtual void init_impl();
	virtual bool update_impl();
};


//! Lognormal diffusion process with constant parameters
class basic_lognormal
:	public single_agent_impl<double>
{
public:
	void setup(
		double_agent_ptr _rnd,
		double _drift,
		double _volatility,
		double _initial=0.0);

	virtual void dump(std::ostream& _strm) const;

protected:
	double_agent_ptr mp_rnd;
	double m_initial,m_drift,m_vol;
	fbox::simulate::year_fraction<double> year_fraction;

	virtual void init_impl();
	virtual bool update_impl();
};


//! Orstein-Uhlenbeck process with constant vol and reversion speed parameters
class basic_ou
:	public multi_agent_impl<double>
{
public:
	void setup(
		double_agent_ptr _rnd,
		double_agent_ptr _reversion_level,
		double _reversion_speed,
		double _volatility,
		double _initial=0.0);

	virtual void dump(std::ostream& _strm) const;

protected:
	double_agent_ptr mp_rnd,mp_level;
	double m_initial,m_speed,m_vol;
	fbox::simulate::year_fraction<double> year_fraction;

	virtual void init_impl();
	virtual bool update_impl();
};


//! Simple single jump ("event") counting process
class basic_event
:	public multi_agent_impl<double>
{
public:
	void setup(
		double_agent_ptr _rnd,			//!< Random noise source
		double_agent_ptr _intensity,	//!< Jump intensity
		double _initial=0.0);			//!< Initial state

	virtual void dump(std::ostream& _strm) const;

protected:
	double_agent_ptr mp_rnd,mp_intensity;
	fbox::simulate::year_fraction<double> year_fraction;

	virtual void init_impl();
	virtual bool update_impl();
};


//! Simple jump counting process
class basic_jump
:	public basic_event
{
protected:
	virtual bool update_impl();
};


//! 2-state lognormal diffusion
class lognormal_2_state
:	public multi_agent_impl<double>
{
public:
	void setup(
		double_agent_ptr _diffusion,
		double_agent_ptr _switch,
		double _drift0,
		double _vol0,
		double _p0,
		double _drift1,
		double _vol1,
		double _p1,
		double _initial=0.0);

	virtual void dump(std::ostream& _strm) const;

protected:
	double_agent_ptr mp_diff,mp_switch;
	double m_initial,m_d0,m_v0,m_h0,m_d1,m_v1,m_h1;
	fbox::simulate::year_fraction<double> year_fraction;

	virtual void init_impl();
	virtual bool update_impl();

private:
	size_type m_s;
};


} // namespace simulate
} // namespace fbox

#endif
