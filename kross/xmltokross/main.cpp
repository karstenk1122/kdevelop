/***************************************************************************
 *   Copyright 2008 Aleix Pol <aleixpol@gmail.com>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QFile>
#include <QDebug>
#include <QStringList>
#include "cppxmlparse.h"

class KrossWrapper : public XmlToKross
{
    public:
        KrossWrapper(QXmlStreamReader& _xml) : XmlToKross(_xml) {}
        QString output;
        QString handlersHeader;
        
        void writeDocument()
        {
            handlersHeader += "#ifndef "+filename.toUpper()+"_H\n";
            handlersHeader += "#define "+filename.toUpper()+"_H\n\n";
            handlersHeader += "#include<QtCore/QVariant>\n\n";
            handlersHeader += "//This is file has been generated by xmltokross, "
                              "you should not edit this file but the files used to generate it.\n\n";
                              
            output += "//This is file has been generated by xmltokross, you should not edit this file but the files used to generate it.\n\n"
                      "#include <QtCore/QObject>\n"
                      "#include <QtCore/QVariant>\n"
                      "#include <kross/core/manager.h>\n"
                      "#include <kross/core/wrapperinterface.h>\n";
            foreach(const QString & include, includes)
            {
                output += "#include <"+include+">\n";
                handlersHeader += "#include <"+include+">\n";
            }
            output +='\n';
        }
        
        void writeClass(const QString& classname, const QString& baseclass, const QList<QStringList>& enums)
        {
            classNamespace[classname]=inNamespace;
            definedClasses.append(classname);
            output += "class Kross" + classname + " : public QObject, public Kross::WrapperInterface\n"
                      "{\n"
                      "\tQ_OBJECT\n"
                      "\tpublic:\n"
                      "\t\tKross"+classname+"("+/*inNamespace.isEmpty() ? QString() : inNamespace+"::"
                                 +*/classname+"* obj, QObject* parent=0) : QObject(parent), wrapped(obj) {}\n"
                      "\t\tvoid* wrappedObject() const { return wrapped; }\n\n";
        }
        
        void writeEndClass()
        { output += "\tprivate:\n"
                    "\t\t"+(inNamespace.isEmpty() ? QString() : inNamespace+"::")+definedClasses.last()+"* wrapped;\n"
                    "};\n\n"; }
                    
        void writeVariable(const QString& name, const QString& type, bool isConst)
        {
            QString write;
            if(!isConst)
                write=" WRITE set"+name;
            output += "\t\tQ_PROPERTY("+type+' '+name+" READ get"+name+write+" SCRIPTABLE true)\n";
            if(!isConst) output += "\t\tQ_SCRIPTABLE void set"+name+"(const "+type+"& val) { wrapped->"+name+"=val; }\n";
            output += "\t\tQ_SCRIPTABLE "+type+" get"+name+"() const { return wrapped->"+name+"; }\n";
        }
        
        void writeNamespace(const QString& name)
        {
            output += "using namespace "+name+";\n\n";
        }
        
        void writeEndEnum(const QStringList& fl)
        {
            QStringList flags=fl;
            output += "\t\tQ_ENUMS("+(inNamespace.isEmpty() ? QString() : inNamespace+"::")+definedClasses.last()+flags.takeFirst()+");\n"
                      "\t\tQ_FLAGS(";
            
            foreach(const QString& flag, flags)
                output += ' '+(inNamespace.isEmpty() ? QString() : inNamespace+"::")+definedClasses.last()+flag;
            output += ");\n\n";
        }
        
        void createHandler(const QString& classname)
        {
            //TODO: Should improve the memory management. Use harald's script tools.
            QString classNS;
            if(classNamespace.contains(classname) && !classNamespace[classname].isEmpty())
                classNS=classNamespace[classname]+"::";
            
            QString handlername=classname;
            handlername[0]=handlername[0].toLower();
            
            handlersHeader += "\tQVariant _"+handlername+"Handler(void* type);\n";
            handlersHeader += "\tQVariant "+handlername+"Handler("+classNS+classname+"* type) { return _"+handlername+"Handler((void*) type); }\n";
            handlersHeader += "\tQVariant "+handlername+"Handler(const "+classNS+classname+"* type) "
                                "{ return _"+handlername+"Handler((void*) type); }\n\n";
            
            output += "QVariant _"+handlername+"Handler(void* type)\n"
            "{\n"
            "\tif(!type) return QVariant();\n"
            "\t"+(classNS.isEmpty() ? QString() : (classNS))+classname+"* t=static_cast<"+classNS+classname+"*>(type);\n"
            "\treturn qVariantFromValue((QObject*) new Kross"+classname+"(t, 0));\n"
            "}\n"
            "bool b_"+classname+"="+filename+"_registerHandler(\""+classNS+classname+"*\", _"+handlername+"Handler);\n\n";
        }
        
        void writeEndDocument()
        {
            output += "namespace Handlers\n{\n";
            
            output += "\tbool "+filename+"_registerHandler(const QByteArray& name, Kross::MetaTypeHandler::FunctionPtr* handler)\n"
                      "{ Kross::Manager::self().registerMetaTypeHandler(name, handler); return false; }\n\n";
                      
            handlersHeader += "namespace Handlers\n{\n";
            foreach(const QString& aclass, definedClasses)
                createHandler(aclass);
            output += "}\n";
            handlersHeader += "}\n\n";
            output += "#include \""+filename+".moc\"\n";
            
            handlersHeader += "#endif\n";
        }
        
        void writeEndFunction(const method& m)
        {
            QString rettype=m.returnType;
            rettype=rettype.replace('&', QString());
            output += "\t\tQ_SCRIPTABLE " + rettype +' '+ m.funcname+'(';
            QStringList values;
            
            int param=0;
            foreach(const method::argument& arg, m.args)
            {
                QString varname=arg.name;
                if(varname.isEmpty()) {
                    qWarning() << "The paramenter number "+QString::number(param)+" in method: "+
                            inNamespace+"::"+definedClasses.last()+"::"+m.funcname+" does not have a name";
                    varname=QString("x%1").arg(param);
                }
                values += varname;
                output += arg.type +' '+ varname;
                if(!arg.def.isEmpty())
                    output+='='+arg.def;
                output += ", ";
                param++;
            }
            
            if(!values.isEmpty())
                output.resize(output.size()-2);
            output += ')';
            if(m.isConst)
                output+=" const";
            
            QString shouldReturn= m.returnType=="void" ? QString() : QString("return ");
            
            output += " { "+shouldReturn+"wrapped->"+m.funcname+"(";
            foreach(const QString& val, values)
            {
                output+=val+", ";
            }
            
            if(!values.isEmpty())
                output.resize(output.size()-2);
            
            output += "); }\n";
        }
};

