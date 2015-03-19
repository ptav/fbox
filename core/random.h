#ifndef __FBOX_CORE_RANDOM_H__
#define __FBOX_CORE_RANDOM_H__
/*!
	\file
	\copyright Copyright (c) 2015 Pedro Tavares
	\warning Please refer to the copyright notices and important disclaimers in file 'LICENSE'

	Random number generators
*/

#include "main.h"
#include "MersenneTwister.h"

namespace fbox {
namespace math {

//! Random number interface
class FBOX_DLL random
{
public:
	//! Retrieve next number in sequence
	virtual double operator() () = 0;

	//! Set long integer seed
	virtual void seed(unsigned long seed) = 0;

	//! Save state of generator
	virtual void save() = 0;

	//! Restore saved state
	virtual void restore() = 0;
};


//! Mersenne twister implementation of the interface
class FBOX_DLL mersenne_twister 
:	public random,
	private MTRand
{
public:
	virtual double operator() () { return MTRand::rand(); }

	virtual void seed(unsigned long seed) { MTRand::seed(seed); }

	virtual void save() { MTRand::save(m_save_array); }
	virtual void restore() { MTRand::load(m_save_array); }

private:
	MTRand::uint32 m_save_array[SAVE];
};



} // namespace math
} // namespace fbox

#endif
