/**
 * Copyright 2014 Ahmed I. Khalil <ahmedibrahimkhali@gmail.com>
 * Copyright 2015 Martin Gräßlin <mgraesslin@kde.org>
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

#include "mousepadplugin.h"
#include <KPluginFactory>
#include <KLocalizedString>
#include <QDebug>
#include <QGuiApplication>
#include <QX11Info>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <fakekey/fakekey.h>

#if HAVE_WAYLAND
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/fakeinput.h>
#include <KWayland/Client/registry.h>
#endif

K_PLUGIN_FACTORY_WITH_JSON( KdeConnectPluginFactory, "kdeconnect_mousepad.json", registerPlugin< MousepadPlugin >(); )

enum MouseButtons {
    LeftMouseButton = 1,
    MiddleMouseButton = 2,
    RightMouseButton = 3,
    MouseWheelUp = 4,
    MouseWheelDown = 5
};

//Translation table to keep in sync within all the implementations
int SpecialKeysMap[] = {
    0,              // Invalid
    XK_BackSpace,   // 1
    XK_Tab,         // 2
    XK_Linefeed,    // 3
    XK_Left,        // 4
    XK_Up,          // 5
    XK_Right,       // 6
    XK_Down,        // 7
    XK_Page_Up,     // 8
    XK_Page_Down,   // 9
    XK_Home,        // 10
    XK_End,         // 11
    XK_Return,      // 12
    XK_Delete,      // 13
    XK_Escape,      // 14
    XK_Sys_Req,     // 15
    XK_Scroll_Lock, // 16
    0,              // 17
    0,              // 18
    0,              // 19
    0,              // 20
    XK_F1,          // 21
    XK_F2,          // 22
    XK_F3,          // 23
    XK_F4,          // 24
    XK_F5,          // 25
    XK_F6,          // 26
    XK_F7,          // 27
    XK_F8,          // 28
    XK_F9,          // 29
    XK_F10,         // 30
    XK_F11,         // 31
    XK_F12,         // 32
};

template <typename T, size_t N>
size_t arraySize(T(&arr)[N]) { (void)arr; return N; }

MousepadPlugin::MousepadPlugin(QObject* parent, const QVariantList& args)
    : KdeConnectPlugin(parent, args), m_fakekey(nullptr), m_x11(QX11Info::isPlatformX11())
#if HAVE_WAYLAND
    , m_waylandInput(nullptr)
    , m_waylandAuthenticationRequested(false)
#endif
{
#if HAVE_WAYLAND
    setupWaylandIntegration();
#endif
}

MousepadPlugin::~MousepadPlugin()
{
    if (m_fakekey) {
        free(m_fakekey);
        m_fakekey = nullptr;
    }
}

bool MousepadPlugin::receivePackage(const NetworkPackage& np)
{
    if (m_x11) {
        return handlePackageX11(np);
    }
#if HAVE_WAYLAND
    if (m_waylandInput) {
        if (!m_waylandAuthenticationRequested) {
            m_waylandInput->authenticate(i18n("KDE Connect"), i18n("Use your phone as a touchpad and keyboard"));
            m_waylandAuthenticationRequested = true;
        }
        handPackageWayland(np);
    }
#endif
    return false;
}

bool MousepadPlugin::handlePackageX11(const NetworkPackage &np)
{
    //qDebug() << np.serialize();

    //TODO: Split mouse/keyboard in two different plugins to avoid this big if statement

    float dx = np.get<float>("dx", 0);
    float dy = np.get<float>("dy", 0);

    bool isSingleClick = np.get<bool>("singleclick", false);
    bool isDoubleClick = np.get<bool>("doubleclick", false);
    bool isMiddleClick = np.get<bool>("middleclick", false);
    bool isRightClick = np.get<bool>("rightclick", false);
    bool isSingleHold = np.get<bool>("singlehold", false);
    bool isSingleRelease = np.get<bool>("singlerelease", false);
    bool isScroll = np.get<bool>("scroll", false);
    QString key = np.get<QString>("key", "");
    int specialKey = np.get<int>("specialKey", 0);

    if (isSingleClick || isDoubleClick || isMiddleClick || isRightClick || isSingleHold || isScroll || !key.isEmpty() || specialKey) {
        Display *display = QX11Info::display();
        if(!display) {
            return false;
        }

        if (isSingleClick) {
            XTestFakeButtonEvent(display, LeftMouseButton, True, 0);
            XTestFakeButtonEvent(display, LeftMouseButton, False, 0);
        } else if (isDoubleClick) {
            XTestFakeButtonEvent(display, LeftMouseButton, True, 0);
            XTestFakeButtonEvent(display, LeftMouseButton, False, 0);
            XTestFakeButtonEvent(display, LeftMouseButton, True, 0);
            XTestFakeButtonEvent(display, LeftMouseButton, False, 0);
        } else if (isMiddleClick) {
            XTestFakeButtonEvent(display, MiddleMouseButton, True, 0);
            XTestFakeButtonEvent(display, MiddleMouseButton, False, 0);
        } else if (isRightClick) {
            XTestFakeButtonEvent(display, RightMouseButton, True, 0);
            XTestFakeButtonEvent(display, RightMouseButton, False, 0);
        } else if (isSingleHold){
            //For drag'n drop
            XTestFakeButtonEvent(display, LeftMouseButton, True, 0);
        } else if (isSingleRelease){
            //For drag'n drop. NEVER USED (release is done by tapping, which actually triggers a isSingleClick). Kept here for future-proofnes.
            XTestFakeButtonEvent(display, LeftMouseButton, False, 0);
        } else if (isScroll) {
            if (dy < 0) {
                XTestFakeButtonEvent(display, MouseWheelDown, True, 0);
                XTestFakeButtonEvent(display, MouseWheelDown, False, 0);
            } else if (dy > 0) {
                XTestFakeButtonEvent(display, MouseWheelUp, True, 0);
                XTestFakeButtonEvent(display, MouseWheelUp, False, 0);
            }
        } else if (!key.isEmpty() || specialKey) {

            bool ctrl = np.get<bool>("ctrl", false);
            bool alt = np.get<bool>("alt", false);
            bool shift = np.get<bool>("shift", false);

            if (ctrl) XTestFakeKeyEvent (display, XKeysymToKeycode(display, XK_Control_L), True, 0);
            if (alt) XTestFakeKeyEvent (display, XKeysymToKeycode(display, XK_Alt_L), True, 0);
            if (shift) XTestFakeKeyEvent (display, XKeysymToKeycode(display, XK_Shift_L), True, 0);

            if (specialKey)
            {
                if (specialKey >= (int)arraySize(SpecialKeysMap)) {
                    qWarning() << "Unsupported special key identifier";
                    return false;
                }

                int keycode = XKeysymToKeycode(display, SpecialKeysMap[specialKey]);

                XTestFakeKeyEvent (display, keycode, True, 0);
                XTestFakeKeyEvent (display, keycode, False, 0);

            } else {

                if (!m_fakekey) {
                    m_fakekey = fakekey_init(display);
                    if (!m_fakekey) {
                        qWarning() << "Failed to initialize libfakekey";
                        return false;
                    }
                }

                //We use fakekey here instead of XTest (above) because it can handle utf characters instead of keycodes.
                for (int i=0;i<key.length();i++) {
                    QByteArray utf8 = QString(key.at(i)).toUtf8();
                    fakekey_press(m_fakekey, (const uchar*)utf8.constData(), utf8.size(), 0);
                    fakekey_release(m_fakekey);
                }
            }

            if (ctrl) XTestFakeKeyEvent (display, XKeysymToKeycode(display, XK_Control_L), False, 0);
            if (alt) XTestFakeKeyEvent (display, XKeysymToKeycode(display, XK_Alt_L), False, 0);
            if (shift) XTestFakeKeyEvent (display, XKeysymToKeycode(display, XK_Shift_L), False, 0);

        }

        XFlush(display);

    } else { //Is a mouse move event
        QPoint point = QCursor::pos();
        QCursor::setPos(point.x() + (int)dx, point.y() + (int)dy);
    }
    return true;
}

#if HAVE_WAYLAND
void MousepadPlugin::setupWaylandIntegration()
{
    if (!QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        // not wayland
        return;
    }
    using namespace KWayland::Client;
    ConnectionThread *connection = ConnectionThread::fromApplication(this);
    if (!connection) {
        // failed to get the Connection from Qt
        return;
    }
    Registry *registry = new Registry(this);
    registry->create(connection);
    connect(registry, &Registry::fakeInputAnnounced, this,
        [this, registry] (quint32 name, quint32 version) {
            m_waylandInput = registry->createFakeInput(name, version, this);
        }
    );
    registry->setup();
}

bool MousepadPlugin::handPackageWayland(const NetworkPackage &np)
{
    const float dx = np.get<float>("dx", 0);
    const float dy = np.get<float>("dy", 0);

    const bool isSingleClick = np.get<bool>("singleclick", false);
    const bool isDoubleClick = np.get<bool>("doubleclick", false);
    const bool isMiddleClick = np.get<bool>("middleclick", false);
    const bool isRightClick = np.get<bool>("rightclick", false);
    const bool isSingleHold = np.get<bool>("singlehold", false);
    const bool isSingleRelease = np.get<bool>("singlerelease", false);
    const bool isScroll = np.get<bool>("scroll", false);
    const QString key = np.get<QString>("key", "");
    const int specialKey = np.get<int>("specialKey", 0);

    if (isSingleClick || isDoubleClick || isMiddleClick || isRightClick || isSingleHold || isScroll || !key.isEmpty() || specialKey) {

        if (isSingleClick) {
            m_waylandInput->requestPointerButtonClick(Qt::LeftButton);
        } else if (isDoubleClick) {
            m_waylandInput->requestPointerButtonClick(Qt::LeftButton);
            m_waylandInput->requestPointerButtonClick(Qt::LeftButton);
        } else if (isMiddleClick) {
            m_waylandInput->requestPointerButtonClick(Qt::MiddleButton);
        } else if (isRightClick) {
            m_waylandInput->requestPointerButtonClick(Qt::RightButton);
        } else if (isSingleHold){
            //For drag'n drop
            m_waylandInput->requestPointerButtonPress(Qt::LeftButton);
        } else if (isSingleRelease){
            //For drag'n drop. NEVER USED (release is done by tapping, which actually triggers a isSingleClick). Kept here for future-proofnes.
            m_waylandInput->requestPointerButtonRelease(Qt::LeftButton);
        } else if (isScroll) {
            m_waylandInput->requestPointerAxis(Qt::Vertical, dy);
        } else if (!key.isEmpty() || specialKey) {
            // TODO: implement key support
        }

    } else { //Is a mouse move event
        m_waylandInput->requestPointerMove(QSizeF(dx, dy));
    }
    return true;
}
#endif

#include "mousepadplugin.moc"
