#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2014 Ulrich von Zadow
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Current versions can be found at www.libavg.de
#
# Original author of this file is OXullo Interecans <x at brainrapers dot org>


from collections import namedtuple

from libavg import avg, player

IGNORED_KEYMODS = avg.KEYMOD_NUM
KEYMOD_ANY = -1

LOGCAT = avg.logger.configureCategory('KEYBOARDMANAGER',
        avg.logger.Severity.WARN)

class KeyboardManagerPublisher(avg.Publisher):
    BINDINGS_UPDATED = avg.Publisher.genMessageID()
    def __init__(self):
        super(KeyboardManagerPublisher, self).__init__()
        self.publish(self.BINDINGS_UPDATED)

    def notifyUpdate(self):
        self.notifySubscribers(self.BINDINGS_UPDATED, [])

publisher = KeyboardManagerPublisher()

_KeyBinding = namedtuple('_KeyBinding',
        ['scancode', 'keyname', 'text', 'handler', 'help', 'modifiers', 'type'])


_keyBindings = []
_keyBindingsStack = []
_isEnabled = True


def init():
    player.subscribe(player.KEY_DOWN, _onKey)
    player.subscribe(player.KEY_UP, _onKey)
    avg.logger.debug('Keyboardmanager initialized', LOGCAT)

def bindKeyDown(scancode=None, keyname=None, text=None, handler=None, help=None,
        modifiers=avg.KEYMOD_NONE):
    _bindKey(scancode, keyname, text, handler, help, modifiers, avg.KEYDOWN)

def bindKeyUp(scancode=None, keyname=None, handler=None, help=None,
        modifiers=avg.KEYMOD_NONE):
    _bindKey(scancode, keyname, None, handler, help, modifiers, avg.KEYUP)

def unbindKeyDown(scancode=None, keyname=None, text=None, modifiers=avg.KEYMOD_NONE):
    _unbindKey(scancode, keyname, text, modifiers, avg.KEYDOWN)

def unbindKeyUp(scancode=None, keyname=None, modifiers=avg.KEYMOD_NONE):
    _unbindKey(scancode, keyname, None, modifiers, avg.KEYUP)

def unbindAll():
    global _keyBindings, _keyBindingsStack
    _keyBindings = []
    _keyBindingsStack = []
    publisher.notifyUpdate()

def push():
    """
    Push the current non-modified defined key bindings to the stack
    """
    global _keyBindings, _keyBindingsStack
    _keyBindingsStack.append(_keyBindings)
    _keyBindings = []
    publisher.notifyUpdate()

def pop():
    """
    Pop from the stack the current non-modified defined key bindings
    """
    global _keyBindings, _keyBindingsStack
    _keyBindings = _keyBindingsStack.pop()
    publisher.notifyUpdate()

def getCurrentBindings():
    return _keyBindings

def enable():
    global _isEnabled
    _isEnabled = True

def disable():
    global _isEnabled
    _isEnabled = False

def _bindKey(scancode, keyname, text, handler, help, modifiers, type_):
    err = False
    if scancode is not None:
        if keyname is not None or text is not None:
            err = True
    if keyname is not None:
        if scancode is not None or text is not None:
            err = True
    if text is not None:
        if scancode is not None or keyname is not None:
            err = True
    if err:
        raise avg.Exception(
                "Binding a key requires exactly one of scancode, keyname or text.")
    keyBinding = _KeyBinding(scancode, keyname, text, handler, help, modifiers, type_)
    _checkDuplicates(keyBinding)

    _keyBindings.append(keyBinding)
    publisher.notifyUpdate()

def _unbindKey(scancode, keyname, text, modifiers, type_):
    for keyBinding in _keyBindings:
        if (keyBinding.scancode == scancode and keyBinding.keyname == keyname and
                keyBinding.text == text and keyBinding.modifiers == modifiers and
                keyBinding.type == type_):
            _keyBindings.remove(keyBinding)
            break

    publisher.notifyUpdate()

def _onKey(event):
    if _isEnabled:
        for keyBinding in _keyBindings:
            if _testMatchEvent(keyBinding, event):
                keyBinding.handler()
                return

def _testMatchEvent(keyBinding, event):
    if event.type != keyBinding.type:
        return False
    if not _testModifiers(event.modifiers, keyBinding.modifiers):
        return False
    if (keyBinding.keyname is not None and
            _testMatchKeyName(keyBinding.keyname, event.keyname)):
        return True
    return keyBinding.scancode == event.scancode or keyBinding.text == event.text

def _testModifiers(mod1, mod2):
    if mod1 == KEYMOD_ANY or mod2 == KEYMOD_ANY:
        return True
    mod1 &= ~IGNORED_KEYMODS
    mod2 &= ~IGNORED_KEYMODS
    return mod1 == mod2 or mod1 & mod2

def _checkDuplicates(keyBinding):
    for oldBinding in _keyBindings:
        if (oldBinding.type == keyBinding.type and
                oldBinding.scancode == keyBinding.scancode and
                oldBinding.keyname == keyBinding.keyname and
                oldBinding.text == keyBinding.text and
                _testModifiers(oldBinding.modifiers, keyBinding.modifiers)):
            raise avg.Exception('Key binding scancode=%s keyname=%s text=%s modifiers=%s'
                    ' already defined' % (keyBinding.scancode, keyBinding.keyname,
                    keyBinding.text, keyBinding.modifiers))

def _testMatchKeyName(bindingName, keyPressedName):
    if bindingName == keyPressedName:
        return True

    for baseName in ("Shift", "Ctrl", "Option", "Command"):
        if (bindingName == baseName and
                (keyPressedName == "Left "+baseName or
                 keyPressedName == "Right "+baseName)):
            return True

    return False
