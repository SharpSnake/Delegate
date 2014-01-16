// ===========================================================================================================
// 						              - WuYu, 2012.12.
// History:
// 
// ===========================================================================================================

#include <memory>


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
// һ��ί��Ԫ�صĳ�����࣬��¼����ί�е����������Ϣ
// һ��������ί���������޹صģ�ֻҪ�����������ͺ�����������ģ��һ�¶����Խ���
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

	// ���������ͬ�Ķ����ͬһ������������ͬ��ί��
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
// һ��ί��Ԫ�ص�ʵ������¼����Ķ������Ա����ָ�룬��ί���д˶���װ������
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
	// ʵ����ί��
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
	// ί�еĻ�����Ϣ
	Delegate_base< R, T1, T2 >* m_Delegate;

public:
	// ���湹��
	template< typename Class, typename R, typename T1, typename T2 >
	delegate( Class *_Obj, R( Class::* _Method )( T1, T2 ) )
	{
		m_Delegate = new Delegate_impl< Class, R, T1, T2 >( _Obj, _Method );
		m_Delegate->AddRef();
	}

	// ��������
	delegate( const delegate< R, T1, T2 > &other )
	{
		m_Delegate = other.m_Delegate;
		m_Delegate->AddRef();
	}

	// ֱ�Ӹ�ֵ
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

	// ����һ������
	inline void AddRef(void)
	{
		m_Delegate->AddRef();
	}

	// ��������
	inline void Release(void)
	{
		m_Delegate->Release();
	}

	// ִ�д�ί��
	R operator () ( T1 v1, T2 v2 )
	{
		return ( *m_Delegate )( v1, v2 );
	}

	// ����ί�����Ƚ�
	bool operator == ( const delegate< R, T1, T2 > &other ) const
	{
		return *m_Delegate == *other.m_Delegate;
	}

	bool operator != ( const delegate< R, T1, T2 > &other ) const
	{
		return *m_Delegate != *other.m_Delegate;
	}
};