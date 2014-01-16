// ===========================================================================================================
// A C# like delegate(list), only support two parameters so far.
// 						              - WuYu, 2012.12.
//
// I was still confused how to make Delegate support any number parameters?
// 
// ===========================================================================================================

#include <vector>

#include "DelegateBase.h"

using namespace std;


#define INVOKER2( InvokerName )\
template< typename T1, typename T2 >\
R InvokerName( T1 v1, T2 v2 )\
{\
	for ( int i = 0; i < m_DelegateQueue.size() - 1; ++i )\
		m_DelegateQueue[ i ]( v1, v2 );\
	return m_DelegateQueue[ m_DelegateQueue.size() - 1 ]( v1, v2 );\
}


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// 多路广播（ MulticastDelegate ）委托（列表）
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
template< typename R = void, typename DelType = delegate< void, int, int > >
class Delegate
{
private:
	vector< DelType > m_DelegateQueue;   // maybe use a DelType array is better

public:
	// 空构造
	Delegate(void)
	{}

	// 拷贝构造函数
	Delegate( const Delegate< R, DelType > &other )
	{
		m_DelegateQueue = other.m_DelegateQueue;
		for ( int i = 0, n = m_DelegateQueue.size(); i < n; ++i )
		{
			m_DelegateQueue[ i ].AddRef();
		}
	}

	// 直接赋值
	Delegate< R, DelType >& operator = ( const Delegate< R, DelType > &other )
	{
		Release();

		m_DelegateQueue = other.m_DelegateQueue;
		for ( int i = 0, n = m_DelegateQueue.size(); i < n; ++i )
		{
			m_DelegateQueue[ i ].AddRef();
		}

		return *this;
	}

	~Delegate(void)
	{
		Release();
	}

	inline void Release(void)
	{
		for ( int i = 0, n = m_DelegateQueue.size(); i < n; ++i )
		{
			m_DelegateQueue[ i ].Release();
		}
		m_DelegateQueue.clear();
	}

public:
	// 添加一个方法
	void operator += ( DelType newDel )
	{
		newDel.AddRef();
		m_DelegateQueue.push_back( newDel );
	}

	// 从末尾删掉一个方法
	void operator -= ( DelType item )
	{
		for ( int i = m_DelegateQueue.size() - 1; i >= 0; --i )
		{
			if ( m_DelegateQueue[ i ] == item )
			{
				m_DelegateQueue[ i ].Release();
				m_DelegateQueue.erase( m_DelegateQueue.begin() + i );
				return;
			}
		}
	}

	// fire the delegate list, like light firecrackers.
	INVOKER2( Invoke );
	INVOKER2( operator () );

	// 必须委托列表长度相同，且从头到尾对应的每一项都相同，两个MulticastDelegate才相同
	bool operator == ( const Delegate< R, DelType > &other ) const
	{
		int n1 = m_DelegateQueue.size(), n2 = other.m_DelegateQueue.size();

		if( n1 != n2 )
			return false;

		for ( int i = 0; i < n1; ++i )
		{
			if( m_DelegateQueue[ i ] != other.m_DelegateQueue[ i ] )
				return false;
		}

		return true;
	}

	bool operator != ( const Delegate< R, DelType > &other ) const
	{
		int n1 = m_DelegateQueue.size(), n2 = other.m_DelegateQueue.size();

		if( n1 != n2 )
			return true;

		for ( int i = 0; i < n1; ++i )
		{
			if( m_DelegateQueue[ i ] != other.m_DelegateQueue[ i ] )
				return true;
		}

		return false;
	}

	// 委托列表是否不空
	inline bool NotEmpty(void) const
	{
		return m_DelegateQueue.size() > 0;
	}
};