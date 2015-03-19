/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Yield curve models
*/

#include "yield_curve_models.h"

namespace fbox {
namespace simulate {


//////////////////////////////////////////////////////
// basic_yield_curve
//////////////////////////////////////////////////////

void basic_yield_curve::basic_bond::setup(
	shared_ptr<basic_yield_curve> _yc,
	const time_type& _end)
{
	set_curve(_yc),
	set_end(_end);
}


void basic_yield_curve::basic_bond::set_curve(shared_ptr<basic_yield_curve> _yc)
{
	connect(mp_yc = _yc);
}


void basic_yield_curve::basic_bond::set_end(const time_type& _end)
{
	m_mat = _end;
}


void basic_yield_curve::term_bond::set_redemption(bool _redemption)
{
	m_redemption = _redemption;
}


void basic_yield_curve::term_bond::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true),true)
		.attr("ptr",this)
		.attr("maturity",m_mat)
		.attr("redemption",m_redemption)
		.attr("yield curve",mp_yc);
}


void basic_yield_curve::rolling_bond::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true),true)
		.attr("ptr",this)
		.attr("term",m_mat)
		.attr("yield curve",mp_yc);
}


void basic_yield_curve::spot_bond::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true),true)
		.attr("ptr",this)
		.attr("yield curve",mp_yc);
}



//////////////////////////////////////////////////////
// constant_rate_yield_curve
//////////////////////////////////////////////////////

void constant_rate_yield_curve::setup(double _rate)
{
	m_state = m_rate = _rate;
}


double constant_rate_yield_curve::discount()
{
	return std::exp(-m_rate * m_time/365.0);
}


double constant_rate_yield_curve::discount(const time_type& _time)
{
	return std::exp(-m_rate * (_time - m_time)/365.0);
}


//////////////////////////////////////////////////////
// static_yield_curve
//////////////////////////////////////////////////////

void static_yield_curve::setup(shared_ptr<math::line> _discount_factors)
{
	mp_df = _discount_factors;
}


double static_yield_curve::discount()
{
	return (*mp_df)(m_time);
}


double static_yield_curve::discount(const time_type& _time)
{
	return (*mp_df)(_time) / (*mp_df)(m_time);
}


void static_yield_curve::reset_impl()
{
	update_impl();
}


bool static_yield_curve::update_impl()
{
	m_state = log( (*mp_df)(m_time) / (*mp_df)(m_time+1) ) * 365.0;
	return true;
}


//////////////////////////////////////////////////////
// libor_yield_curve
//////////////////////////////////////////////////////

void libor_yield_curve::clear()
{
	clear_connected();
	m_rates.clear();
	m_df.clear();
}


void libor_yield_curve::set_tenor(duration_type const& _tenor)
{
	m_tenor = _tenor;
}


void libor_yield_curve::add_rate(double_agent_ptr _rate)
{
	connect(_rate);
	m_rates.push_back(_rate);
	m_df.push_back(1.0);
}


double libor_yield_curve::discount()
{
	return m_spot_df;
}


double libor_yield_curve::discount(const time_type& _time)
{
	double t = _time - m_time;
	double n = t / m_tenor;
	size_type i = static_cast<size_type>(n);
	size_type i1 = i >= m_rates.size()? m_rates.size()-1 : i;

	double acc = year_fraction.yf(t - i1 * m_tenor);
	double df = std::exp(-m_rates[i1]->state() * acc);

	if (i == 0)
		return df;
	else
		return m_df[i-1] * df;
}


void libor_yield_curve::reset_impl()
{
	m_spot_df = 1.0;
	update_impl();
}


bool libor_yield_curve::update_impl()
{
	// update spot discount factor
	double dt = year_fraction.yf(time_interval());
	m_spot_df *= std::exp(-(*m_rates.begin())->state() * dt);

	// update future discount factors
	double df = 1.0;
	double t = year_fraction.yf(m_tenor);
	for(size_type i = 0; i < m_rates.size(); ++i)
	{
		df = m_df[i] = df * std::exp(-m_rates[i]->state() * t);
	}

	m_state = m_rates[0]->state();

	return true;
}


void libor_yield_curve::dump(std::ostream& _strm) const
{
	basic_yield_curve::dump(_strm);
}



//////////////////////////////////////////////////////
// swap_yield_curve
//////////////////////////////////////////////////////

void swap_yield_curve::clear()
{
	clear_connected();
	m_rates.clear();
}


void swap_yield_curve::add_rate(double_agent_ptr _rate,duration_type const& _tenor)
{
	connect(_rate);
	m_rates.push_back( rate_type(_rate,_tenor) );
}


double swap_yield_curve::discount()
{
	return m_spot_df;
}


double swap_yield_curve::discount(const time_type& _time)
{
	rate_vector_type::iterator itr = m_rates.begin();
	while(itr != m_rates.end() && itr->tenor > _time) ++itr;

	if (itr == m_rates.begin()) return 1.0;

	--itr;

	double t = year_fraction.yf(_time);
	return std::exp(-itr->rate->state() * t);
}


void swap_yield_curve::reset_impl()
{
	m_spot_df = 1.0;
	update_impl();
}


bool swap_yield_curve::update_impl()
{
	// update spot discount factor
	double dt = year_fraction.yf(time_interval());
	m_spot_df *= std::exp(-m_rates.begin()->rate->state() * dt);

	m_state = m_rates.begin()->rate->state();

	return true;
}


void swap_yield_curve::dump(std::ostream& _strm) const
{
	basic_yield_curve::dump(_strm);
}



//////////////////////////////////////////////////////
// hw_yield_curve
//////////////////////////////////////////////////////

