//
//	Includes
//
#include "../src/ncd_core.h"

#include <iostream>

using namespace ncd;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// Notification usage
//
class CSender1
{
public:
	Notification<CSender1, int, int>	SomethingChanged;
	Notification<CSender1>				NothingChanged;

	void DoSomething()
	{
		SomethingChanged.Notify(this, int(0), int(1));
	}

	void DoNothing()
	{
		NothingChanged.Notify(this);
	}

	void ThankMeForNotifingYou() const
	{
		std::cout << "Sender1: Thank me for notifing you!" << std::endl;
	}
};


class CSender2
{
public:
	NotificationEx<CSender2, int, int>	SomethingChanged;
	NotificationEx<CSender2>			NothingChanged;

	CSender2() :
		SomethingChanged(*this),
		NothingChanged(*this)
	{
	}

	void DoSomething()
	{
		SomethingChanged.Notify(int(2), int(3));
	}

	void DoNothing()
	{
		NothingChanged.Notify();
	}
};


class CListener1
{
public:
	void onSomethingChanged(CSender1*, int, int);
	void onNothingChanged(CSender1*) const;

	TConnection<int, int> m_onSomethingChanged;
	TConnection<> m_onNothingChanged;

	CListener1(CSender1 const&);

	int m_a, m_b;
};

class CListener2
{
public:
	virtual void onSomethingChanged(int, int) = 0;
	virtual void onNothingChanged() const;

	TConnection<int, int> m_onSomethingChanged;
	TConnection<> m_onNothingChanged;

	CListener2();

	int m_a, m_b;
};

class CListener2Der : public CListener2
{
public:
	void onSomethingChanged(int, int) override;
};



CListener1::CListener1(CSender1 const& oSender) :
	m_onSomethingChanged(oSender.SomethingChanged, decltype(m_onSomethingChanged)::DelegateType::CreateEx
						 <CSender1, CListener1, &CListener1::onSomethingChanged>(*this)),
	m_onNothingChanged(oSender.NothingChanged, decltype(m_onNothingChanged)::DelegateType::CreateEx
					   <CSender1, CListener1, &CListener1::onNothingChanged>(*this))
{
}

void CListener1::onSomethingChanged(CSender1* pSender, int a, int b)
{
	m_a = a;
	m_b = b;
	pSender->ThankMeForNotifingYou();
}

void CListener1::onNothingChanged(CSender1* pSender) const
{
	pSender->ThankMeForNotifingYou();
}


CListener2::CListener2() :
	m_onSomethingChanged(decltype(m_onSomethingChanged)::DelegateType::Create<CListener2, &CListener2::onSomethingChanged>(*this)),
	m_onNothingChanged(decltype(m_onNothingChanged)::DelegateType::Create<CListener2, &CListener2::onNothingChanged>(*this))
{
}

void CListener2::onSomethingChanged(int a, int b)
{
	m_a = a;
	m_b = b;
	std::cout << "Listener2: Thank you for notifing me!" << std::endl;
}

void CListener2::onNothingChanged() const
{
	std::cout << "Listener2: Thank you for notifing me!" << std::endl;
}

void CListener2Der::onSomethingChanged(int a, int b)
{
	CListener2::onSomethingChanged(a, b);
	std::cout << "Listener2Der: Thank you twice for notifing me!" << std::endl;
}

void GloballyChanged()
{
	std::cout << "Thanks globallly!" << std::endl;
}

TConnection<> g_CnctnGloballyChanged(TConnection<>::DelegateType::Create<&GloballyChanged>());


class Func
{
public:
	inline void operator()(int a, int b) const
	{
		(void) (a < b);
	}
};


class CListener3
{
public:
	template <typename TEventType>
	CListener3(TEventType const&);

	void onSomethingChanged1(int, int);
	void onSomethingChanged2(int, int) const;
	static void onSomethingChanged3(int, int);

	void onSomethingChanged4(CSender1*, int, int);
	void onSomethingChanged5(CSender1*, int, int) const;
	static void onSomethingChanged6(CSender1*, int, int);

	virtual void onNothingChanged1();
	virtual void onNothingChanged2() const;
	static void onNothingChanged3();

	virtual void onNothingChanged4(CSender1*);
	virtual void onNothingChanged5(CSender1*) const;
	static void onNothingChanged6(CSender1*);