int main(int argc, char** argv)
{
    QStringList includes;
    QString filename;
    QString directory;
    int i;
    for(i=1; i<argc; i++)
    {
        if(argv[i][0]=='-' && argv[i][1]!=0) {
            QString param=argv[i];
            switch(argv[i][1])
            {
                case 'I':
                    param=param.right(param.size()-2);
                    includes += param.split(';');
                    break;
                case 'F':
                    filename=param.right(param.size()-2);
                    break;
                case 'D':
                    directory=param.right(param.size()-2);
                    break;
            }
        }
        else
        {
            qDebug() << "error. Unrecognized parameter: " << argv[i];
            return 1;
        }
    }
    
    QFile f;
    if(!f.open(stdin, QIODevice::ReadOnly)) {
        qDebug() << "error. can't read the input: " << argv[i];
        return 33;
    }
    
    QXmlStreamReader xml(&f);
    KrossWrapper p(xml);
    p.setIncludes(includes);
    p.setFileName(filename);
    
    int ret=p.start();
    
    fprintf(stdout, "%s", qPrintable(p.output));
    
    if(!filename.isEmpty()) {
        QFile headerFile(directory+'/'+filename+".h");
        if(!headerFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "error. can't write the header: " << filename+".h";
            return 33;
        }
        
        QTextStream out(&headerFile);
        out << p.handlersHeader;
        headerFile.close();
    }
    return ret;
}