inline double emt(double m,double t)
{
	if (m * t > 2e-1) return (1.0 - std::exp(-m * t)) / m;
	else return (1.0 - m * t / 2.0) * t; // 2nd order expansion of the above
}


void hw_yield_curve::calibrator::setup(shared_ptr<math::line> _bonds,double _mrs,double _vol)
{
	bonds = _bonds;
	mrs = _mrs;
	vol = _vol;
}


void hw_yield_curve::calibrator::reset_impl()
{
	m_state.b = (*bonds)(m_start);
	m_state.f = -std::log((*bonds)(m_start+1) / m_state.b) * year_fraction.ratio();
	a = m_state.mu = m_state.f;
}


bool hw_yield_curve::calibrator::update_impl()
{
	double t = year_fraction.yf( time() - start() );
	double dt = year_fraction.yf( time_interval() );

	m_state.b = (*bonds)(m_time);
	m_state.f = -std::log((*bonds)(m_time+1) / m_state.b) * year_fraction.ratio();

	double g = (1.0 - std::exp(-mrs * t)) / mrs * vol;
	double a1 = m_state.f + g * g / 2.0;
	m_state.mu = a1 - a * std::exp(-mrs * dt);
	a = a1;

	return true;
}


void hw_yield_curve::setup(
	double_agent_ptr _rnd,
	shared_ptr<math::line> _bonds,
	double _reversion_speed,
	double _volatility)
{
	mp_calibrator.reset(new calibrator);
	mp_calibrator->setup(_bonds,_reversion_speed,_volatility);

	clear_connected();
	connect(mp_rnd = _rnd);
	connect(mp_calibrator);
}


void hw_yield_curve::reset_impl()
{
	m_state = mp_calibrator->state().f;
	m_df = 1.0;
}


bool hw_yield_curve::update_impl()
{
	double last = m_state;

	double mrs = mp_calibrator->mrs;
	double vol = mp_calibrator->vol;
	double mu = mp_calibrator->state().mu;

	double dt = year_fraction.yf(time_interval());

	double e = last * std::exp(-mrs * dt) + mu;
	double v = ( (1.0 - std::exp(-2.0 * mrs * dt)) / mrs ) * vol * vol / 2.0;

	m_state = e + sqrt(v) * mp_rnd->state();
	m_df *= std::exp(-(last+m_state)/2.0 * dt); // rough approximation

	return true;
}


double hw_yield_curve::discount()
{
	return m_df;
}


double hw_yield_curve::discount(const time_type& _time)
{
	double m = mp_calibrator->mrs;
	double v = mp_calibrator->vol;
	double b = mp_calibrator->state().b;
	double f = mp_calibrator->state().f;

	double fb = (*mp_calibrator->bonds)(_time) / b;

	double t = year_fraction.yf( time() - start() );
	double dt = year_fraction.yf(_time - m_time);
	
	double B = (1.0 - std::exp(-m * dt)) / m;
	double s = (1.0 - std::exp(-2.0 * m * t)) / m;
	double A = fb * std::exp( B * (f - B * v * v * s / 4.0) );
	
	return A * std::exp(-B * m_state);
}

 
void hw_yield_curve::dump(std::ostream& _strm) const
{
	fbox::xml::make_tag(_strm,fbox::type_id(*this,true),true)
		.attr("ptr",this)
		.attr("mrs",mp_calibrator->mrs)
		.attr("sigma",mp_calibrator->vol)
		.attr("rnd",mp_rnd);
}



//////////////////////////////////////////////////////
// shifted_yield_curve
//////////////////////////////////////////////////////

void shifted_yield_curve::setup(
	boost::shared_ptr<basic_yield_curve> _yc,
	boost::shared_ptr<math::line> _shift,
	shift_type _type)
{
	clear_connected();
	connect(mp_yc = _yc);
	mp_shift = _shift;
	m_type = _type;
}


double shifted_yield_curve::discount()
{
	if (m_type == additive)
		throw error("Additive rate shifts are not allowed yet. Use combined_yield_curve instead.");
	else
		return std::pow(mp_yc->discount(),(*mp_shift)(0.0));
}


double shifted_yield_curve::discount(const time_type& _time)
{
	double dt = _time - m_time;

	if (m_type == additive)
		throw error("Additive rate shifts are not allowed yet. Use combined_yield_curve instead.");
	else
		return std::pow(mp_yc->discount(_time),(*mp_shift)(dt));
}


void shifted_yield_curve::reset_impl()
{
	update_impl();
}


bool shifted_yield_curve::update_impl()
{
	if (m_type == additive)
		m_state = mp_yc->state() + (*mp_shift)(0);
	else
		m_state = mp_yc->state() * (*mp_shift)(0);

	return true;
}



//////////////////////////////////////////////////////
// combined_yield_curve
//////////////////////////////////////////////////////

void combined_yield_curve::setup(
	boost::shared_ptr<basic_yield_curve> _yc1,
	boost::shared_ptr<basic_yield_curve> _yc2)
{
	clear_connected();
	connect(mp_yc1 = _yc1);
	connect(mp_yc2 = _yc2);
}


double combined_yield_curve::discount()
{
	return mp_yc1->discount() * mp_yc2->discount();
}


double combined_yield_curve::discount(const time_type& _time)
{
	return mp_yc1->discount(_time) * mp_yc2->discount(_time);
}


void combined_yield_curve::reset_impl()
{
	update_impl();
}


bool combined_yield_curve::update_impl()
{
	m_state = mp_yc1->state() + mp_yc2->state();
	return true;
}



} // namespace simulate
} // namespace fbox