	Connection<decltype(&onNothingChanged1)>	m_onNothingChanged0;


	Connection<decltype(&onSomethingChanged1)>	m_onSomethingChanged1;
	Connection<decltype(&onSomethingChanged2)>	m_onSomethingChanged2;
	Connection<decltype(&onSomethingChanged3)>	m_onSomethingChanged3;
	Connection2<decltype(&onSomethingChanged4)>	m_onSomethingChanged4;
	Connection2<decltype(&onSomethingChanged5)>	m_onSomethingChanged5;
	Connection2<decltype(&onSomethingChanged6)>	m_onSomethingChanged6;

	Connection<decltype(&onNothingChanged1)>	m_onNothingChanged1;
	Connection<decltype(&onNothingChanged2)>	m_onNothingChanged2;
	Connection<decltype(&onNothingChanged3)>	m_onNothingChanged3;
	Connection2<decltype(&onNothingChanged4)>	m_onNothingChanged4;
	Connection2<decltype(&onNothingChanged5)>	m_onNothingChanged5;
	Connection2<decltype(&onNothingChanged6)>	m_onNothingChanged6;

};

template <typename TEventType>
CListener3::CListener3(TEventType const& oNtfctn)
{
	m_onSomethingChanged1.Init<&CListener3::onSomethingChanged1>(oNtfctn, *this);
	m_onSomethingChanged2.Init<&CListener3::onSomethingChanged2>(oNtfctn, *this);
	m_onSomethingChanged3.Init<&CListener3::onSomethingChanged3>(oNtfctn);
	m_onSomethingChanged4.Init<&CListener3::onSomethingChanged4>(oNtfctn, *this);
	m_onSomethingChanged5.Init<&CListener3::onSomethingChanged5>(oNtfctn, *this);
	m_onSomethingChanged6.Init<&CListener3::onSomethingChanged6>(oNtfctn);

	m_onNothingChanged1.Init<&CListener3::onNothingChanged1>(*this);
	m_onNothingChanged2.Init<&CListener3::onNothingChanged2>(*this);
	m_onNothingChanged3.Init<&CListener3::onNothingChanged3>();
	m_onNothingChanged4.Init<&CListener3::onNothingChanged4>(*this);
	m_onNothingChanged5.Init<&CListener3::onNothingChanged5>(*this);
	m_onNothingChanged6.Init<&CListener3::onNothingChanged6>();
}

void CListener3::onSomethingChanged1(int, int)
{
	std::cout << "Listener3::onSomethingChanged (1)" << std::endl;
}

void CListener3::onSomethingChanged2(int, int) const
{
	std::cout << "Listener3::onSomethingChanged (2)" << std::endl;
}

void CListener3::onSomethingChanged3(int, int)
{
	std::cout << "Listener3::onSomethingChanged (3)" << std::endl;
}

void CListener3::onSomethingChanged4(CSender1* p, int, int)
{
	std::cout << "Listener3::onSomethingChanged (4) #" << p << std::endl;
}

void CListener3::onSomethingChanged5(CSender1*, int, int) const
{
	std::cout << "Listener3::onSomethingChanged (5)" << std::endl;
}

void CListener3::onSomethingChanged6(CSender1*, int, int)
{
	std::cout << "Listener3::onSomethingChanged (6)" << std::endl;
}


void CListener3::onNothingChanged1()
{
	std::cout << "Listener3::onNothingChanged (1)" << std::endl;
}

void CListener3::onNothingChanged2() const
{
	std::cout << "Listener3::onNothingChanged (2)" << std::endl;
}

void CListener3::onNothingChanged3()
{
	std::cout << "Listener3::onNothingChanged (3)" << std::endl;
}

void CListener3::onNothingChanged4(CSender1*)
{
	std::cout << "Listener3::onNothingChanged (4)" << std::endl;
}

void CListener3::onNothingChanged5(CSender1*) const
{
	std::cout << "Listener3::onNothingChanged (5)" << std::endl;
}

void CListener3::onNothingChanged6(CSender1*)
{
	std::cout << "Listener3::onNothingChanged (6)" << std::endl;
}


