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

#include "globalconfigpage.h"
#include "ui_globalconfigpage.h"

#include "globalsettings.h"

namespace shellcheck
{

GlobalConfigPage::GlobalConfigPage(KDevelop::IPlugin* plugin, QWidget* parent)
    : ConfigPage(plugin, GlobalSettings::self(), parent)
{
    Ui::GlobalConfigPage ui;
    ui.setupUi(this);
}

GlobalConfigPage::~GlobalConfigPage()
{
}

KDevelop::ConfigPage::ConfigPageType GlobalConfigPage::configPageType() const
{
    return KDevelop::ConfigPage::AnalyzerConfigPage;
}

QString GlobalConfigPage::name() const
{
    return i18nc("@title:tab", "Shellcheck");
}

QString GlobalConfigPage::fullName() const
{
    return i18nc("@title:tab", "Configure Shellcheck Settings");
}

QIcon GlobalConfigPage::icon() const
{
    return QIcon::fromTheme(QStringLiteral("shellcheck"));
}

}
