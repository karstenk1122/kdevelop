//This is file has been generated by xmltokross, you should not edit this file but the files used to generate it.

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <kross/core/manager.h>
#include <kross/core/wrapperinterface.h>
#include <interfaces/context.h>

class KrossKDevelopContext : public QObject, public Kross::WrapperInterface
{
	Q_OBJECT
		Q_ENUMS(KDevelop::Context::Type)
		Q_FLAGS( KDevelop::Context::EditorContext KDevelop::Context::FileContext KDevelop::Context::CodeContext KDevelop::Context::ProjectItemContext)

	public:
		KrossKDevelopContext(KDevelop::Context* obj, QObject* parent=0) : QObject(parent), wrapped(obj)		{ setObjectName("KDevelop::Context"); }
		void* wrappedObject() const { return wrapped; }

		Q_SCRIPTABLE int type() const { return wrapped->type(); }
		Q_SCRIPTABLE bool hasType(int x0) const { return wrapped->hasType(x0); }
	private:
		KDevelop::Context* wrapped;
};

class KrossKDevelopEditorContext : public KrossKDevelopContext
{
	Q_OBJECT
	public:
		KrossKDevelopEditorContext(KDevelop::EditorContext* obj, QObject* parent=0) : KrossKDevelopContext(obj, parent), wrapped(obj)
	{ setObjectName("KDevelop::EditorContext"); }
		void* wrappedObject() const { return wrapped; }

		Q_SCRIPTABLE int type() const { return wrapped->type(); }
		Q_SCRIPTABLE KUrl url() const { return wrapped->url(); }
		Q_SCRIPTABLE KTextEditor::Cursor position() const { return wrapped->position(); }
		Q_SCRIPTABLE QString currentLine() const { return wrapped->currentLine(); }
		Q_SCRIPTABLE QString currentWord() const { return wrapped->currentWord(); }
		Q_SCRIPTABLE KTextEditor::View* view() const { return wrapped->view(); }
	private:
		KDevelop::EditorContext* wrapped;
};

class KrossKDevelopFileContext : public KrossKDevelopContext
{
	Q_OBJECT
	public:
		KrossKDevelopFileContext(KDevelop::FileContext* obj, QObject* parent=0) : KrossKDevelopContext(obj, parent), wrapped(obj)
	{ setObjectName("KDevelop::FileContext"); }
		void* wrappedObject() const { return wrapped; }

		Q_SCRIPTABLE int type() const { return wrapped->type(); }
		Q_SCRIPTABLE KUrl::List urls() const { return wrapped->urls(); }
	private:
		KDevelop::FileContext* wrapped;
};

class KrossKDevelopProjectItemContext : public KrossKDevelopContext
{
	Q_OBJECT
	public:
		KrossKDevelopProjectItemContext(KDevelop::ProjectItemContext* obj, QObject* parent=0) : KrossKDevelopContext(obj, parent), wrapped(obj)
	{ setObjectName("KDevelop::ProjectItemContext"); }
		void* wrappedObject() const { return wrapped; }

		Q_SCRIPTABLE int type() const { return wrapped->type(); }
		Q_SCRIPTABLE QList< KDevelop::ProjectBaseItem* > items() const { return wrapped->items(); }
	private:
		KDevelop::ProjectItemContext* wrapped;
};

bool krosscontext_registerHandler(const QByteArray& name, Kross::MetaTypeHandler::FunctionPtr* handler)
{ Kross::Manager::self().registerMetaTypeHandler(name, handler); return false; }

namespace Handlers
{
QVariant _kDevelopProjectItemContextHandler(void* type)
{
	if(!type) return QVariant();
	KDevelop::ProjectItemContext* t=static_cast<KDevelop::ProjectItemContext*>(type);
	Q_ASSERT(dynamic_cast<KDevelop::ProjectItemContext*>(t));
	return qVariantFromValue((QObject*) new KrossKDevelopProjectItemContext(t, 0));
}
bool b_KDevelopProjectItemContext=krosscontext_registerHandler("KDevelop::ProjectItemContext*", _kDevelopProjectItemContextHandler);
QVariant kDevelopProjectItemContextHandler(KDevelop::ProjectItemContext* type){ return _kDevelopProjectItemContextHandler(type); }
QVariant kDevelopProjectItemContextHandler(const KDevelop::ProjectItemContext* type) { return _kDevelopProjectItemContextHandler((void*) type); }

QVariant _kDevelopFileContextHandler(void* type)
{
	if(!type) return QVariant();
	KDevelop::FileContext* t=static_cast<KDevelop::FileContext*>(type);
	Q_ASSERT(dynamic_cast<KDevelop::FileContext*>(t));
	return qVariantFromValue((QObject*) new KrossKDevelopFileContext(t, 0));
}
bool b_KDevelopFileContext=krosscontext_registerHandler("KDevelop::FileContext*", _kDevelopFileContextHandler);
QVariant kDevelopFileContextHandler(KDevelop::FileContext* type){ return _kDevelopFileContextHandler(type); }
QVariant kDevelopFileContextHandler(const KDevelop::FileContext* type) { return _kDevelopFileContextHandler((void*) type); }

QVariant _kDevelopEditorContextHandler(void* type)
{
	if(!type) return QVariant();
	KDevelop::EditorContext* t=static_cast<KDevelop::EditorContext*>(type);
	Q_ASSERT(dynamic_cast<KDevelop::EditorContext*>(t));
	return qVariantFromValue((QObject*) new KrossKDevelopEditorContext(t, 0));
}
bool b_KDevelopEditorContext=krosscontext_registerHandler("KDevelop::EditorContext*", _kDevelopEditorContextHandler);
QVariant kDevelopEditorContextHandler(KDevelop::EditorContext* type){ return _kDevelopEditorContextHandler(type); }
QVariant kDevelopEditorContextHandler(const KDevelop::EditorContext* type) { return _kDevelopEditorContextHandler((void*) type); }

QVariant _kDevelopContextHandler(void* type)
{
	if(!type) return QVariant();
	KDevelop::Context* t=static_cast<KDevelop::Context*>(type);
	Q_ASSERT(dynamic_cast<KDevelop::Context*>(t));
	if(dynamic_cast<KDevelop::EditorContext*>(t)) return _kDevelopEditorContextHandler(type);
	else if(dynamic_cast<KDevelop::FileContext*>(t)) return _kDevelopFileContextHandler(type);
	else if(dynamic_cast<KDevelop::ProjectItemContext*>(t)) return _kDevelopProjectItemContextHandler(type);
	else return qVariantFromValue((QObject*) new KrossKDevelopContext(t, 0));
}
bool b_KDevelopContext=krosscontext_registerHandler("KDevelop::Context*", _kDevelopContextHandler);
QVariant kDevelopContextHandler(KDevelop::Context* type){ return _kDevelopContextHandler(type); }
QVariant kDevelopContextHandler(const KDevelop::Context* type) { return _kDevelopContextHandler((void*) type); }

}
#include "krosscontext.moc"
