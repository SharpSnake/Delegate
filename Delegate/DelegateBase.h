// ===========================================================================================================
// 						              - WuYu, 2012.12.
// History:
// 
// ===========================================================================================================

#include <memory>


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// 一个委托元素的抽象基类，记录关于委托的最基础的信息
// 一个基础的委托是与类无关的，只要函数返回类型和输入类型与模版一致都可以接受
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
template< typename R, typename T1, typename T2 >   // R means 'return value type', the following is the same.
class Delegate_base
{
protected:
	// referenc time
	int m_RefNum;

	// we provided: if two delegates have the same object address and member function address, 
	// we consider one delegate is equal to another.
	int m_ObjectAddress;
	int m_MethodAddress;

public:
	Delegate_base(void)
	{
		m_RefNum = 0;
	}

	inline void AddRef(void)
	{
		++m_RefNum;
	}

	inline void Release(void)
	{
		if ( --m_RefNum <= 0 )
			delete this;
	}

	// 如果来自相同的对象的同一方法，就是相同的委托
	bool operator == ( const Delegate_base< R, T1, T2 > &other ) const
	{
		return m_ObjectAddress == other.m_ObjectAddress && m_MethodAddress == other.m_MethodAddress;
	}

	bool operator != ( const Delegate_base< R, T1, T2 > &other ) const
	{
		return m_ObjectAddress != other.m_ObjectAddress || m_MethodAddress != other.m_MethodAddress;
	}

	// call delegate
	virtual R operator()( T1 v1, T2 v2 ) = 0;
};


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// 一个委托元素的实例，记录具体的对象及其成员函数指针，在委托中此对象被装箱隐藏
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
template< typename Class, typename R, typename T1, typename T2 >
class Delegate_impl : public Delegate_base< R, T1, T2 >
{
	// define member function pointer
	typedef R ( Class::* MethodName )( T1, T2 );

private:
	// the object where the delegate come from
	Class* m_Obj;

	// the member function pointer match with delegate
	MethodName m_Method;

public:
	// 实例化委托
	// e.g.: 
	//     Class1 a;
	//     new Delegate_impl< Class1, int, int, int >( &a, &Class1::Function1 );
	Delegate_impl( Class *_Obj, MethodName _Method )
	{
		m_Obj = _Obj;
		m_Method = _Method;

		m_ObjectAddress = ( int )m_Obj;
		memcpy( &m_MethodAddress, &m_Method, 4 );   // i think it's the only way to get member function's address, what do you say?
	}

	// implement the base class's virtual operator()
	R operator() ( T1 v1, T2 v2 )
	{
		return ( m_Obj->*m_Method )( v1, v2 );
	}
};



// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// Delegate item
// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
template< typename R, typename T1, typename T2 >
class delegate
{
private:
	// 委托的基础信息
	Delegate_base< R, T1, T2 >* m_Delegate;

public:
	// 常规构造
	template< typename Class, typename R, typename T1, typename T2 >
	delegate( Class *_Obj, R( Class::* _Method )( T1, T2 ) )
	{
		m_Delegate = new Delegate_impl< Class, R, T1, T2 >( _Obj, _Method );
		m_Delegate->AddRef();
	}

	// 拷贝构造
	delegate( const delegate< R, T1, T2 > &other )
	{
		m_Delegate = other.m_Delegate;
		m_Delegate->AddRef();
	}

	// 直接赋值
	delegate< R, T1, T2 >& operator = ( const delegate< R, T1, T2 > &other )
	{
		m_Delegate->Release();
		m_Delegate = other.m_Delegate;
		m_Delegate->AddRef();
		return *this;
	}

	~delegate(void)
	{
		m_Delegate->Release();
	}

	// 增加一次引用
	inline void AddRef(void)
	{
		m_Delegate->AddRef();
	}

	// 尝试销毁
	inline void Release(void)
	{
		m_Delegate->Release();
	}

	// 执行此委托
	R operator () ( T1 v1, T2 v2 )
	{
		return ( *m_Delegate )( v1, v2 );
	}

	// 两个委托做比较
	bool operator == ( const delegate< R, T1, T2 > &other ) const
	{
		return *m_Delegate == *other.m_Delegate;
	}

	bool operator != ( const delegate< R, T1, T2 > &other ) const
	{
		return *m_Delegate != *other.m_Delegate;
	}
};