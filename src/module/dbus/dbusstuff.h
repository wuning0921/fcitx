/***************************************************************************
 *   Copyright (C) 2010~2010 by CSSlayer                                   *
 *   wengxt@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _FCITX_DBUS_H
#define _FCITX_DBUS_H
#include <dbus/dbus.h>

#define FCITX_DBUS_NAME "fcitx-dbus"
#define FCITX_DBUS_GETCONNECTION 0
#define FCITX_DBUS_GETCONNECTION_RETURNTYPE DBusConnection*
#define FCITX_DBUS_ADDEVENTHANDLER 1
#define FCITX_DBUS_ADDEVENTHANDLER_RETURNTYPE void
#define FCITX_DBUS_REMOVEEVENTHANDLER 2
#define FCITX_DBUS_REMOVEEVENTHANDLER_RETURNTYPE void

#define FCITX_DBUS_PATH "/org/fcitx/fcitx"
#define FCITX_DBUS_SERVICE "org.fcitx.fcitx"

typedef struct FcitxDBusEventHandler {
    boolean (*eventHandler)(void* instance, DBusMessage * event);
    void* instance;
} FcitxDBusEventHandler;

#endif // _FCITX_DBUS_H