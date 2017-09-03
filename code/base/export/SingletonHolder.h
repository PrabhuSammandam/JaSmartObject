/*
 * SingletonHolder.h
 *
 *  Created on: Jun 29, 2017
 *      Author: psammand
 */

#ifndef NETWORK_EXPORT_SINGLETONHOLDER_H_
#define NETWORK_EXPORT_SINGLETONHOLDER_H_

namespace ja_iot
{
namespace base
{
typedef void ( *atexit_pfn_t ) ();
////////////////////////////////////////////////////////////////////////////////
// class template NoDestroy
// Implementation of the LifetimePolicy used by SingletonHolder
// Never destroys the object
////////////////////////////////////////////////////////////////////////////////

template<class T>
struct NoDestroy
{
    static void ScheduleDestruction( T *, atexit_pfn_t pFun ) {}

    static void OnDeadReference() {}
};


////////////////////////////////////////////////////////////////////////////////
// class template CreateUsingNew
// Implementation of the CreationPolicy used by SingletonHolder
// Creates objects using a straight call to the new operator
////////////////////////////////////////////////////////////////////////////////

template<class T> struct CreateUsingNew
{
    static T* Create() { return ( new T{} ); }

    static void Destroy( T *p ) { delete p; }
};


template<class T> struct CreateStatic
{
    union MaxAlign
    {
        char        t_[sizeof( T )];
        short int   shortInt_;
        int         int_;
        long int    longInt_;
        float       float_;
        double      double_;
        long double longDouble_;
        struct Test;

        int Test::  *pMember_;
        int (Test::*pMemberFn_) ( int );
    };

    static T* Create()
    {
        static MaxAlign staticMemory_{};

        return ( new(&staticMemory_) T{} );
    }

    static void Destroy( T *p )
    {
        p->~T ();
    }
};

template
<
    typename T,
    template<class> class CreationPolicy = CreateStatic,
    template<class> class LifetimePolicy = NoDestroy
>
class SingletonHolder
{
public:
    static T& Inst();

private:
    static void MakeInstance();
    static void DestroySingleton();
    SingletonHolder ();

    static T       *_pInstance;
    static bool    _destroyed;
};

template
<
    class T,
    template<class> class C,
    template<class> class L
>
T * SingletonHolder<T, C, L>::_pInstance{ nullptr };

template
<
    class T,
    template<class> class C,
    template<class> class L
>
bool SingletonHolder<T, C, L>::_destroyed{ false };

template
<
    class T,
    template<class> class CreationPolicy,
    template<class> class LifetimePolicy
>
inline T &SingletonHolder<T, CreationPolicy, LifetimePolicy>::Inst()
{
    if( !_pInstance )
    {
        MakeInstance();
    }

    return ( *_pInstance );
}

template
<
    class T,
    template<class> class CreationPolicy,
    template<class> class LifetimePolicy
>
void SingletonHolder<T, CreationPolicy, LifetimePolicy>::MakeInstance()
{
    if( !_pInstance )
    {
        if( _destroyed )
        {
            LifetimePolicy<T>::OnDeadReference();
            _destroyed = false;
        }

        _pInstance = CreationPolicy<T>::Create();
        LifetimePolicy<T>::ScheduleDestruction( _pInstance, &DestroySingleton );
    }
}

template
<
    class T,
    template<class> class CreationPolicy,
    template<class> class LifetimePolicy
>
void SingletonHolder<T, CreationPolicy, LifetimePolicy>::DestroySingleton()
{
    CreationPolicy<T>::Destroy( _pInstance );
    _pInstance = 0;
    _destroyed = true;
}
}
}
#endif /* NETWORK_EXPORT_SINGLETONHOLDER_H_ */
