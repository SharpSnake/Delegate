// ===========================================================================================================
// A C# like simple MulticastDelegate.
// 						              - WuYu, 2012.12.
// History:
//	WuYu, 2012.12    : Created.
//	WuYu, 2018.12.01 : Update to C++11, and support multiple parameter.
// 
// ===========================================================================================================
#ifndef METEORA_DELEGATE_HPP
#define METEORA_DELEGATE_HPP

#include <memory>		// for shared_ptr
#include <utility>		// for pair, forward, move
#include <list>
#include <algorithm>	// for find


namespace Meteora		{

// helper classes fwd declare
namespace details	{
	template< typename T >					class Delegate_base;
	template< typename Class, typename T >	class Delegate_impl_mem;
	template< typename T >					class Delegate_impl_global;
}

/*
*	Single delegate( binder ).
*/
template< typename T >
class Delegate	{};

template< typename R, typename... Args >
class Delegate< R( Args... ) >
{
	typedef Delegate											my_type;
	typedef details::Delegate_base< R( Args... ) >				binder_base;
	typedef std::shared_ptr< binder_base >						binder_ptr;

	template< typename Class >
	using binder_mem = details::Delegate_impl_mem< Class, R( Args... ) >;
	typedef details::Delegate_impl_global< R( Args... ) >		binder_global;

public:
	/*
	*	Initialize with empty function
	*/
	Delegate() = default;

	Delegate( nullptr_t )
		: Delegate()	{}

	/*
	*	bind to a member function
	*/
	template< typename Class >
	Delegate( R( Class::*method )( Args... ), Class *obj )
		: m_Binder( new binder_mem< Class >( method, obj ) )	{}

	/*
	*	bind to a global function or lambda expression
	*/
	Delegate( R( *method )( Args... ) )
		: m_Binder( new binder_global( method ) )	{}

	/*
	*	reset to a global function or lambda expression
	*/
	my_type& operator=( R( *method )( Args... ) )
	{
		m_Binder.reset( new binder_global( method ) );
		return *this;
	}

	/*
	*	Empty this delegate
	*/
	my_type& operator=( nullptr_t )	{
		m_Binder.reset();
		return *this;
	}

	// default copy and move
	Delegate( my_type&& ) = default;
	my_type& operator=( my_type&& ) = default;

public:
	/*
	*	Test if delegate is not empty
	*/
	explicit operator bool() const	{ return ( bool )m_Binder; }

	/*
	*	Invoke
	*/
	template< typename... Types >
	R operator()( Types&&... args ) const	{
		return m_Binder->operator()( std::forward< Types >( args )... );
	}

	friend bool operator==( const my_type &left, const my_type &right )	{
		return *left.m_Binder == *right.m_Binder;
	}
	friend bool operator!=( const my_type &left, const my_type &right )	{
		return !( left == right );
	}

protected:
	binder_ptr				m_Binder;
};



/*
*	This is not real Multicast Delegate like C# FOR NOW, more like "Delegate List" or "Delegates Manager".
*	I will make some improvement in the future.
*/
template< typename T >
class MulticastDelegate	{};

template< typename R, typename... Args >
class MulticastDelegate< R( Args... ) >
{
	typedef MulticastDelegate				my_type;
	typedef Delegate< R( Args... ) >		item_type;

public:
	/*
	*	Initialize with empty invocationList
	*/
	MulticastDelegate() = default;

	MulticastDelegate( nullptr_t )
		: MulticastDelegate()	{}

	/*
	*	Initialize with a single delegate, any args can construct a Delegate.
	*/
	template< typename... Types >
	explicit MulticastDelegate( Types&&... args )	{
		m_InvocationList.emplace_back( std::forward< Types >( args )... );
	}

	/*
	*	Clear invocationList
	*/
	my_type& operator=( nullptr_t )
	{
		m_InvocationList.clear();
		return *this;
	}

	// default copy and move
	MulticastDelegate( const my_type& ) = default;
	my_type& operator=( const my_type& ) = default;
	MulticastDelegate( my_type&& ) = default;
	my_type& operator=( my_type&& ) = default;

public:
	/*
	*	Test if invocationList is not empty
	*/
	explicit operator bool() const	{ return m_InvocationList.size(); }

