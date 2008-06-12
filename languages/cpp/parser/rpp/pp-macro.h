/*
  Copyright 2005 Roberto Raggi <roberto@kdevelop.org>

  Permission to use, copy, modify, distribute, and sell this software and its
  documentation for any purpose is hereby granted without fee, provided that
  the above copyright notice appear in all copies and that both that
  copyright notice and this permission notice appear in supporting
  documentation.

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
  KDEVELOP TEAM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
  AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef PP_MACRO_H
#define PP_MACRO_H

//krazy:excludeall=inline

#include <QtCore/QStringList>
#include <QVector>
#include <cppparserexport.h>
#include <indexedstring.h>
#include <hashedstring.h>

namespace rpp {

  //This contains the data of a macro that can be marshalled by directly copying the memory
struct KDEVCPPRPP_EXPORT pp_macro_direct_data
{
  pp_macro_direct_data(const KDevelop::IndexedString& nm = KDevelop::IndexedString());
  typedef uint HashType;

  KDevelop::IndexedString name;
  KDevelop::IndexedString file;
  
  int sourceLine; //line

  bool defined: 1; // !isUndefMacro
  bool hidden: 1;
  bool function_like: 1; // hasArguments
  bool variadics: 1;
  
  //The valueHash is not necessarily valid
  mutable HashType m_valueHash; //Hash that represents the values of all macros
}; ///@todo enable structure packing
  
class KDEVCPPRPP_EXPORT pp_macro : public pp_macro_direct_data
{
public:
  pp_macro();
  pp_macro(const KDevelop::IndexedString& name);
  pp_macro(const char* name);

  QVector<uint> definition; //Indices in the string index(essentially the same thing as IndexedString)
  QVector<uint> formals; // argumentList, also indices in the string repository(IndexedString)

  bool operator == ( const pp_macro& rhs ) const;

  QString toString() const;
  
  bool isUndef() const  {
    return !defined;
  }

  HashType idHash() const {
    return name.hash();
  }
  HashType valueHash() const {
    if( !m_valueHashValid ) computeHash();
    return m_valueHash;
  }

  ///Hash that identifies all of this macro, the value and the identity
  HashType completeHash() const {
    return valueHash() + idHash() * 3777;
  }

  void invalidateHash();

  struct NameCompare {
    bool operator () ( const pp_macro& lhs, const pp_macro& rhs ) const {
      return lhs.name.index() < rhs.name.index();
    }
    #ifdef Q_CC_MSVC
    
    HashType operator () ( const pp_macro& macro ) const
    {
        return macro.idHash();
    }
    
    enum
		{	// parameters for hash table
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N
    #endif
  };

  //Hash over id and value
  struct CompleteHash {
    HashType operator () ( const pp_macro& lhs ) const {
        return lhs.completeHash();
    }
    
    #ifdef Q_CC_MSVC
    
    bool operator () ( const pp_macro& lhs, const pp_macro& rhs ) const {
        HashType lhash = lhs.valueHash()+lhs.idHash();
        HashType rhash = rhs.valueHash()+rhs.idHash();
        if( lhash < rhash ) return true;
        else if( lhash > rhash ) return false;

      int df = lhs.name.str().compare( rhs.name.str() );
      return df < 0;
    }
    
    enum
		{	// parameters for hash table
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N
    #endif
  };
  
  private:
    void computeHash() const;
    mutable bool m_valueHashValid;
};

inline bool pp_macro::operator == ( const pp_macro& rhs ) const {
  computeHash();
  rhs.computeHash();

  ///@todo think about maybe doing more exact comparison when hashes are equal
  return name.index() == rhs.name.index() && m_valueHash == rhs.m_valueHash;
}

}

inline uint qHash( const rpp::pp_macro& m ) {
  return (uint)m.idHash();
}

#endif // PP_MACRO_H

