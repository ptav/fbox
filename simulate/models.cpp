/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Fundamental modelling processes
*/

#include "models.h"
#include "../core/xml_utils.h"
#include "../core/math.h"


namespace fbox {
namespace simulate {


//////////////////////////////////////////////////////
// year_fraction
//////////////////////////////////////////////////////

template<>
double year_fraction<double>::m_ratio = 365.0;



//////////////////////////////////////////////////////
// basic_diffusion
//////////////////////////////////////////////////////

void basic_diffusion::setup(
	double_agent_ptr _rnd,
	double _drift,
	double _volatility,
	double _initial)
{
	connect(mp_rnd = _rnd);
	m_drift = _drift;
	m_vol = _volatility;
	m_state = m_initial = _initial;
}


void basic_diffusion::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
		.attr("initial",m_initial)
		.attr("drift",m_drift)
		.attr("volatility",m_vol)
		.attr("random driver",mp_rnd);
}


void basic_diffusion::init_impl()
{
	if (!mp_rnd) throw error("basic_diffusion agent not setup correctly");
}
	

bool basic_diffusion::update_impl()
{
	double dt = year_fraction.yf(time_interval());
	m_state += m_drift * dt + m_vol * std::sqrt(dt) * mp_rnd->state();
	return true;
}



//////////////////////////////////////////////////////
// basic_lognormal
//////////////////////////////////////////////////////

void basic_lognormal::setup(
	double_agent_ptr _rnd,
	double _drift,
	double _volatility,
	double _initial)
{
	connect(mp_rnd = _rnd);
	m_drift = _drift;
	m_vol = _volatility;
	m_state = m_initial = _initial;
}


void basic_lognormal::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
		.attr("initial",m_initial)
		.attr("drift",m_drift)
		.attr("volatility",m_vol)
		.attr("random driver",mp_rnd);
}


void basic_lognormal::init_impl()
{
	if (!mp_rnd) throw error("basic_lognormal agent not setup correctly");
}
	

bool basic_lognormal::update_impl()
{
	double dt = year_fraction.yf(time_interval());
	m_state *= std::exp( (m_drift - m_vol*m_vol / 2.0) * dt + m_vol * std::sqrt(dt) * mp_rnd->state() );
	return true;
}



//////////////////////////////////////////////////////
// basic_ou
//////////////////////////////////////////////////////

void basic_ou::setup(
	double_agent_ptr _rnd,
	double_agent_ptr _reversion_level,
	double _reversion_speed,
	double _volatility,
	double _initial)
{
	connect(mp_rnd = _rnd);
	connect(mp_level = _reversion_level);
	m_speed = _reversion_speed;
	m_vol = _volatility;
	m_state = m_initial = _initial;
}


void basic_ou::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
		.attr("initial",m_initial)
		.attr("speed",m_speed)
		.attr("volatility",m_vol)
		.attr("level",mp_level)
		.attr("random driver",mp_rnd);
}


void basic_ou::init_impl()
{
	if (!mp_rnd) throw error("basic_ou agent not setup correctly");
}
	

bool basic_ou::update_impl()
{
	double mrl = mp_level->state();
	double dt = year_fraction.yf(time_interval());
	double emdt = std::exp(-m_speed * dt);
	m_state = m_state * emdt + mrl * (1.0 - emdt) + sqrt(m_vol * m_vol * (1.0 - emdt * emdt) / (2.0 * m_speed)) * mp_rnd->state();
	return true;
}



//////////////////////////////////////////////////////
// basic_event
//////////////////////////////////////////////////////

void basic_event::setup(
	double_agent_ptr _rnd,
	double_agent_ptr _intensity,
	double _initial)
{
	clear_connected();
	connect(mp_rnd = _rnd);
	connect(mp_intensity = _intensity);
	m_state = _initial;
}


void basic_event::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
		.attr("random driver",mp_rnd)
		.attr("intensity",mp_intensity);
}


void basic_event::init_impl()
{
	if (!mp_rnd) throw error("Missing random driver in basic_event agent");
	if (!mp_intensity) throw error("Missing intensity process in basic_event agent");
}
	

bool basic_event::update_impl()
{
	double q = mp_intensity->state() * year_fraction.yf(time_interval());
	double p = std::exp(-q);
	
	if (mp_rnd->state() > p) 
	{
		if (m_state > fbox::math::TINY)
			m_state = 0.0;
		else
			m_state = 1.0;

		return false;
	}

	return true;
}



//////////////////////////////////////////////////////
// basic_jump
//////////////////////////////////////////////////////

bool basic_jump::update_impl()
{
	double q = mp_intensity->state() * year_fraction.yf(time_interval());
	double pk = std::exp(-q);
	double p = pk;
	double k = 0.0;
	
	while (mp_rnd->state() > p) 
	{
		++k;
		pk *= q / k;
		p += pk;
	}

	m_state += k;

	return true;
}



//////////////////////////////////////////////////////
// lognormal_2_state
//////////////////////////////////////////////////////

void lognormal_2_state::setup(
	double_agent_ptr _diffusion,
	double_agent_ptr _switch,
	double _drift0,
	double _vol0,
	double _p0,
	double _drift1,
	double _vol1,
	double _p1,
	double _initial)
{
	clear_connected();
	connect(mp_diff = _diffusion);
	connect(mp_switch = _switch);

	m_d0 = _drift0;
	m_v0 = _vol0;
	m_h0 = -std::log(_p0);

	m_d1 = _drift1;
	m_v1 = _vol1;
	m_h1 = -std::log(_p1);

	m_state = m_initial = _initial;
}


void lognormal_2_state::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true))
		.attr("drift0",m_d0)
		.attr("vol0",m_v0)
		.attr("switch0",m_h0)
		.attr("drift1",m_d1)
		.attr("vol1",m_v1)
		.attr("switch1",m_h1)
		.attr("random driver",mp_rnd)
		.attr("switching driver",mp_switch);
}


void lognormal_2_state::init_impl()
{
	if (!mp_rnd) throw error("Missing Gaussian diffusion driver in lognormal_2_state");
	if (!mp_switch) throw error("Missing state switching driver in lognormal_2_state");
}
	

bool lognormal_2_state::update_impl()
{
	double dt = year_fraction.yf(time_interval());
	double rdt = std::sqrt(dt);

	double d,v;
	double p = mp_switch->state();
	if (m_s == 0)
	{
		if (p < std::exp(-m_h0*dt))
		{
			m_s = 1;
			d = m_d1;
			v = m_v1;
		}
		else
		{
			d = m_d0;
			v = m_v0;
		}
	}
	else
	{
		if (p < std::exp(-m_h1*dt))
		{
			m_s = 0;
			d = m_d0;
			v = m_v0;
		}
		else
		{
			d = m_d1;
			v = m_v1;
		}
	}

	m_state *= std::exp((d- v*v/2.0)*dt + v*rdt*mp_diff->state());

	return true;
}



} // namespace simulate
} // namespace fbox