	/*
	*	Fire( Invoke ) the invocationList, return the last item's return value.
	*/
	template< typename... Types >
	R operator()( Types&&... args ) const
	{
		auto lastiter = ( ++m_InvocationList.rbegin() ).base();
		for( auto i = m_InvocationList.begin(); i != lastiter; ++i )
			i->operator()( std::forward< Types >( args )... );
		return lastiter->operator()( std::forward< Types >( args )... );
	}

	/*
	*	Push a new delegate with any input arguments
	*/
	void operator+=( const item_type &del )	{
		m_InvocationList.push_back( del );
	}
	void operator+=( item_type &&del )	{
		m_InvocationList.emplace_back( std::move( del ) );
	}
	void operator+=( R( *method )( Args... ) )	{
		m_InvocationList.emplace_back( method );
	}

	/*
	*	Remove a delegate from invocationList
	*/
	void operator-=( const item_type &del )
	{
		auto iter = std::find( m_InvocationList.begin(), m_InvocationList.end(), del );
		if( iter != m_InvocationList.end() )
			m_InvocationList.erase( iter );
	}
	void operator-=( R( *method )( Args... ) )	{
		this->operator-=( item_type( method ) );
	}

	// Add or remove an empty delegate is not allowed
	void operator+=( nullptr_t ) = delete;
	void operator-=( nullptr_t ) = delete;

private:
	std::list< item_type >				m_InvocationList;
};



namespace details	{
/*
*	Abstract delegate, just like std::function, but only do compare and invoke.
*/
template< typename T >
class Delegate_base	{};

template< typename R, typename... Args >
class Delegate_base< R( Args... ) >
{
	typedef Delegate_base	my_type;

public:
	/*
	*	for member function
	*/
	Delegate_base( intptr_t funcaddr, intptr_t varaddr )
		: m_Address{ funcaddr, varaddr }	{}

	/*
	*	for global function
	*/
	Delegate_base( intptr_t funcaddr )
		: Delegate_base( funcaddr, 0 )	{}

	virtual ~Delegate_base() = default;

public:
	// invoke
	virtual R operator()( Args... args ) const = 0;

	friend bool operator ==( const my_type &left, const my_type &right )	{
		return left.m_Address == right.m_Address;
	}
	friend bool operator !=( const my_type &left, const my_type &right )	{
		return !( left == right );
	}

private:
	// Address of func, < func's addr, var's addr( if it's a member function ) >
	std::pair< intptr_t, intptr_t >		m_Address;
};



/*
*	Member function implementation
*/
template< typename Class, typename T >
class Delegate_impl_mem	{};

template< typename Class, typename R, typename... Args >
class Delegate_impl_mem< Class, R( Args... ) >
	: public Delegate_base< R( Args... ) >
{
	typedef Delegate_base< R( Args... ) >		base_type;
	typedef R( Class::*method_type )( Args... );

public:
	Delegate_impl_mem( method_type func, Class *obj )
		: base_type( reinterpret_cast< intptr_t& >( func ), ( intptr_t )obj ), 
		m_Method( func ), m_Obj( obj )	{}

	R operator()( Args... args )	const	override	{
		return ( m_Obj->*m_Method )( args... );
	}

private:
	method_type		m_Method;	// member function pointer match with delegate
	Class			*m_Obj;		// object ptr where the delegate came from
};



/*
*	Global function( or Lambda expression ) implementation
*/
template< typename T >
class Delegate_impl_global	{};

template< typename R, typename... Args >
class Delegate_impl_global< R( Args... ) >
	: public Delegate_base< R( Args... ) >
{
	typedef Delegate_base< R( Args... ) >		base_type;
	typedef R( *method_type )( Args... );

public:
	Delegate_impl_global( method_type func )
		: base_type( reinterpret_cast< intptr_t& >( func ) ), 
		m_Method( func )	{}

	R operator()( Args... args )	const	override	{
		return m_Method( args... );
	}

private:
	method_type		m_Method;	// global function pointer
};

}	// !details
}	// !Meteora
#endif	// !METEORA_DELEGATE_HPP