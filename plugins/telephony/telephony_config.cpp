/**
 * Copyright 2013 Albert Vaca <albertvaka@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "telephony_config.h"
#include "ui_telephony_config.h"

#include <KPluginFactory>

K_PLUGIN_FACTORY(TelephonyConfigFactory, registerPlugin<TelephonyConfig>();)

TelephonyConfig::TelephonyConfig(QWidget *parent, const QVariantList& args)
    : KdeConnectPluginKcm(parent, args, "kdeconnect_telephony_config")
    , m_ui(new Ui::TelephonyConfigUi())
{
    m_ui->setupUi(this);

    connect(m_ui->show_name, SIGNAL(toggled(bool)), this, SLOT(changed()));
    connect(m_ui->show_number, SIGNAL(toggled(bool)), this, SLOT(changed()));
}

TelephonyConfig::~TelephonyConfig()
{
    delete m_ui;
}

void TelephonyConfig::defaults()
{
    KCModule::defaults();
    m_ui->show_name->setChecked(true);
    m_ui->show_number->setChecked(true);
    Q_EMIT changed(true);
}

void TelephonyConfig::load()
{
    KCModule::load();
    bool name = config()->get("showName", true);
    bool number = config()->get("showNumber", false);
    m_ui->show_name->setChecked(name);
    m_ui->show_number->setChecked(number);
    Q_EMIT changed(false);
}

void TelephonyConfig::save()
{
    config()->set("showName", m_ui->show_name->isChecked());
    config()->set("showNumber", m_ui->show_number->isChecked());
    KCModule::save();
    Q_EMIT changed(false);
}

#include "telephony_config.moc"
