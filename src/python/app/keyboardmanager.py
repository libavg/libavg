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
# Sponsored by Archimedes Exhibitions GmbH ( http://www.archimedes-exhibitions.de )


'''
Convenient stack-based keyboard bindings handler

Minimal example:

>>> keyboardmanager.bindKeyDown(keystring='d',
>>>     handler=doSomething,
>>>     help='Do something')
'''

from collections import namedtuple

from libavg import avg, player

IGNORED_KEYMODS = avg.KEYMOD_NUM

_KeyBinding = namedtuple('_KeyBinding',
        ['keystring', 'handler', 'help', 'modifiers', 'type'])


_modifiedKeyBindings = []
_plainKeyBindings = []
_plainKeyBindingsStack = []
_isEnabled = True

def init():
    '''
    Initialize the manager, normally done by L{app.App}
    '''
    player.subscribe(player.KEY_DOWN, _onKeyDown)
    player.subscribe(player.KEY_UP, _onKeyUp)

def bindKeyDown(keystring, handler, help, modifiers=avg.KEYMOD_NONE):
    '''
    Bind a down-key action to an handler defining a mandatory help text
    '''
    _bindKey(keystring, handler, help, modifiers, avg.KEYDOWN)

def bindKeyUp(keystring, handler, help, modifiers=avg.KEYMOD_NONE):
    '''
    Bind an up-key action to an handler defining a mandatory help text
    '''
    _bindKey(keystring, handler, help, modifiers, avg.KEYUP)

def unbindKeyUp(keystring, modifiers=avg.KEYMOD_NONE):
    '''
    Unbind an up-key action that's been previously defined be bindKeyUp
    '''
    _unbindKey(keystring, modifiers, avg.KEYUP)

def unbindKeyDown(keystring, modifiers=avg.KEYMOD_NONE):
    '''
    Unbind a down-key action that's been previously defined be bindKeyDown
    '''
    _unbindKey(keystring, modifiers, avg.KEYDOWN)

def unbindAll():
    global _modifiedKeyBindings, _plainKeyBindings, _plainKeyBindingsStack
    _modifiedKeyBindings = []
    _plainKeyBindings = []
    _plainKeyBindingsStack = []

def push():
    '''
    Push the current non-modified defined key bindings to the stack
    '''
    global _plainKeyBindings
    _plainKeyBindingsStack.append(_plainKeyBindings)
    _plainKeyBindings = []

def pop():
    '''
    Pop from the stack the current non-modified defined key bindings
    '''
    global _plainKeyBindings
    _plainKeyBindings = _plainKeyBindingsStack.pop()

def getCurrentBindings():
    return _modifiedKeyBindings + _plainKeyBindings

def enable():
    '''
    Enable the bindings. Key presses and releases trigger the bound handlers
    '''
    global _isEnabled
    _isEnabled = True

def disable():
    '''
    Disabled the bindings. Key presses and releases do not trigger the bound handlers
    '''
    global _isEnabled
    _isEnabled = False

def _bindKey(keystring, handler, help, modifiers, type):
    _checkDuplicates(keystring, modifiers, type)
    keyBinding = _KeyBinding(keystring, handler, help, modifiers, type)

    if modifiers != avg.KEYMOD_NONE:
        _modifiedKeyBindings.append(keyBinding)
    else:
        _plainKeyBindings.append(keyBinding)

def _findAndRemoveKeybinding(keystring, modifiers, type, list):
    for keybinding in list:
            if keybinding.keystring == keystring and \
               keybinding.modifiers == modifiers and \
               keybinding.type == type:
                   list.remove(keybinding)
                   break;

def _unbindKey(keystring, modifiers, type):
    if modifiers != avg.KEYMOD_NONE:
        _findAndRemoveKeybinding(keystring, modifiers, type, _modifiedKeyBindings)
    else:
        _findAndRemoveKeybinding(keystring, modifiers, type, _plainKeyBindings)

def _onKeyDown(event):
    if _isEnabled:
        _processEvent(event, avg.KEYDOWN)

def _onKeyUp(event):
    if _isEnabled:
        _processEvent(event, avg.KEYUP)

def _areMatchingModifiers(mod1, mod2):
    mod1 &= ~IGNORED_KEYMODS
    mod2 &= ~IGNORED_KEYMODS
    return mod1 == mod2 or mod1 & mod2

def _processEvent(event, type):
    for keyBinding in _plainKeyBindings + _modifiedKeyBindings:
        if ((keyBinding.keystring, keyBinding.type) == (event.keystring, type) and
                _areMatchingModifiers(event.modifiers, keyBinding.modifiers)):
            keyBinding.handler()
            return

def _checkDuplicates(keystring, modifiers, type):
    for keyBinding in _plainKeyBindings + _modifiedKeyBindings:
        if ((keyBinding.keystring, keyBinding.type) == (keystring, type) and
                _areMatchingModifiers(modifiers, keyBinding.modifiers)):
            raise RuntimeError('Key binding keystring=%s modifiers=%s type=%s '
                    'already defined' % (keystring, modifiers, type))

