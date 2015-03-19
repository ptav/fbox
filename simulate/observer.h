#ifndef __FBOX_SIMULATE_OBSERVER_H__
#define __FBOX_SIMULATE_OBSERVER_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Observers for simulation framework
*/


#include "agent_impl.h"
#include <fbox/main.h>
#include <fbox/error.h>
#include <boost/functional.hpp>


namespace fbox {
namespace simulate {


//! Observer interface
class observer
{
public:
	FBOX_LOCAL_ERROR(error,fbox::error,"observer error");

	observer() {}
	observer(double_agent_ptr _agent) : mp_agent(_agent) {}
	observer(const observer& _ob) : mp_agent(_ob.mp_agent) {}

	void set_agent(double_agent_ptr _agent) { mp_agent = _agent; }
	double_agent_ptr agent() const { return mp_agent; }

	virtual void init() {}
	virtual void observe(double _weight) = 0;
	virtual void end() {}

protected:
	double_agent_ptr mp_agent;
};



//! Accumulator for computing expectation
class expectation : public observer
{
public:
	expectation() {}
	expectation(double_agent_ptr _agent) : observer(_agent) {}
	expectation(const expectation& _e) : observer(_e),m_value(_e.m_value),m_weight(_e.m_weight) {}

	void init();
	void observe(double _weight);
	void end();
	
	double value() const { return m_value; }

private:
	double m_value;
	double m_weight;
};



//! Accumulator for computing bounds of a simulation
class bounds : public observer
{
public:
	bounds() {}
	bounds(double_agent_ptr _agent) : observer(_agent) {}
	bounds(const bounds& _e) : observer(_e),m_max(_e.m_max),m_min(_e.m_min) {}

	void init();
	void observe(double _weight);
	
	double max() const { return m_max; }
	double min() const { return m_min; }

private:
	double m_max,m_min;
};



//! Accumulator for computing expectation, variance and standard error
class statistics : public observer
{
public:
	statistics () {}
	statistics (double_agent_ptr _agent) : observer(_agent) {}
	statistics (const statistics & _e);

	void init();
	void observe(double _weight);
	void end();

	double expectation() const { return m_expectation; } //!< Expectation value
	double variance() const { return m_variance; } //!< Variance (unbiased)
	double standard_deviation() const; //!< Standard deviation (unbiased)
	double standard_error() const { return m_stderr; } //!< Standard MC error = sqrt(var / samples)
	double maximum() const { return m_max; } //! Maximum value observed
	double minimum() const { return m_min; } //! Maximum value observed

private:
	double m_expectation,m_variance,m_stderr;
	double m_max,m_min;
	double m_cum_weight;
};



//! Accumulator for computing multi agent moments: expectations and covariance
class cross_moments
{
public:
	cross_moments() {}
	cross_moments(const cross_moments& _o);

	void clear() { m_agents.clear(); }
	void add_agent(double_agent_ptr _agent) { m_agents.push_back(_agent); }
	double_agent_ptr agent(size_type _i) const { return m_agents[_i]; }

	void init();
	void observe(double _weight);
	void end();

	double expectation(size_type _i) const { return m_expectation[_i]; } //!< Expectation value
	double covariance(size_type _i,size_type _j) const { return m_covar[_i][_j]; } //!< Covariance (unbiased)
	double standard_error(size_type _i) const; //!< Standard MC error = sqrt(var / samples)

	void set_agent(double_agent_ptr _agent) {} // required by simulator implementation

private:
	std::vector<double_agent_ptr> m_agents;
	std::vector<double> m_expectation;
	std::vector<std::vector<double> > m_covar;
	size_type m_samples;
	double m_weight;
};



//! Accumulator for storing outcome at each time step
class scenarios : public observer
{
public:
	scenarios() {}
	scenarios(double_agent_ptr _agent) : observer(_agent) {}
	scenarios(const scenarios& _e);

	void init();
	void observe(double _weight);

	const std::vector<double>& outcomes() const { return m_outcomes; }
	const std::vector<double>& weights() const { return m_weights; }

private:
	std::vector<double> m_outcomes;
	std::vector<double> m_weights;
};


//! Accumulator for computing expectation
class histogram : public observer
{
public:
	histogram() {}

	histogram(
		double_agent_ptr _agent,
		double _min,
		double _max,
		double _step,
		size_type _order = 1,
		bool _pdf = false
		);
	
	histogram(const histogram& _e);

	void init();
	void observe(double _weight);
	void end();

	const std::vector<double>& bins() const { return m_bins; }
	const std::vector<double>& frequency() const { return m_probs; }

private:
	bool m_pdf;
	size_type m_order;
	double m_max,m_min,m_step,m_cum_weight;
	std::vector<double> m_bins,m_probs;

	void pos(double _value,size_type& _grid,double& _remainder);
};


template<typename _observer>
class observer_vector 
:	public std::vector<_observer>,
	public observer
{
public:
	typedef _observer observer_type;
	typedef std::vector<_observer> vector_type;

	void init() { std::for_each(vector_type::begin(),vector_type::end(),boost::mem_fun_ref(&observer_type::init)); }
	void observe(double _weight) { std::for_each(vector_type::begin(),vector_type::end(),boost::bind2nd(boost::mem_fun_ref(&observer_type::observe),_weight)); }
	void end() { std::for_each(vector_type::begin(),vector_type::end(),boost::mem_fun_ref(&observer_type::end)); }
};

} // namespace simulate
} // namespace fbox

#endif

