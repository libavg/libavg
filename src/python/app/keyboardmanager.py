#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2013 Ulrich von Zadow
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

# XXX: logging2 temporary revert
# LOGCAT = avg.logger.configureCategory('KEYBOARDMANAGER',
#         avg.logger.Severity.WARNING)
LOGCAT = avg.logger.registerCategory('KBMGR')

class KeyboardManagerPublisher(avg.Publisher):
    BINDINGS_UPDATED = avg.Publisher.genMessageID()
    def __init__(self):
        super(KeyboardManagerPublisher, self).__init__()
        self.publish(self.BINDINGS_UPDATED)

    def notifyUpdate(self):
        self.notifySubscribers(self.BINDINGS_UPDATED, [])

publisher = KeyboardManagerPublisher()

_KeyBinding = namedtuple('_KeyBinding',
        ['keystring', 'handler', 'help', 'modifiers', 'type'])


_modifiedKeyBindings = []
_plainKeyBindings = []
_plainKeyBindingsStack = []
_isEnabled = True


def init():
    player.subscribe(player.KEY_DOWN, _onKeyDown)
    player.subscribe(player.KEY_UP, _onKeyUp)
    avg.logger.debug('Keyboardmanager initialized', LOGCAT)

def bindKeyDown(keystring, handler, help, modifiers=avg.KEYMOD_NONE):
    _bindKey(keystring, handler, help, modifiers, avg.KEYDOWN)

def bindKeyUp(keystring, handler, help, modifiers=avg.KEYMOD_NONE):
    _bindKey(keystring, handler, help, modifiers, avg.KEYUP)

def unbindKeyUp(keystring, modifiers=avg.KEYMOD_NONE):
    _unbindKey(keystring, modifiers, avg.KEYUP)

def unbindKeyDown(keystring, modifiers=avg.KEYMOD_NONE):
    _unbindKey(keystring, modifiers, avg.KEYDOWN)

def unbindAll():
    global _modifiedKeyBindings, _plainKeyBindings, _plainKeyBindingsStack
    _modifiedKeyBindings = []
    _plainKeyBindings = []
    _plainKeyBindingsStack = []
    publisher.notifyUpdate()

def push():
    '''
    Push the current non-modified defined key bindings to the stack
    '''
    global _plainKeyBindings
    _plainKeyBindingsStack.append(_plainKeyBindings)
    _plainKeyBindings = []
    publisher.notifyUpdate()

def pop():
    '''
    Pop from the stack the current non-modified defined key bindings
    '''
    global _plainKeyBindings
    _plainKeyBindings = _plainKeyBindingsStack.pop()
    publisher.notifyUpdate()

def getCurrentBindings():
    return _modifiedKeyBindings + _plainKeyBindings

def enable():
    global _isEnabled
    _isEnabled = True

def disable():
    global _isEnabled
    _isEnabled = False

def _bindKey(keystring, handler, help, modifiers, type_):
    if type(keystring) == unicode:
        keystring = keystring.encode('utf8')

    avg.logger.info('Binding key <%s> (mod:%s) to handler %s (%s)' % (keystring,
            modifiers, handler, type), LOGCAT)
    _checkDuplicates(keystring, modifiers, type_)
    keyBinding = _KeyBinding(keystring, handler, help, modifiers, type_)

    if modifiers != avg.KEYMOD_NONE:
        _modifiedKeyBindings.append(keyBinding)
    else:
        _plainKeyBindings.append(keyBinding)

    publisher.notifyUpdate()

def _findAndRemoveKeybinding(keystring, modifiers, type, list):
    for keybinding in list:
            if keybinding.keystring == keystring and \
               keybinding.modifiers == modifiers and \
               keybinding.type == type:
                   list.remove(keybinding)
                   break;

def _unbindKey(keystring, modifiers, type_):
    if type(keystring) == unicode:
        keystring = keystring.encode('utf8')

    avg.logger.info('Unbinding key <%s> (mod:%s) (%s)' % (keystring,
            modifiers, type), LOGCAT)
    if modifiers != avg.KEYMOD_NONE:
        _findAndRemoveKeybinding(keystring, modifiers, type_, _modifiedKeyBindings)
    else:
        _findAndRemoveKeybinding(keystring, modifiers, type_, _plainKeyBindings)

    publisher.notifyUpdate()

def _onKeyDown(event):
    if _isEnabled:
        _processEvent(event, avg.KEYDOWN)

def _onKeyUp(event):
    if _isEnabled:
        _processEvent(event, avg.KEYUP)

def _testModifiers(mod1, mod2):
    if mod1 == KEYMOD_ANY or mod2 == KEYMOD_ANY:
        return True

    mod1 &= ~IGNORED_KEYMODS
    mod2 &= ~IGNORED_KEYMODS
    return mod1 == mod2 or mod1 & mod2

def _testPatternMatch(pattern, text):
    if pattern in ('shift', 'alt', 'ctrl', 'meta', 'super'):
        return pattern in text
    else:
        return False

def _testMatchString(keyBinding, keyString, type_):
    sameType = keyBinding.type == type_
    patternMatch = _testPatternMatch(keyBinding.keystring, keyString)
    directMatch = keyBinding.keystring == keyString

    return sameType and (directMatch or patternMatch)

def _testMatchEvent(keyBinding, event, type_):
    if not _testModifiers(event.modifiers, keyBinding.modifiers):
        return False

    if _testMatchString(keyBinding, event.keystring, type_):
        return True

    if type_ == avg.KEYDOWN:
        return _testMatchString(keyBinding,
                unichr(event.unicode).encode('utf8'), type_)
    else:
        return False

def _processEvent(event, type_):
    avg.logger.debug('Processing event keystring=%s '
            'modifiers=%s type=%s' % (event.keystring, event.modifiers, event.type),
            LOGCAT)
    for keyBinding in _plainKeyBindings + _modifiedKeyBindings:
        if _testMatchEvent(keyBinding, event, type_):
            avg.logger.debug('  Found keyBinding=%s' % (keyBinding,), LOGCAT)
            keyBinding.handler()
            return

def _checkDuplicates(keystring, modifiers, type_):
    for keyBinding in _plainKeyBindings + _modifiedKeyBindings:
        if (_testModifiers(keyBinding.modifiers, modifiers) and
                _testMatchString(keyBinding, keystring, type_)):
            raise RuntimeError('Key binding keystring=%s modifiers=%s type=%s '
                    'already defined' % (keystring, modifiers, type_))

