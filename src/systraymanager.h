/*!
 * This file is part of WatchFlower.
 * COPYRIGHT (C) 2018 Emeric Grange - All Rights Reserved
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \date      2018
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef SYSTRAY_MANAGER_H
#define SYSTRAY_MANAGER_H

#include <QSystemTrayIcon>

class QMenu;
class QAction;
class QApplication;
class QQuickView;

/*!
 * \brief The SystrayManager class
 */
class SystrayManager: public QObject
{
    Q_OBJECT

    QQuickView *m_saved_view = nullptr;
    QApplication *m_saved_app = nullptr;

    QSystemTrayIcon *m_sysTray = nullptr;
    QMenu *m_sysTrayMenu = nullptr;
    QAction *m_actionShow = nullptr;
    QAction *m_actionSettings = nullptr;
    QAction *m_actionExit = nullptr;

    static SystrayManager *instance;

    SystrayManager();
    ~SystrayManager();

public:
    static SystrayManager *getInstance();

public slots:
    void initSystray(QApplication *app, QQuickView *view);
    bool installSystray();
    void removeSystray();
    bool sendNotification(QString &text);
    void showHide(QSystemTrayIcon::ActivationReason r);
};

#endif // SYSTRAY_MANAGER_H