int main()
{
	CSender1* pSender1 = new CSender1;
	CSender2* pSender2 = new CSender2;

	pSender2->SomethingChanged.cnt_Notify.Connect(pSender1->SomethingChanged);
	pSender2->NothingChanged.cnt_Notify.Connect(pSender1->NothingChanged);
	pSender1->NothingChanged.Notification<CSender1>::NotificationType::Notify(pSender1);

	CListener3* pReceiver3 = new CListener3(pSender1->SomethingChanged);

	//pReceiver3->m_onNothingChanged0.Connect(pSender1->NothingChanged);

	pReceiver3->m_onNothingChanged1.Connect(pSender1->NothingChanged);
	pReceiver3->m_onNothingChanged2.Connect(pSender1->NothingChanged);
	pReceiver3->m_onNothingChanged3.Connect(pSender1->NothingChanged);
	pReceiver3->m_onNothingChanged4.Connect(pSender1->NothingChanged);
	pReceiver3->m_onNothingChanged5.Connect(pSender1->NothingChanged);
	pReceiver3->m_onNothingChanged6.Connect(pSender1->NothingChanged);

	pReceiver3->m_onSomethingChanged1.Connect(pSender2->SomethingChanged);
	pReceiver3->m_onSomethingChanged2.Connect(pSender2->SomethingChanged);
	pReceiver3->m_onSomethingChanged3.Connect(pSender2->SomethingChanged);
	pReceiver3->m_onSomethingChanged4.Connect(pSender2->SomethingChanged);
	pReceiver3->m_onSomethingChanged5.Connect(pSender2->SomethingChanged);
	pReceiver3->m_onSomethingChanged6.Connect(pSender2->SomethingChanged);

	pReceiver3->m_onNothingChanged1.Connect(pSender2->NothingChanged);
	pReceiver3->m_onNothingChanged2.Connect(pSender2->NothingChanged);
	pReceiver3->m_onNothingChanged3.Connect(pSender2->NothingChanged);
	pReceiver3->m_onNothingChanged4.Connect(pSender2->NothingChanged);
	pReceiver3->m_onNothingChanged5.Connect(pSender2->NothingChanged);
	pReceiver3->m_onNothingChanged6.Connect(pSender2->NothingChanged);

	pSender1->DoSomething();
	pSender1->DoNothing();

	pSender2->DoSomething();
	pSender2->DoNothing();

	delete pReceiver3;
	pReceiver3 = nullptr;





	CListener1* pReceiver1 = new CListener1(*pSender1);
	CListener2* pReceiver2 = new CListener2Der;


	pReceiver2->m_onSomethingChanged.Connect(pSender2->SomethingChanged);
	pReceiver2->m_onSomethingChanged.Connect(pSender1->SomethingChanged);

	pReceiver2->m_onNothingChanged.Connect(pSender2->NothingChanged);
	pReceiver2->m_onNothingChanged.Connect(pSender1->NothingChanged);

	Func oFunc;
	TConnection<int, int> oFuncCnctn(TConnection<int, int>::DelegateType::Create<Func>(oFunc));
	oFuncCnctn.Connect(pSender1->SomethingChanged);
	oFuncCnctn.Connect(pSender2->SomethingChanged);

	{
		auto fn = [](int a, int b)
		{
			(a < b) ? a : b;
		};

		TConnection<int, int> oLambdaCnctn(pSender1->SomethingChanged, TConnection<int, int>::DelegateType::Create<decltype(fn)>(fn));

		pSender1->DoSomething();
		pSender1->DoNothing();

		pSender2->DoSomething();
		pSender2->DoNothing();
	}

	delete pReceiver2;


	g_CnctnGloballyChanged.Connect(pSender1->NothingChanged);
	g_CnctnGloballyChanged.Connect(pSender2->NothingChanged);

	pSender1->DoSomething();
	pSender1->DoNothing();

	pSender2->DoSomething();
	pSender2->DoNothing();

	delete pSender1;

	pSender2->DoSomething();
	pSender2->DoNothing();

	delete pReceiver1;


	pSender2->DoSomething();
	pSender2->DoNothing();

	pSender2->NothingChanged.RemoveAllConnections();
	pSender2->SomethingChanged.RemoveConnection(oFuncCnctn);

	g_CnctnGloballyChanged.Disconnect(pSender2->NothingChanged);

	pSender2->DoSomething();
	pSender2->DoNothing();

	delete pSender2;

	return 0;
}
