/*
	Copyright (C) 2019 by Tigran Khachakranc 
	MIT license:
	http://en.wikipedia.org/wiki/MIT_License

	This is a derived work based on the ideas published by Sergey Alexandrovich Kryukov
	in his article “The Impossibly Fast C++ Delegates, Fixed”, 12 Mar 2017.
	Who in his turn derived his work from the original work of Sergey Ryazanov
	published in his article “The Impossibly Fast C++ Delegates”, 18 Jul 2005.

	Inspired by the original idea of fast delegates developed it further and turned it into [Notification - Connection - Delegate] system

	Original Article:
	"The Impossibly Fast C++ Delegates, Fixed", 12 Mar 2017
	https://www.codeproject.com/Articles/1170503/The-Impossibly-Fast-Cplusplus-Delegates-Fixed

	Most original article:
	"The Impossibly Fast C++ Delegates", 18 Jul 2005
	https://www.codeproject.com/articles/11015/the-impossibly-fast-c-delegates
*/



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Definition/Implememntation of the "Notification - Connection - Delegate infrastructure"
//
//	Objects could define Notifactions and fire them when there is necessity to notify outer word about something
//	Others can listen certain Notifactions and react accordingly
//	for this purpose they should dedicate Listener method and corresponding connection object
//	Ehenever Notification or Connecton destroys the link between them safely destroyed
//	Connection object tightly bound to its target object, ususally connection is a member of the receiver object
//	But when connection object kept seperately from its target then it is responsibility of the Programmer
//	to clear or destroyed it when target object destroyed, otherwise undefined behavoir (crash) could occour
//
//
//	Usage example
//
/*
class CSender
{
public:

	Notification<CSender, ...>		ntfSomethingHappened1(...)
	NotificationEx<CSender, ...>	ntfSomethingHappened2(...)

	void DoSomething()
	{
		ntfSomethingHappened1.Notify(this, ...);
		ntfSomethingHappened2.Notify(...);
	}
};

class CTarget
{
public:
	CTarget(CSender oSender);

	void onSomething(...);
	static void onSomethingStatic(...);

	NCD_DECLARE_CONNECTION(onSomething)
	NCD_DECLARE_CONNECTION_TATIC(onSomethingStatic)
};

NCD_DEFINE_CONNECTION_STATIC(CTarget, onSomethingStatic)

CTarget::CTarget(CSender oSender)
{
	NCD_INIT_CONNECTION(CTarget, onSomething)
	NCD_INIT_CONNECTION_AND_LINK(CTarget, onSomething, oSender.ntfSomethingHappened1)
		...
	NCD_CONNECT_LOCAL(oSender.ntfSomethingHappened1, onSomething)
	NCD_CONNECT(oSender.ntfSomethingHappened2, CTarget, onSomething)
		...
	NCD_DCONNECT_LOCAL(oSender.ntfSomethingHappened1, onSomething)	// optional
	NCD_DCONNECT(oSender.ntfSomethingHappened1, CTarget, onSomething)	// optional
}
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef NCD_NOTIFICATION_H
#define NCD_NOTIFICATION_H

//
//	Includes
//
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <utility>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Defines - NCD stands as a prefix for the 'Notification - Connection - Delegate' scope
//
// Standartized connection name: cnt stands for the word 'connection'
#define NCD_CONNECTION_NAME(_Listener_Name_)																		\
	cnt_##_Listener_Name_

// Connection declaration
#define NCD_DECLARE_CONNECTION(_Listener_Name_)																		\
	::ntf::Connection<decltype(&_Listener_Name_)>	NCD_CONNECTION_NAME(_Listener_Name_);

// Workaround for the (TSender* pSender) issue, will be removed after switching VS 2017 compiler (waiting to C++17)
#define NCD_DECLARE_CONNECTION2(_Listener_Name_)																	\
	::ntf::Connection2<decltype(&_Listener_Name_)>	NCD_CONNECTION_NAME(_Listener_Name_);

// Connection initialization
#define NCD_INIT_CONNECTION(_ClassName_, _Listener_Name_)															\
	NCD_CONNECTION_NAME(_Listener_Name_).Init<&_ClassName_::_Listener_Name_>(*this);

#define NCD_INIT_CONNECTION_AND_LINK(_ClassName_, _Listener_Name_, _Ntfctn_)										\
	NCD_CONNECTION_NAME(_Listener_Name_).Init<&_ClassName_::_Listener_Name_>(_Ntfctn_, *this);

// Connect notification with connection
// Connections should be initialized before connecting with notifications
#define NCD_CONNECT(_Ntfctn_, _Receiver_, _Listener_Name_)															\
	(_Ntfctn_).AddConnection(_Receiver_.NCD_CONNECTION_NAME(_Listener_Name_));

#define NCD_CONNECT_LOCAL(_Ntfctn_, _Listener_Name_)																\
	(_ntfctn_).AddConnection(NCD_CONNECTION_NAME(_Listener_Name_));

// Disconnect notification from its connection
#define NCD_DISCONNECT(_Ntfctn_, _Receiver_, _Listener_Name_)														\
	(_Ntfctn_).RemoveConnection(_Receiver_.NCD_CONNECTION_NAME(_Listener_Name_));

#define NCD_DISCONNECT_LOCAL(_Ntfctn_, _Listener_Name_)																\
	(_Ntfctn_).RemoveConnection(NCD_CONNECTION_NAME(_Listener_Name_));

#define NCD_DISCONNECT_ALL(_Listener_Name_)																			\
	(_Ntfctn_).RemoveAllConnections();

#define NCD_DISCONNECT_ALL_LOCAL(_Listener_Name_)																	\
	(_Ntfctn_).RemoveAllConnections();

// Static connection declaration
#define NCD_DECLARE_CONNECTION_STATIC(_Listener_Name_)																\
	static ::ntf::Connection<decltype(&_Listener_Name_)>	NCD_CONNECTION_NAME(_Listener_Name_);

// Static connection definition & initialization
#define NCD_DEFINE_CONNECTION_STATIC(_ClassName_, _Listener_Name_)													\
	::ntf::Connection<decltype(&_ClassName_::_Listener_Name_)>	_ClassName_::NCD_CONNECTION_NAME(_Listener_Name_);	\
	class StaticConnectionInitializer_##_ClassName_##_##_Listener_Name_												\
	{																												\
		inline StaticConnectionInitializer_##_ClassName_##_##_Listener_Name_()										\
		{																											\
			_ClassName_::NCD_CONNECTION_NAME(_Listener_Name_).Init<&_ClassName_::_Listener_Name_>();				\
		}																											\
		StaticConnectionInitializer_##_ClassName_##_##_Listener_Name_ instance;										\
	};																												\
	StaticConnectionInitializer_##_ClassName_##_##_Listener_Name_													\
		StaticConnectionInitializer_##_ClassName_##_##_Listener_Name_::instance;

#define NCD_INIT_CONNECTION_STATIC(_ClassName_, _Listener_Name_)													\
	NCD_CONNECTION_NAME(_Listener_Name_).Init<&_ClassName_::_Listener_Name_>();

#define NCD_INIT_CONNECTION_STATIC_AND_LINK(_ClassName_, _Listener_Name_, _Ntfctn_)									\
	NCD_CONNECTION_NAME(_Listener_Name_).Init<&_ClassName_::_Listener_Name_>(_Ntfctn_);

// Connection scoped mute/unmute
#define NCD_MUTE_CONNECTION(_Listener_Name_)																		\
	::ntf::ConnectionMuter o##_Listener_Name_##Muter(MYM_CONNECTION_NAME(_Listener_Name_));

// Notifications scoped blocking
#define NCD_BLOCK(_Ntfctn_)																							\
	::ntf::NotificationBlocker o##_Ntfctn_##Blocker(_Ntfctn_);
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace ncd { // Notification - Connection - Delegate
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Delegate
//	Encapsulates and bounds together function pointer and target object reference as a single callable object
//

// Generic delegate declaration
template <typename TCallable> class TDelegate;

// Partial specialization for Function like syntax
template <typename TRetVal, typename ...TArguments>
class TDelegate<TRetVal(TArguments...)> final
{
public:
	//
	// Construction
	//

	// Default constructor (constructs null delegate)
	inline TDelegate() = default;
	// Destructor
	inline ~TDelegate() = default;
	// Copy constructor
	inline TDelegate(TDelegate const& other) = default;
	// Move constructor
	inline TDelegate(TDelegate&& other) = default;

private:
	// Internal constructor
	using t_pobSender = void*;
	using t_pobReceiver = void*;
	using t_pfnCallback = TRetVal(*)(t_pobSender pSender, t_pobReceiver pReceiver, TArguments... args);

	inline TDelegate(t_pobReceiver pTargetObject, t_pfnCallback pFunctionCaller) :
		m_tCallback(pTargetObject, pFunctionCaller)
		{}

public:
	//
	// Creation
	//

	// Constructor for TReceiver::TMethod(TArguments...)
	template <class TReceiver, TRetVal(TReceiver::*TMethod)(TArguments...)>
	static TDelegate Create(TReceiver& oTargetObject)
		{return TDelegate((t_pobReceiver) &oTargetObject, MethodCaller<TReceiver, TMethod>);}

	// Constructor for TReceiver::TMethod(TArguments...) const 
	template <typename TReceiver, TRetVal(TReceiver::*TMethod)(TArguments...) const>
	static TDelegate Create(TReceiver const& oTargetObject)
		{return TDelegate((t_pobReceiver) &oTargetObject, ConstMethodCaller<TReceiver, TMethod>);}

	// Constructor for static TFunction(TArguments...)
	template <TRetVal(*TFunction)(TArguments...)>
	static TDelegate Create()
		{return TDelegate((t_pobReceiver) nullptr, FunctionCaller<TFunction>);}

	// Constructor for Lambda/TFunctor(TArguments...)
	template <typename TFunctor>
	static TDelegate Create(TFunctor const& oTargetObject)
		{return TDelegate((t_pobReceiver) &oTargetObject, FunctorCaller<TFunctor>);}

	// Constructor with Sender for TReceiver::TMethod(TSender*, TArguments...)
	template <class TSender, class TReceiver, TRetVal(TReceiver::*TMethod)(TSender*, TArguments...)>
	static TDelegate CreateEx(TReceiver& oTargetObject)
		{return TDelegate((t_pobReceiver) &oTargetObject, MethodCallerWithSender<TSender, TReceiver, TMethod>);}

	// Constructor with Sender for TReceiver::TMethod(TSender*, TArguments...) const 
	template <typename TSender, typename TReceiver, TRetVal(TReceiver::*TMethod)(TSender*, TArguments...) const>
	static TDelegate CreateEx(TReceiver const& oTargetObject)
		{return TDelegate((t_pobReceiver) &oTargetObject, ConstMethodCallerWithSender<TSender, TReceiver, TMethod>);}

	// Constructor with Sender for static TFunction(TSender*, TArguments...)
	template <typename TSender, TRetVal(*TFunction)(TSender*, TArguments...)>
	static TDelegate CreateEx()
		{return TDelegate((t_pobReceiver) nullptr, FunctionCallerWithSender<TSender, TFunction>);}

	// Constructor with Sender for TFunctor(TSender*, TArguments...)
	template <typename TSender, typename TFunctor>
	static TDelegate CreateEx(TFunctor const& oTargetObject)
		{return TDelegate((t_pobReceiver) &oTargetObject, FunctorCallerWithSender<TSender, TFunctor>);}

public:
	//
	//	Operators
	//
	inline TDelegate& operator = (TDelegate const& other) = default;
	inline TDelegate& operator = (TDelegate&& other) = default;

	inline TDelegate& operator = (nullptr_t)
		{m_tCallback = nullptr;}

	inline bool operator == (TDelegate const& other) const
		{m_tCallback == other.m_tCallback;}

	inline bool operator == (nullptr_t) const
		{m_tCallback == nullptr;}

	inline bool operator != (TDelegate const& other) const
		{m_tCallback != other.m_tCallback;}

	inline bool operator != (nullptr_t) const
		{m_tCallback != nullptr;}

	template <typename TSender>
	inline TRetVal operator () (TSender* pSender, TArguments... args) const
		{return (*m_tCallback.pFunc)(pSender, m_tCallback.pObj, args...);}

public:
	//
	//	Methods
	//
	inline bool IsNull() const
		{return (m_tCallback == nullptr);}

private:
	//
	//	Implementation
	//
	struct SCallbackItem
	{
		t_pobReceiver pObj = nullptr;
		t_pfnCallback pFunc = nullptr;

		inline SCallbackItem() = default;
		inline SCallbackItem(SCallbackItem const&) = default;
		inline SCallbackItem(SCallbackItem&& o) :
			pObj(o.pObj), pFunc(o.pFunc)
			{o.pObj = nullptr; o.pFunc = nullptr;}
		inline SCallbackItem(t_pobReceiver obj, t_pfnCallback fn) :
			pObj(obj), pFunc(fn)
			{}

		inline void operator=(SCallbackItem const& o)
			{pObj = o.pObj; pFunc = o.pFunc;}
		inline void operator=(SCallbackItem&& o)
			{pObj = o.pObj; pFunc = o.pFunc; o.pObj = nullptr; o.pFunc = nullptr;}
		inline void operator=(nullptr_t)
			{pObj = nullptr; pFunc = nullptr;}
		inline bool operator ==(SCallbackItem const& o) const
			{return o.pObj == pObj && o.pFunc == pFunc;}
		inline bool operator !=(SCallbackItem const& o) const
			{return o.pObj != pObj || o.pFunc != pFunc;}
		inline bool operator ==(nullptr_t) const
			{return pFunc == nullptr;}
		inline bool operator !=(nullptr_t) const
			{return pFunc != nullptr;}
	};

	//
	//	Caller stubs specialized for different use-cases
	//
	template <class TReceiver, TRetVal(TReceiver::*TMethod)(TArguments...)>
	static TRetVal MethodCaller(t_pobSender, t_pobReceiver pObj, TArguments... args)
	{
		TReceiver* pTargetObj = static_cast<TReceiver*>(pObj);
		return (pTargetObj->*TMethod)(args...);
	}

	template <class TReceiver, TRetVal(TReceiver::*TMethod)(TArguments...) const>
	static TRetVal ConstMethodCaller(t_pobSender, t_pobReceiver pObj, TArguments... args)
	{
		TReceiver const* pTargetObj = static_cast<TReceiver*>(pObj);
		return (pTargetObj->*TMethod)(args...);
	}

	template <TRetVal(*TMethod)(TArguments...)>
	static TRetVal FunctionCaller(t_pobSender, t_pobReceiver, TArguments... args)
	{
		return (TMethod) (args...);
	}

	template <typename TFunctor>
	static TRetVal FunctorCaller(t_pobSender, t_pobReceiver pObj, TArguments... args)
	{
		TFunctor* pFuncObj = static_cast<TFunctor*>(pObj);
		return (pFuncObj->operator())(args...);
	}

	template <class TSender, class TReceiver, TRetVal(TReceiver::*TMethod)(TSender*, TArguments...)>
	static TRetVal MethodCallerWithSender(t_pobSender pSender, t_pobReceiver pObj, TArguments... args)
	{
		TReceiver* pTargetObj = static_cast<TReceiver*>(pObj);
		return (pTargetObj->*TMethod)(static_cast<TSender*>(pSender), args...);
	}

	template <class TSender, class TReceiver, TRetVal(TReceiver::*TMethod)(TSender*, TArguments...) const>
	static TRetVal ConstMethodCallerWithSender(t_pobSender pSender, t_pobReceiver pObj, TArguments... args)
	{
		TReceiver const* pTargetObj = static_cast<TReceiver*>(pObj);
		return (pTargetObj->*TMethod)(static_cast<TSender*>(pSender), args...);
	}

	template <class TSender, TRetVal(*TMethod)(TSender*, TArguments...)>
	static TRetVal FunctionCallerWithSender(t_pobSender pSender, t_pobReceiver, TArguments... args)
	{
		return (TMethod) (static_cast<TSender*>(pSender), args...);
	}

	template <class TSender, typename Functor>
	static TRetVal FunctorCallerWithSender(t_pobSender pSender, t_pobReceiver pObj, TArguments... args)
	{
		Functor* pFuncObj = static_cast<Functor*>(pObj);
		return (pFuncObj->operator())(static_cast<TSender*>(pSender), args...);
	}

private:
	// Contents
	SCallbackItem	m_tCallback;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration of the Base class for connection objects
class CConnectionBase;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CNotificationBase
//
//	Serves as the base class for Notification objects
//	Keeps and manages connections, could be Blocked/unblocked
//
class CNotificationBase
{
protected:
	//
	// Constructors
	//
	inline CNotificationBase() = default;

	CNotificationBase(CNotificationBase const&) = delete;
	CNotificationBase(CNotificationBase&&) = delete;

	void operator=(CNotificationBase const&) = delete;
	void operator=(CNotificationBase&&) = delete;

public:
	inline ~CNotificationBase();

public:
	//
	// Methods
	//

	// Returns true if the notification hac active connections
	inline bool HasConnections() const;
	// Returns true if the specified connection is connected to this Notification
	inline bool IsConnected(CConnectionBase const& oCnctn) const;

	// Removes specifed connection from the Notification
	// Returns true if connection found and removed, false if connection not found
	inline bool RemoveConnection(CConnectionBase const& oCnctn) const;
	// Removes all connections
	inline void RemoveAllConnections() const;

	///////////////////////////////////////////////////////////////////////////////
	//
	//	CNotificationBlocker
	//	Block specifiend notification from firing upon construction and
	//	automatically unblocks it upon destruction
	//	Notification should not be deleted while it is Blocked otherwise behavior is undefined
	//
	class CBlocker
	{
	public:
		inline CBlocker(CNotificationBase& oNtfctn);
		inline CBlocker(CBlocker&&);
		inline ~CBlocker();

		CBlocker(CBlocker const&) = delete;
		void operator=(CBlocker const&) = delete;
		void operator=(CBlocker&&) = delete;

		inline void Release();

	private:
		CNotificationBase&	m_oNtfctn;
		bool const			m_bPrevState;
		bool				m_bReleased;
	};
	///////////////////////////////////////////////////////////////////////////////

	// Returns Notifications blocked state
	// Blocked notification immediately returns without invoking its connections
	inline bool IsBlocked() const;
	// Sets notification blocked state accordingly, returns previous blocked state
	inline bool SetBlockedState(bool bMute);
	// Blocks notification and returns its scoped blocker (will be unblocked automatically)
	inline CBlocker Block();

protected:
	//
	//	Implementation
	//
	inline void Add(CConnectionBase const* pCnctn) const;
	inline bool Remove(CConnectionBase const* pCnctn) const;

protected:
	//
	// Contents
	//
	bool m_blocked = false;
	mutable std::vector<CConnectionBase const*> m_aConnections;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CConnectionBase
//	Serves as the base class for connection objects
//	Tracks connected notifications and manages notifications Mute state
//
class CConnectionBase
{
protected:
	inline CConnectionBase() = default;

	CConnectionBase(CConnectionBase const&) = delete;
	CConnectionBase(CConnectionBase&&) = delete;

	void operator=(CConnectionBase const&) = delete;
	void operator=(CConnectionBase&&) = delete;

public:
	inline ~CConnectionBase();

public:
	//
	// Methods
	//

	// Returns weather connection contains connected notifications
	inline bool HasConnectedNotifications() const;
	// Returns true if specified Notification is connected
	inline bool IsConnected(CNotificationBase const& oNtfctn) const;

	// Disconnectes from the specified Notification
	inline bool Disconnect(CNotificationBase const& oNtfctn) const;
	// Disconnectes from all connected Notifications
	inline void DisconnectAll() const;

	///////////////////////////////////////////////////////////////////////////////
	//
	//	Muter
	//	Temporary disables connection object and turns on upon destruction
	//
	class CMuter
	{
	public:
		inline CMuter(CConnectionBase& oCnctn);
		inline CMuter(CMuter&&);
		inline ~CMuter();

		CMuter(CMuter const&) = delete;
		void operator=(CMuter const&) = delete;
		void operator=(CMuter&&) = delete;

		inline void Release();

	private:
		CConnectionBase&	m_oConnection;
		bool const			m_bPrevState;
		bool				m_bReleased;
	};
	///////////////////////////////////////////////////////////////////////////////

	// Returns connections Muted (enabled/disabled) state
	inline bool IsMuted() const;
	// Sets Connection muted state accordingly, returns previous state 
	inline bool SetMuteState(bool bMute);
	// Mutes connection and returns its scoped muter (will be unmuted automatically)
	inline CMuter Mute();

protected:
	//
	//	Implementation
	//
	inline void Add(CNotificationBase const* pNtfctn) const;
	inline bool Remove(CNotificationBase const* pNtfctn) const;

protected:
	// Controls connection enabled/disabled state
	// Notifications could stay connected but if the connection is not enabledit should not pass calls to the delegate
	bool m_bMuted = false;
	// Set of connected Notifications (senders)
	mutable std::unordered_set<CNotificationBase const*> m_setConnections;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Notification class forward declaration
template <typename ...TArguments>
class TNotification;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Connection
//	Initializes with Delegate object
//	Could be connected to the Notifications and Invoked externally
//
template <typename ...TArguments>
class TConnection : public CConnectionBase
{
public:
	//
	//	Constructors
	//
	using DelegateType = TDelegate<void(TArguments...)>;
	using NotificationType = TNotification<TArguments...>;

	inline TConnection() = default;
	inline TConnection(DelegateType oDelegate);
	inline TConnection(NotificationType const& oNtfctn, DelegateType oDelegate);

public:
	//
	//	Methods
	//

	// Initializers
	inline void Init(DelegateType const& oDelegate);
	inline void Init(NotificationType const& oNtfctn, DelegateType const& oDelegate);

	// Connects specified notification to the associated delegate, if the Notification already connected does nothing
	inline void Connect(NotificationType const& oNtfctn) const;

	// Invokes associated delegate with specifed arguments
	// Usually this method called by corresponding Notifications conntected to this connection
	template <typename TSender>
	inline void Invoke(TSender* pSenderObject, TArguments... args) const;

private:
	// Contents
	DelegateType	m_oDelegate;
};

//
//	Connection helper
//	Encapsulates Delegate instatiation from the outside use
//
template <typename TCallable> class TConnectionX;
// Temporary solution for the (TSender* pSender) issue untill will sweatch to VS 2017 compiler (waiting to C++17 to remove)
template <typename TCallable> class TConnectionX2;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Notification
//	Could be emitted and connected
//
template <typename... TArguments>
class TNotification : public CNotificationBase
{
public:
	//
	//	Constructors
	//
	inline TNotification() = default;
	inline ~TNotification() = default;

public:
	//
	// Methods
	//
	using ConnectionType = TConnection<TArguments...>;

	// Adds specified connection to the notification (appends to the end)
	// If connection already exist Add just moves it to the end
	inline void AddConnection(ConnectionType const& oCnctn) const;

	// Emits the notification with the specified sender and arguments
	template <typename TSender>
	inline void Notify(TSender* pSender, TArguments... args) const;

public:
	//
	// Operators
	//
	inline TNotification& operator += (ConnectionType const& oCnctn);
	inline TNotification& operator -= (ConnectionType const& oCnctn);

	template <typename TSender>
	inline void operator () (TSender* pSender, TArguments... args) const;
};

//
//	Notification helper
//	Restricts Notify method calls and allows it only for the Sender
//
template <class TSender, typename... TArguments>
class TNotificationX : public TNotification<TArguments...>
{
public:
	inline TNotificationX();
	inline ~TNotificationX() = default;

	using NotificationType = TNotification<TArguments...>;
	using ConnectionType = typename NotificationType::ConnectionType;


	// Notify method - invokes all connections
	inline void Notify(TSender* pSender, TArguments... args) const;
	inline void operator() (TSender* pSender, TArguments... args) const;

	//	Notifaction to Notification connection
	//	Embedded connection object to link Notifications with same sender & argument types 
	ConnectionType cnt_Notify; // cnt - stands as abbreviation for the word 'connection'
};

//
//	Notification helper
//	Keeps its sender and allows linking of different Notifications with same arguments
//
template <class TSender, typename... TArguments>
class TNotificationEX final : public TNotificationX<TSender, TArguments...>
{
public:
	inline TNotificationEX(TSender& oSender);
	inline ~TNotificationEX() = default;

	using Base = TNotificationX<TSender, TArguments...>;
	using NotificationType = typename Base::NotificationType;
	using ConnectionType = typename Base::ConnectionType;

	// Notify method
	inline void Notify(TArguments... args) const;
	inline void operator() (TArguments... args) const;

private:
	// Own sender object
	TSender& m_oSender;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	Notifications connecting/disconnecting mechanism
//
//! Connects given notifications to the master notifaction,
//! if connected notification(s) emits then ArgMasterNtfctn will be emited too
template <typename TMasterNtfctn, typename TNtfctn1, typename... TNtfctns>
void ConnectNotifications(TMasterNtfctn& ArgMasterNtfctn, TNtfctn1& ArgNtfctn1, TNtfctns&... ArgNtfctns)
{
	NCD_CONNECT(ArgNtfctn1, ArgMasterNtfctn, Notify);		// Connect first notification
	ConnectNotifications(ArgMasterNtfctn, ArgNtfctns...);	// Connect other notifactions
}

//! Disconnects connected notifactions from the master notification
template <typename TMasterNtfctn, typename TNtfctn1, typename... TNtfctns>
void DisconnectNotifications(TMasterNtfctn& ArgMasterNtfctn, TNtfctn1& ArgNtfctn1, TNtfctns&... ArgNtfctns)
{
	NCD_DISCONNECT(ArgNtfctn1, ArgMasterNtfctn, Notify);		// Disconnect first notification
	DisconnectNotifications(ArgMasterNtfctn, ArgNtfctns...);	// Disconnect other notifications
}

template <typename TMasterNtfctn>
inline void ConnectNotifications(TMasterNtfctn const&) {}		// Finalizing function of the template ConnectNotifications()
template <typename TMasterNtfctn>
inline void DisconnectNotifications(TMasterNtfctn const&) {}	// Finalizing function of the template DisonnectNotifications()
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
//	Final Notification definition for basic usage
//
template <class TSender, typename... TArguments>
using Notification = TNotificationX<TSender, TArguments...>;

//
//	Final Notification definition for extended usage
//
template <class TSender, typename... TArguments>
using NotificationEx = TNotificationEX<TSender, TArguments...>;

//
//	Final connection definition for the external use
// 
template <typename TCallable>
using Connection = TConnectionX<TCallable>;

template <typename TCallable>
using Connection2 = TConnectionX2<TCallable>;

//
// NotificationBlocker definition for external use
//
using NotificationBlocker = CNotificationBase::CBlocker;

//
//	ConnectionMuter definition for external use
//
using ConnectionMuter = CConnectionBase::CMuter;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CNotificationBase Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline CNotificationBase::~CNotificationBase()
{
	RemoveAllConnections();
}

inline bool CNotificationBase::HasConnections() const
{
	return !m_aConnections.empty();
}

inline bool CNotificationBase::IsConnected(CConnectionBase const& oCnctn) const
{
	auto it = std::find(m_aConnections.begin(), m_aConnections.end(), &oCnctn);
	return (it != m_aConnections.end());
}

inline bool CNotificationBase::RemoveConnection(CConnectionBase const& oCnctn) const
{
	bool bRemoved = Remove(&oCnctn);
	if (bRemoved && oCnctn.HasConnectedNotifications() && oCnctn.IsConnected(*this))
		oCnctn.Disconnect(*this);
	return bRemoved;
}

inline void CNotificationBase::RemoveAllConnections() const
{
	auto aConnections = std::move(m_aConnections);
	for (CConnectionBase const* pCnctn : aConnections)
	{
		if (pCnctn->HasConnectedNotifications() && pCnctn->IsConnected(*this))
			pCnctn->Disconnect(*this);
	}
}

inline bool CNotificationBase::IsBlocked() const
{
	return m_blocked;
}

inline bool CNotificationBase::SetBlockedState(bool bNewState)
{
	bool bOldState = m_blocked;
	m_blocked = bNewState;
	return bOldState;
}

inline CNotificationBase::CBlocker CNotificationBase::Block()
{
	return std::move(CBlocker(*this));
}

inline void CNotificationBase::Add(CConnectionBase const* pCnctn) const
{
	Remove(pCnctn);
	m_aConnections.push_back(pCnctn);
}

inline bool CNotificationBase::Remove(CConnectionBase const* pCnctn) const
{
	bool bRemoved = false;
	auto it = std::find(m_aConnections.begin(), m_aConnections.end(), pCnctn);
	if (it != m_aConnections.end())
	{
		bRemoved = true;
		m_aConnections.erase(it);
	}
	return bRemoved;
}

//
//	CBlocker
//
inline CNotificationBase::CBlocker::CBlocker(CNotificationBase& oNtfctn)
	: m_oNtfctn(oNtfctn), m_bPrevState(oNtfctn.SetBlockedState(true)), m_bReleased(false)
{
}

inline CNotificationBase::CBlocker::CBlocker(CBlocker&& o)
	: m_oNtfctn(o.m_oNtfctn), m_bPrevState(o.m_bPrevState), m_bReleased(o.m_bReleased)
{
	o.m_bReleased = true;
}

inline CNotificationBase::CBlocker::~CBlocker()
{
	Release();
}

inline void CNotificationBase::CBlocker::Release()
{
	if (!m_bReleased)
	{
		m_bReleased = true;
		m_oNtfctn.SetBlockedState(m_bPrevState);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	CConnectionBase Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
inline CConnectionBase::~CConnectionBase()
{
	DisconnectAll();
}

inline bool CConnectionBase::HasConnectedNotifications() const
{
	return !m_setConnections.empty();
}

inline bool CConnectionBase::IsConnected(CNotificationBase const& oNtfctn) const
{
	return (m_setConnections.count(&oNtfctn) > 0);
}

inline bool CConnectionBase::Disconnect(CNotificationBase const& oNtfctn) const
{
	bool bRemoved = Remove(&oNtfctn);
	if (bRemoved && oNtfctn.HasConnections() && oNtfctn.IsConnected(*this))
		oNtfctn.RemoveConnection(*this);
	return bRemoved;
}

inline void CConnectionBase::DisconnectAll() const
{
	auto aCnctns = std::move(m_setConnections);
	for (CNotificationBase const* pNtfctn : aCnctns)
	{
		if (pNtfctn->HasConnections() && pNtfctn->IsConnected(*this))
			pNtfctn->RemoveConnection(*this);
	}
}

inline bool CConnectionBase::IsMuted() const
{
	return m_bMuted;
}

inline bool CConnectionBase::SetMuteState(bool bMute)
{
	bool bPrevMuted = m_bMuted;
	m_bMuted = bMute;
	return bPrevMuted;
}

inline CConnectionBase::CMuter CConnectionBase::Mute()
{
	return std::move(CMuter(*this));
}

inline void CConnectionBase::Add(CNotificationBase const* pNtfctn) const
{
	m_setConnections.insert(pNtfctn);
}

inline bool CConnectionBase::Remove(CNotificationBase const* pNtfctn) const
{
	return (m_setConnections.erase(pNtfctn) > 0);
}

//
//	CMuter
//
inline CConnectionBase::CMuter::CMuter(CConnectionBase& oCnctn)
	: m_oConnection(oCnctn), m_bPrevState(oCnctn.SetMuteState(true)), m_bReleased(false)
{
}

inline CConnectionBase::CMuter::CMuter(CMuter&& o)
	: m_oConnection(o.m_oConnection), m_bPrevState(o.m_bPrevState), m_bReleased(o.m_bReleased)
{
	o.m_bReleased = true;
}

inline CConnectionBase::CMuter::~CMuter()
{
	Release();
}

inline void CConnectionBase::CMuter::Release()
{
	if (!m_bReleased)
	{
		m_bReleased = true;
		m_oConnection.SetMuteState(m_bPrevState);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TConnection Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... TArguments>
inline TConnection<TArguments...>::TConnection(DelegateType oDelegate) :
	m_oDelegate(oDelegate)
{
}

template <typename... TArguments>
inline TConnection<TArguments...>::TConnection(NotificationType const& oNtfctn, DelegateType oDelegate) :
	m_oDelegate(oDelegate)
{
	Connect(oNtfctn);
}

template <typename... TArguments>
inline void TConnection<TArguments...>::Init(DelegateType const& oDelegate)
{
	CConnectionBase::DisconnectAll();
	m_oDelegate = oDelegate;
}

template <typename... TArguments>
inline void TConnection<TArguments...>::Init(NotificationType const& oNtfctn, DelegateType const& oDelegate)
{
	Init(oDelegate);
	Connect(oNtfctn);
}

template <typename... TArguments>
inline void TConnection<TArguments...>::Connect(NotificationType const& oNtfctn) const
{
	//ASSERT(!m_oDelegate.IsNull(), "Connection object should be initialized first then linied.");
	Add(&oNtfctn);
	if (!oNtfctn.IsConnected(*this))
		oNtfctn.AddConnection(*this);
}

template <typename... TArguments>
template <typename TSender>
inline void TConnection<TArguments...>::Invoke(TSender* pSender, TArguments... args) const
{
	if (!m_bMuted && !m_oDelegate.IsNull())
		m_oDelegate(pSender, args...);
}

//
//	Connection helpers
//

// Connection specialization for class member functions (1)
template <class TReceiver, typename ...TArguments>
class TConnectionX<void(TReceiver::*)(TArguments...)> : public TConnection<TArguments...>
{
public:
	//	Type definitions
	using ConnectionType = TConnection<TArguments...>;
	using DelegateType = typename ConnectionType::DelegateType;
	using NotificationType = typename ConnectionType::NotificationType;

	//	Constructors
	inline TConnectionX() = default;

	// Initializers
	template <void(TReceiver::*TMethod)(TArguments...)>
	inline void Init(TReceiver& oTargetObject)
	{
		ConnectionType::Init(DelegateType::template Create<TReceiver, TMethod>(oTargetObject));
	}

	template <void(TReceiver::*TMethod)(TArguments...)>
	inline void Init(NotificationType const& oNtfctn, TReceiver& oTargetObject)
	{
		ConnectionType::Init(oNtfctn, DelegateType::template Create<TReceiver, TMethod>(oTargetObject));
	}
};

// Connection specialization for class member const functions (2)
template <class TReceiver, typename ...TArguments>
class TConnectionX<void(TReceiver::*)(TArguments...) const> : public TConnection<TArguments...>
{
public:
	//	Type definitions
	using ConnectionType = TConnection<TArguments...>;
	using DelegateType = typename ConnectionType::DelegateType;
	using NotificationType = typename ConnectionType::NotificationType;

	//	Constructors
	inline TConnectionX() = default;

	// Initializers
	template <void(TReceiver::*TMethod)(TArguments...) const>
	inline void Init(TReceiver& oTargetObject)
	{
		ConnectionType::Init(DelegateType::template Create<TReceiver, TMethod>(oTargetObject));
	}

	template <void(TReceiver::*TMethod)(TArguments...) const>
	inline void Init(NotificationType const& oNtfctn, TReceiver& oTargetObject)
	{
		ConnectionType::Init(oNtfctn, DelegateType::template Create<TReceiver, TMethod>(oTargetObject));
	}
};

// Connection specialization for static functions (3)
template <typename ...TArguments>
class TConnectionX<void(*)(TArguments...)> : public TConnection<TArguments...>
{
public:
	//	Type definitions
	using ConnectionType = TConnection<TArguments...>;
	using DelegateType = typename ConnectionType::DelegateType;
	using NotificationType = typename ConnectionType::NotificationType;

	//	Constructors
	inline TConnectionX() = default;

	// Initializers
	template <void(*TFunction)(TArguments...)>
	inline void Init()
	{
		ConnectionType::Init(DelegateType::template Create<TFunction>());
	}

	template <void(*TFunction)(TArguments...)>
	inline void Init(NotificationType const& oNtfctn)
	{
		ConnectionType::Init(oNtfctn, DelegateType::template Create<TFunction>());
	}
};

//
//	Temporary solution for the (TSender* pSender) issue untill will sweatch to VS 2017 compiler
//	waiting to C++17 to switch to TConnectionX
//
// Connection specialization for class member functions with Sender (4)
template <class TSender, class TReceiver, typename ...TArguments>
class TConnectionX2<void(TReceiver::*)(TSender*, TArguments...)> : public TConnection<TArguments...>
{
public:
	//	Type definitions
	using ConnectionType = TConnection<TArguments...>;
	using DelegateType = typename ConnectionType::DelegateType;
	using NotificationType = typename ConnectionType::NotificationType;

	//	Constructors
	inline TConnectionX2() = default;

	// Initializers
	template <void(TReceiver::*TMethod)(TSender*, TArguments...)>
	inline void Init(TReceiver& oTargetObject)
	{
		ConnectionType::Init(DelegateType::template CreateEx<TSender, TReceiver, TMethod>(oTargetObject));
	}

	template <void(TReceiver::*TMethod)(TSender*, TArguments...)>
	inline void Init(NotificationType const& oNtfctn, TReceiver& oTargetObject)
	{
		ConnectionType::Init(oNtfctn, DelegateType::template CreateEx<TSender, TReceiver, TMethod>(oTargetObject));
	}
};

// Connection specialization for class const member functions with Sender (5)
template <class TSender, class TReceiver, typename ...TArguments>
class TConnectionX2<void(TReceiver::*)(TSender*, TArguments...) const> : public TConnection<TArguments...>
{
public:
	//	Type definitions
	using ConnectionType = TConnection<TArguments...>;
	using DelegateType = typename ConnectionType::DelegateType;
	using NotificationType = typename ConnectionType::NotificationType;

	//	Constructors
	inline TConnectionX2() = default;

	// Initializers
	template <void(TReceiver::*TMethod)(TSender*, TArguments...) const>
	inline void Init(TReceiver& oTargetObject)
	{
		ConnectionType::Init(DelegateType::template CreateEx<TSender, TReceiver, TMethod>(oTargetObject));
	}

	template <void(TReceiver::*TMethod)(TSender*, TArguments...) const>
	inline void Init(NotificationType const& oNtfctn, TReceiver& oTargetObject)
	{
		ConnectionType::Init(oNtfctn, DelegateType::template CreateEx<TSender, TReceiver, TMethod>(oTargetObject));
	}
};

// Connection specialization for static functions with Sender (6)
template <class TSender, typename ...TArguments>
class TConnectionX2<void(*)(TSender*, TArguments...)> : public TConnection<TArguments...>
{
public:
	//	Type definitions
	using ConnectionType = TConnection<TArguments...>;
	using DelegateType = typename ConnectionType::DelegateType;
	using NotificationType = typename ConnectionType::NotificationType;

	//	Constructors
	inline TConnectionX2() = default;

	// Initializers
	template <void(*TFunction)(TSender*, TArguments...)>
	inline void Init()
	{
		ConnectionType::Init(DelegateType::template CreateEx<TSender, TFunction>());
	}

	template <void(*TFunction)(TSender*, TArguments...)>
	inline void Init(NotificationType const& oNtfctn)
	{
		ConnectionType::Init(oNtfctn, DelegateType::template CreateEx<TSender, TFunction>());
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//	TNotification Implementation
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename... TArguments>
inline void TNotification<TArguments...>::AddConnection(ConnectionType const& oCnctn) const
{
	Add(&oCnctn);
	if (!oCnctn.IsConnected(*this))
		oCnctn.Connect(*this);
}

template <typename... TArguments>
template <typename TSender>
inline void TNotification<TArguments...>::Notify(TSender* pSender, TArguments... args) const
{
	if (!m_blocked)
	{
		// Go through connections and invoke them
		for (CConnectionBase const* pCnctnBase : m_aConnections)
		{
			//ASSERT_PTR(pCnctnBase);
			ConnectionType const* pCnctn = static_cast<ConnectionType const*>(pCnctnBase);
			pCnctn->template Invoke<TSender>(pSender, args...);
		}
	}
}

template <typename... TArguments>
inline TNotification<TArguments...>& TNotification<TArguments...>::operator += (ConnectionType const& oCnctn)
{
	AddConnection(oCnctn);
	return *this;
}

template <typename... TArguments>
inline TNotification<TArguments...>& TNotification<TArguments...>::operator -= (ConnectionType const& oCnctn)
{
	RemoveConnection(oCnctn);
	return *this;
}

template <typename... TArguments>
template <typename TSender>
inline void TNotification<TArguments...>::operator () (TSender* pSender, TArguments... args) const
{
	Notify(pSender, args...);
}

//
//	TNotifactionX
//
template <class TSender, typename... TArguments>
inline TNotificationX<TSender, TArguments...>::TNotificationX()
{
	using Me = TNotificationX<TSender, TArguments...>;
	using DelegateType = typename ConnectionType::DelegateType;
	cnt_Notify.Init(DelegateType::template CreateEx<TSender, Me, &Me::Notify>(*this));
}

template <class TSender, typename... TArguments>
inline void TNotificationX<TSender, TArguments...>::Notify(TSender* pSender, TArguments... args) const
{
	NotificationType::template Notify<TSender>(pSender, args...);
}

template <class TSender, typename... TArguments>
inline void TNotificationX<TSender, TArguments...>::operator() (TSender* pSender, TArguments... args) const
{
	Notify(pSender, args...);
}

//
//	TNotifactionEX
//
template <class TSender, typename... TArguments>
inline TNotificationEX<TSender, TArguments...>::TNotificationEX(TSender& owner) :
	m_oSender(owner)
{
	using Me = TNotificationEX<TSender, TArguments...>;
	using DelegateType = typename ConnectionType::DelegateType;
	Base::cnt_Notify.Init(DelegateType::template Create<Me, &Me::Notify>(*this));
}

template <class TSender, typename... TArguments>
inline void TNotificationEX<TSender, TArguments...>::Notify(TArguments... args) const
{
	Base::Notify(&m_oSender, args...);
}

template <class TSender, typename... TArguments>
inline void TNotificationEX<TSender, TArguments...>::operator() (TArguments... args) const
{
	Notify(args...);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace ntf
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //NCD_NOTIFICATION_H
