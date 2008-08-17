//This is file has been generated by xmltokross, you should not edit this file but the files used to generate it.

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <kross/core/manager.h>
#include <kross/core/wrapperinterface.h>
#include <language/duchain/use.h>

class KrossKDevelopUse : public QObject, public Kross::WrapperInterface
{
	Q_OBJECT
	public:
		KrossKDevelopUse(KDevelop::Use* obj, QObject* parent=0) : QObject(parent), wrapped(obj)		{ setObjectName("KDevelop::Use"); }
		void* wrappedObject() const { return wrapped; }

		typedef KDevelop::SimpleRange KDevelopSimpleRange;
		Q_PROPERTY(KDevelopSimpleRange  m_range READ getm_range WRITE setm_range SCRIPTABLE true)
		Q_SCRIPTABLE void setm_range(const KDevelopSimpleRange  val) { wrapped->m_range=val; }
		Q_SCRIPTABLE KDevelopSimpleRange  getm_range() const { return wrapped->m_range; }
		Q_PROPERTY(int  m_declarationIndex READ getm_declarationIndex WRITE setm_declarationIndex SCRIPTABLE true)
		Q_SCRIPTABLE void setm_declarationIndex(const int  val) { wrapped->m_declarationIndex=val; }
		Q_SCRIPTABLE int  getm_declarationIndex() const { return wrapped->m_declarationIndex; }
	private:
		KDevelop::Use* wrapped;
};

bool krossuse_registerHandler(const QByteArray& name, Kross::MetaTypeHandler::FunctionPtr* handler)
{ Kross::Manager::self().registerMetaTypeHandler(name, handler); return false; }

namespace Handlers
{
QVariant _kDevelopUseHandler(void* type)
{
	if(!type) return QVariant();
	KDevelop::Use* t=static_cast<KDevelop::Use*>(type);
	Q_ASSERT(dynamic_cast<KDevelop::Use*>(t));
	return qVariantFromValue((QObject*) new KrossKDevelopUse(t, 0));
}
bool b_KDevelopUse=krossuse_registerHandler("KDevelop::Use*", _kDevelopUseHandler);
QVariant kDevelopUseHandler(KDevelop::Use* type){ return _kDevelopUseHandler(type); }
QVariant kDevelopUseHandler(const KDevelop::Use* type) { return _kDevelopUseHandler((void*) type); }

}
#include "krossuse.moc"
