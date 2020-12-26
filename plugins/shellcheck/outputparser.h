/*
 * This file is part of KDevelop
 * Copyright 2020  Morten Danielsen Volden <mvolden2@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef OUTPUTPARSER_H
#define OUTPUTPARSER_H

#include <interfaces/iproblem.h>

#include <QVector>
#include <QStringList>

namespace shellcheck 
{

class Fixit;

/**
 * Parses the (Json) output from shellcheck and converts it into 
 * Problems that can be displayed in KDevelop's problem view 
 */
class OutputParser
{
public:
    OutputParser();
    ~OutputParser();
    
    QVector<KDevelop::IProblem::Ptr> parse();
    
    void add(QStringList const& lines);
    void clear();

    
private:
    KDevelop::IProblem::Severity getSeverityFromString(QString const& str);

    /**
     * Parse the json and return a fixit if the Json only has one replacement
     */
    Fixit getFixFromOneReplacement(QJsonArray replacementArray, KDevelop::DocumentRange problemRange, QString problemDescription);

    /**
     * Parse the json and return a fixit if the Json multiple replacements
     */
    Fixit getFixFromMultipleReplacements(QJsonArray replacementArray, KDevelop::DocumentRange problemRange, QString problemDescription);

    QByteArray m_shellcheckoutput;
};

}
#endif // OUTPUTPARSER_H
