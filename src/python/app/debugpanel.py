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
# Original authors of this file are
#   OXullo Interecans <x at brainrapers dot org>
#   Richard Klemm <richy at coding-reality.de>
# Sponsored by Archimedes Exhibitions GmbH ( http://www.archimedes-exhibitions.de )

from collections import defaultdict
from collections import deque
from math import cos

import libavg
from libavg import avg
from libavg import graph
from libavg.widget.scrollarea import ScrollArea
from libavg.widget.button import TextButton
from libavg.apphelpers import DebugTouchVisualization
from libavg.apphelpers import TouchVisualizationOverlay as TouchVisOverlay


import keyboardmanager as kbmgr

g_fontsize = 10

PANGO_ENTITIES_MAP = {
    "&": "&amp;",
    '"': "&quot;",
    "'": "&apos;",
    ">": "&gt;",
    "<": "&lt;",
}

PADDING_LEFT = 10
PADDING_RIGHT = 5
PADDING_TOP = 2
PADDING_BOTTOM = 2


class DebugWidgetFrame(avg.DivNode):

    BORDER = 7
    REMOVE_WIDGET_FRAME = avg.Publisher.genMessageID()
    FRAME_HEIGHT_CHANGED = avg.Publisher.genMessageID()

    def __init__(self, size, widgetCls, *args, **kwargs):
        super(DebugWidgetFrame, self).__init__(size=size, *args, **kwargs)
        self.registerInstance(self, None)
        self.setup(widgetCls)
        self.subscribe(self.SIZE_CHANGED, self._onSizeChanged)
        self.size = size
        self._onSizeChanged(size)

    def setup(self, widgetCls):
        self.__background = avg.RectNode(parent=self, opacity=0.8,
                                         fillcolor='000000', fillopacity=0.8)
        self.__widget = widgetCls(parent=self,
                size=(max(0, self.width - self.BORDER * 2), 0),
                pos=(self.BORDER, self.BORDER))
        self.__selectHighlight = avg.RectNode(parent=self, color="35C0CD",
                strokewidth=self.BORDER, opacity=0.8,
                pos=(self.BORDER / 2, self.BORDER / 2), active=False, sensitive=False)
        self.__boundary = avg.RectNode(parent=self, sensitive=False)
        self.removeButton = TextButton(parent=self, size=(20, 20),
               pos=(self.width - 40, 10), text="X")

        self.publish(self.REMOVE_WIDGET_FRAME)
        self.publish(DebugWidgetFrame.FRAME_HEIGHT_CHANGED)

        self.subscribe(self.CURSOR_DOWN, self.toggleSelect)
        self.removeButton.subscribe(self.removeButton.CLICKED, self.remove)
        self.__widget.subscribe(widgetCls.WIDGET_HEIGHT_CHANGED,
                                self.adjustWidgetHeight)
        self.__widget.update()

    def _onSizeChanged(self, size):
        self.__boundary.size = size
        self.__background.size = size
        childSize = (max(0, size[0] - self.BORDER * 2), max(0, size[1] - self.BORDER * 2))
        self.__selectHighlight.size = (max(0, size[0] - self.BORDER),
                max(0, size[1] - self.BORDER))
        self.__widget.size = childSize
        self.__widget.syncSize(childSize)

    def adjustWidgetHeight(self, height):
        self.size = (max(0, self.width), height + 2 * self.BORDER)
        self.notifySubscribers(DebugWidgetFrame.FRAME_HEIGHT_CHANGED, [])

    def remove(self):
        self.notifySubscribers(self.REMOVE_WIDGET_FRAME, [self])

    def toggleSelect(self, event=None):
        self.__selectHighlight.active = not(self.__selectHighlight.active)

    def isSelected(self):
        return self.__selectHighlight.active

    def select(self):
        self.__selectHighlight.active = True

    def unselect(self):
        self.__selectHighlight.active = False

    def show(self):
        self.active = True
        self.__widget.onShow()
        self.__widget.update()

    def hide(self):
        self.active = False
        self.__widget.onHide()

    @property
    def widget(self):
        return self.__widget


class DebugWidget(avg.DivNode):
    SLOT_HEIGHT = 200

    WIDGET_HEIGHT_CHANGED = avg.Publisher.genMessageID()

    def __init__(self, parent=None, **kwargs):
        super(DebugWidget, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.publish(DebugWidget.WIDGET_HEIGHT_CHANGED)

    def syncSize(self, size):
        pass

    def update(self):
        pass

    def onShow(self):
        pass

    def onHide(self):
        pass

    def kill(self):
        pass


class ScrollableTextWidget(DebugWidget):

    def __init__(self, parent=None, **kwargs):
        super(ScrollableTextWidget, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.autoscroll = True
        self.__maxLines = 100
        self.__lines = avg.DivNode()
        self.__scrollableArea = ScrollArea(self.__lines, (self.width, self.SLOT_HEIGHT),
                                           parent=self)
        self.__scrollableArea.subscribe(self.__scrollableArea.PRESSED,
                                        self.disableAutoscroll)
        self.__scrollableArea.subscribe(self.__scrollableArea.RELEASED,
                                        self.enableAutoscroll)

    def syncSize(self, size):
        paddingSize = avg.Point2D(PADDING_LEFT + PADDING_RIGHT,
                                  PADDING_BOTTOM + PADDING_TOP)

        self.__scrollableArea.size = avg.Point2D(size) - paddingSize
        self.__scrollableArea.pos = (PADDING_LEFT, PADDING_TOP)
        self.__lines.width = size[0] - PADDING_LEFT - PADDING_RIGHT

    def disableAutoscroll(self):
        self.autoscroll = False

    def enableAutoscroll(self):
        self.autoscroll = True
        self.scrollToEnd()

    def scrollToEnd(self):
        # Early escape, there are less lines then screen estate
        if self.__scrollableArea.contentsize.y <= self.__scrollableArea.size.y:
            return
        childrenRemoved = 0
        removedHeight = 0
        while self.__lines.getNumChildren() > self.__maxLines:
            removedHeight += self.__lines.getChild(0).height
            self.__lines.removeChild(0)
            childrenRemoved += 1
            # Remove only some at a time to reduce framerate spikes when
            # releasing the mouse after reading the logs
            if childrenRemoved > 150:
                break
        if childrenRemoved > 0:
            for childID in xrange(0, self.__lines.getNumChildren()):
                child = self.__lines.getChild(childID)
                child.pos = child.pos - (0, removedHeight)
            self.__lines.height -= removedHeight

        distance = (self.__scrollableArea.contentpos.y /
                   (self.__scrollableArea.contentsize.y - self.__scrollableArea.size.y))
        slowdown = 500
        duration = (cos(distance) - cos(1.0)) * slowdown
        avg.LinearAnim(self.__scrollableArea,
                       'contentpos',
                       int(duration),
                       self.__scrollableArea.contentpos,
                       (0, self.__scrollableArea.contentsize.y -
                           self.__scrollableArea.size.y)).start()

    def appendLine(self, line, escape=True):
        if escape:
            line = ''.join(PANGO_ENTITIES_MAP.get(char, char) for char in line)

        node = avg.WordsNode(fontsize=g_fontsize, text=line, font='monospace',
                             pos=(0, self.__lines.height), rawtextmode=True)
        self.__lines.height += node.height
        if node.width > self.__lines.width:
            self.__lines.width = node.width
        self.__lines.appendChild(node)

        if self.autoscroll:
            self.scrollToEnd()

NUM_COLS = 10
COL_WIDTH = 60
ROW_HEIGHT = g_fontsize + 2


class TableRow(avg.DivNode):
    COL_POS_X = 0
    ROW_ID = 0

    def __init__(self, parent=None, **kwargs):
        super(TableRow, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        global NUM_COLS
        NUM_COLS = int((self.parent.width - COL_WIDTH * 4) / COL_WIDTH)
        self._initRow()
        TableRow.ROW_ID += 1

    def _initRow(self):
        self.columnBackground = avg.RectNode(parent=self, fillcolor="222222",
                                             fillopacity=0.6, opacity=0)
        self.columnContainer = avg.DivNode(parent=self)
        if TableRow.ROW_ID % 2 != 0:
            self.columnBackground.fillopacity = 0
        self.cols = [0] * NUM_COLS
        for i in xrange(0, NUM_COLS):
            self.cols[i] = (avg.WordsNode(parent=self.columnContainer,
                                          fontsize=g_fontsize,
                                          text="0", size=(COL_WIDTH / 2.0, ROW_HEIGHT),
                                          pos=(i * COL_WIDTH, 0)),
                            avg.WordsNode(parent=self.columnContainer,
                                          fontsize=g_fontsize,
                                          text="(0)", size=(COL_WIDTH / 2.0, ROW_HEIGHT),
                                          pos=(i * COL_WIDTH + COL_WIDTH / 2, 0),
                                          color="000000"))

        self.liveColumn = avg.WordsNode(parent=self.columnContainer, fontsize=g_fontsize,
                                        text="N/A - SPECIAL",
                                        size=(COL_WIDTH, ROW_HEIGHT), variant="bold",
                                        pos=(NUM_COLS * COL_WIDTH, 0))
        self.rowData = deque([0] * (NUM_COLS + 1), maxlen=NUM_COLS + 1)
        self.label = avg.WordsNode(parent=self, fontsize=g_fontsize, variant="bold")
        self.setLabel("NONE")

    @property
    def height(self):
        return self.label.height

    def setLabel(self, label):
        if self.label.text == label + ":":
            return
        self.label.text = label + ":"
        TableRow.COL_POS_X = max(TableRow.COL_POS_X, self.label.width)
        if self.label.width < TableRow.COL_POS_X:
            self.parent.labelColumnSizeChanged()

    def resizeLabelColumn(self):
        self.columnContainer.pos = (TableRow.COL_POS_X + 10, 0)
        self.columnBackground.size = (self.columnContainer.x + self.liveColumn.x +
                                      self.liveColumn.width, g_fontsize)

    def insertValue(self, data):
        self.rowData.append(data)
        prevValue = 0
        for i in xrange(0, len(self.rowData)):
            if i >= 0:
                data = self.rowData[i]
                column = self.cols[i - 1]
                column[0].text = str(data)
                diff = data - prevValue
                column[1].text = "(%s)" % diff
                column[1].pos = (column[0].x + column[0].getLineExtents(0)[0] + 2,
                                 column[0].y)
                if diff == 0:
                    column[1].color = "000000"
                elif diff < 0:
                    column[1].color = "00FF00"
                else:
                    column[1].color = "FF0000"
            prevValue = self.rowData[i]

    def updateLiveColumn(self, value):
        self.liveColumn.text = str(value)


class Table(avg.DivNode):
    def __init__(self, parent=None, **kwargs):
        super(Table, self).__init__(**kwargs)
        self.registerInstance(self, parent)

    def labelColumnSizeChanged(self):
        for childID in xrange(0, self.getNumChildren()):
            child = self.getChild(childID)
            child.resizeLabelColumn()


class ObjectDumpWidget(DebugWidget):
    def __init__(self, parent=None, **kwargs):
        super(ObjectDumpWidget, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.tableContainer = Table(parent=self, size=(self.width, self.SLOT_HEIGHT))
        self.tableDivs = defaultdict(lambda: TableRow(parent=self.tableContainer))

    def update(self):
        objDump = libavg.player.getTestHelper().getObjectCount()
        pos = (0, 0)
        for key in sorted(objDump.iterkeys()):
            val = objDump[key]
            self.tableDivs[key].updateLiveColumn(val)
            self.tableDivs[key].setLabel(key)
            self.tableDivs[key].pos = pos
            pos = (0, pos[1] + self.tableDivs[key].height)
        height = len(objDump) * self.tableDivs[key].height
        if self.height != height:
            self.notifySubscribers(DebugWidget.WIDGET_HEIGHT_CHANGED, [height])

    def persistColumn(self):
        objDump = libavg.player.getTestHelper().getObjectCount()
        for key, val in objDump.iteritems():
            self.tableDivs[key].insertValue(val)

    def syncSize(self, size):
        self.tableContainer.size = (size[0], size[1] - (g_fontsize + 2))

    def onShow(self):
        self.intervalID = libavg.player.setInterval(1000, self.update)
        kbmgr.bindKeyDown(keystring='i',
                                    handler=self.persistColumn,
                                    help="Persist live column to object dump table")

    def onHide(self):
        libavg.player.clearInterval(self.intervalID)
        kbmgr.unbindKeyDown(keystring='i')

    def kill(self):
        self.onHide()
        for divKey in self.tableDivs.keys():
            self.tableDivs[divKey].unlink()
            self.tableDivs[divKey] = None
        self.tableContainer.unlink()
        self.tableContainer = None


class GraphWidget(DebugWidget):
    def __init__(self, title, graphType, valueSource, **kwargs):
        super(GraphWidget, self).__init__(**kwargs)
        self.registerInstance(self, None)

        if not hasattr(graph, graphType):
            raise Exception("Unknown GraphType requested")
        else:
            self.graphClass = getattr(graph, graphType)
        if not callable(valueSource):
            raise Exception("Value source needs to be a callable")
        else:
            self.sourceCallable = valueSource

        self.title = title
        self.__graph = None

    def onShow(self):
        if self.__graph:
            self.__graph.active = True
        else:
            self.__graph = self.graphClass(title=self.title, size=self.size,
                                           getValue=self.sourceCallable, parent=self)

    def onHide(self):
        if self.__graph:
            self.__graph.active = False


class MemoryGraphWidget(GraphWidget):
    def __init__(self, **kwargs):
        super(MemoryGraphWidget, self).__init__("Memory usage", "AveragingGraph",
                                                avg.getMemoryUsage, **kwargs)
        self.registerInstance(self, None)


class FrametimeGraphWidget(GraphWidget):
    def __init__(self, **kwargs):
        super(FrametimeGraphWidget, self).__init__("Time per frame", "SlidingGraph",
                                                   libavg.player.getFrameTime, **kwargs)
        self.registerInstance(self, None)


class TickerNode(avg.DivNode):
    def __init__(self, label, content, parent=None, *args, **kwargs):
        super(TickerNode, self).__init__(*args, **kwargs)
        self.registerInstance(self, parent)
        self.label = avg.WordsNode(fontsize=g_fontsize, font='monospace', parent=self,
                                   text=label)
        self.content = avg.WordsNode(fontsize=g_fontsize, font='monospace',
                                     pos=(PADDING_LEFT, self.label.height), parent=self)
        self.updateSize()
        self.createAnim()

    def setLabel(self, label):
        self.label.text = label
        self.content.y = self.label.height

    def addContent(self, content):
        self.clearAnim()
        self.content.text += u"%s; " % content
        self.updateSize()
        self.createAnim()

    def createAnim(self):
        diff = self.content.width - self.width
        if diff > 0:
            duration = self._getDuration(diff)
            scrollForwardAnim = avg.EaseInOutAnim(self.content,
                                                  'x',
                                                  int(duration),
                                                  self.content.x,
                                                  -diff,
                                                  int(diff / 4),
                                                  int(diff / 4))
            scrollBackwardAnim = avg.EaseInOutAnim(self.content,
                                                   'x',
                                                   int(duration),
                                                   -diff,
                                                   PADDING_LEFT,
                                                   int(diff / 4),
                                                   int(diff / 4))
            pauseAnim = avg.WaitAnim(1000)

            states = []
            states.append(avg.AnimState('forward', scrollForwardAnim, 'pauseEnd'))
            states.append(avg.AnimState('backward', scrollBackwardAnim, 'pauseStart'))
            states.append(avg.AnimState('pauseEnd', pauseAnim, 'backward'))
            states.append(avg.AnimState('pauseStart', pauseAnim, 'forward'))

            self.anim = avg.StateAnim(states)
            self.anim.setState('pauseStart')
        else:
            self.anim = None

    def clearAnim(self):
        if self.anim:
            if self.anim.isRunning():
                self.anim.abort()
            del self.anim
            self.anim = None

    def updateSize(self):
        height = self.label.height + self.content.height
        self.height = height

    def _getDuration(self, dist, speed=80):
        """
        Get the duration for executing an animation at a certain speed
        @param speed: pixels/seconds
        @type speed: float
        @return duration in ms
        """
        return (dist / speed) * 1000  # 20pixel/sec


class KeyboardManagerBindingsShower(DebugWidget):
    def __init__(self, *args, **kwargs):
        super(KeyboardManagerBindingsShower, self).__init__(**kwargs)
        self.registerInstance(self, None)
        self.keybindingTickers = []

    def clear(self):
        for ticker in self.keybindingTickers:
            ticker.clearAnim()
            ticker.unlink(True)
            ticker = None
        self.keybindingTickers = []

    def update(self):
        self.clear()
        keyClasses = defaultdict(list)
        for binding in kbmgr.getCurrentBindings():
            keyClasses[binding.modifiers].append(binding)

        for modifiers, bindings in keyClasses.iteritems():
            label = 'Modifiers: %s' % (', '.join(self.__modifiersToList(modifiers)))

            ticker = TickerNode(parent=self, size=(max(0, self.width), 0), label=label,
                                content="")

            for binding in bindings:
                ticker.addContent(self.markupBinding(binding.keystring, binding.help))
            self.keybindingTickers.append(ticker)

        if len(kbmgr._plainKeyBindingsStack) > 0:
            label = "Application Bindings without modifier"
            ticker = TickerNode(parent=self, size=(max(0, self.width), 0), label=label,
                                content="")
            for binding in kbmgr._plainKeyBindingsStack[-1]:
                ticker.addContent(self.markupBinding(binding.keystring, binding.help))
            self.keybindingTickers.append(ticker)
        self._positionTickers()

    def markupBinding(self, keystring, help):
        return '<span size="large"><b>%s</b></span>: %s' % (keystring, help)

    def _positionTickers(self):
        height = 0
        for ticker in self.keybindingTickers:
            ticker.pos = (0, height)
            height += ticker.height
        if self.height != height:
            self.notifySubscribers(self.WIDGET_HEIGHT_CHANGED, [height])

    def __modifiersToList(self, modifiers):
        def isSingleBit(number):
            bitsSet = 0
            for i in xrange(8):
                if (1 << i) & number:
                    bitsSet += 1

            return bitsSet == 1

        if modifiers == 0:
            return ['NONE']

        allModifiers = []
        for mod in dir(avg):
            if 'KEYMOD_' in mod:
                maskVal = int(getattr(avg, mod))
                if isSingleBit(maskVal):
                    allModifiers.append((maskVal, mod))

        modifiersStringsList = []
        for modval, modstr in allModifiers:
            if modifiers & modval:
                modifiersStringsList.append(modstr)

        return modifiersStringsList

class DebugPanel(avg.DivNode):
    MAX_OPACITY = 0.9
    UNSENSITIVE_OPACITY = 0.6

    def __init__(self, parent=None, fontsize=10, **kwargs):
        super(DebugPanel, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.__panel = None
        self.__callables = []
        self.__fontsize = fontsize

        self.active = False

    def addWidget(self, widgetCls, *args, **kwargs):
        callable_ = lambda: self.__panel.addWidget(widgetCls, *args, **kwargs)
        if self.__panel:
            callable_()
        else:
            self.__callables.append(callable_)

    def hide(self):
        kbmgr.pop()
        self.active = False
        if self.__panel:
            self.__panel.hide()

    def show(self):
        self.active = True
        kbmgr.push()
        self._setupKeys()
        if self.__panel:
            self.__panel.show()
        else:
            self.__panel = _DebugPanel(parent=self, size=self.size,
                    fontsize=self.__fontsize)
            for callable_ in self.__callables:
                callable_()

    def toggleVisibility(self):
        if self.active:
            self.hide()
        else:
            self.show()

    def _setupKeys(self):
        kbmgr.bindKeyDown(keystring='left ctrl',
                          handler=lambda: self.__setSensitivity(True),
                          help="Set debug panel sensitive")

        kbmgr.bindKeyUp(keystring='left ctrl',
                          handler=lambda: self.__setSensitivity(False),
                          help="Set debug panel unsensitive")

    def __setSensitivity(self, sensitive):
        self.sensitive = sensitive
        if sensitive:
            self.opacity = self.MAX_OPACITY
        else:
            self.opacity = self.UNSENSITIVE_OPACITY

# TODO: better layout management
class _DebugPanel(avg.DivNode):

    def __init__(self, parent=None, fontsize=10, **kwargs):
        super(_DebugPanel, self).__init__(**kwargs)
        self.registerInstance(self, parent)

        self.__slots = []

        self.maxSize = self.size
        self.size = (self.size[0], 0)
        self.activeWidgetClasses = []
        self.__selectedWidget = None
        self.__touchVisOverlay = None

        global g_fontsize
        g_fontsize = fontsize

        self.show()

    def show(self):
        self.setupKeys()
        for widgetFrame in self.__slots:
            if widgetFrame:
                widgetFrame.show()

    def hide(self):
        for widget in self.__slots:
            if widget:
                widget.hide()

    def toggleTouchVisualization(self):
        if self.__touchVisOverlay is None:
            self.__touchVisOverlay = TouchVisOverlay(isDebug=True,
                                                     visClass=DebugTouchVisualization,
                                                     size=self.parent.size,
                                                     parent=self.parent)
        else:
            self.__touchVisOverlay.unlink(True)
            self.__touchVisOverlay = None

    def setupKeys(self):
        kbmgr.bindKeyDown(keystring='m',
                          handler=lambda: self.toggleWidget(MemoryGraphWidget),
                          help="Memory graph")

        kbmgr.bindKeyDown(keystring='f',
                          handler=lambda: self.toggleWidget(FrametimeGraphWidget),
                          help="Frametime graph")

        kbmgr.bindKeyDown(keystring='k',
                          handler=lambda: self.toggleWidget(
                                KeyboardManagerBindingsShower),
                          help="kbmgrBindings")

        kbmgr.bindKeyDown(keystring='o',
                          handler=lambda: self.toggleWidget(ObjectDumpWidget),
                          help="Object dump")

        kbmgr.bindKeyDown(keystring='d',
                          handler=self.removeSelectedWidgetFrames,
                          help="Delete widgets")

        kbmgr.bindKeyDown(keystring='e',
                          handler=self.toggleTouchVisualization,
                          help="CURSOR Visualization")

        kbmgr.bindKeyDown(keystring='down',
                          handler=self.selectNextWidget,
                          help="Select next widget")

        kbmgr.bindKeyDown(keystring='up',
                          handler=self.selectPreviousWidget,
                          help="Select previous widget")

    def toggleWidget(self, widgetClass, *args, **kwargs):
        if widgetClass in self.activeWidgetClasses:
            self.removeWidgetByClass(widgetClass)
        else:
            self.addWidget(widgetClass, *args, **kwargs)

    def addWidget(self, widgetClass, *args, **kwargs):
        if widgetClass in self.activeWidgetClasses:
            libavg.logger.warning("You can't add the same widget twice")
            return

        widgetFrame = DebugWidgetFrame((max(0, self.width), DebugWidget.SLOT_HEIGHT),
                widgetClass)
        height = 0
        for frame in self.__slots:
            if frame:
                height += frame.height
        height += widgetFrame.height

        if height > self.maxSize[1]:
            widgetFrame = None
            libavg.logger.warning("No vertical space left. "
                    "Delete a widget and try again")
            return False

        self.appendChild(widgetFrame)

        widgetPlaced = False
        for idx, slot in enumerate(self.__slots):
            if slot is None:
                self.__slots[idx] = widgetFrame
                widgetPlaced = True
                break
        if not widgetPlaced:
            self.__slots.append(widgetFrame)
        widgetFrame.subscribe(widgetFrame.REMOVE_WIDGET_FRAME, self.removeWidgetFrame)
        widgetFrame.subscribe(widgetFrame.FRAME_HEIGHT_CHANGED, self._heightChanged)

        self.reorderWidgets()
        widgetFrame.show()
        self.activeWidgetClasses.append(widgetClass)
        self.updateWidgets()

    def removeWidgetByClass(self, widgetClass):
        for frame in self.__slots[:]:
            if frame.widget.__class__ == widgetClass:
                self.removeWidgetFrame(frame)
                return

    def _heightChanged(self):
        height = 0
        for childID in xrange(0, self.getNumChildren()):
            child = self.getChild(childID)
            height += child.height
        self.height = height
        self.reorderWidgets()

    def updateWidgets(self):
        for childID in xrange(0, self.getNumChildren()):
            self.getChild(childID).widget.update()

    def selectWidget(self, id):
        id = id % self.getNumChildren()
        for childID in xrange(0, self.getNumChildren()):
            self.getChild(childID).unselect()
        self.getChild(id).select()
        self.__selectedWidget = id

    def selectPreviousWidget(self):
        if self.__selectedWidget is None:
            self.selectWidget(-1)
        else:
            self.selectWidget(self.__selectedWidget - 1)

    def selectNextWidget(self):
        if self.__selectedWidget is None:
            self.selectWidget(0)
        else:
            self.selectWidget(self.__selectedWidget + 1)

    def removeWidgetFrame(self, widgetFrame):
        widget = widgetFrame.widget
        self.activeWidgetClasses.remove(widget.__class__)
        for idx, slot in enumerate(self.__slots):
            if slot == widgetFrame:
                widgetFrame.widget.kill()
                widgetFrame.unlink(True)
                widgetFrame = None
                self.__slots[idx] = None
                break
        self.reorderWidgets()
        self.updateWidgets()

    def removeSelectedWidgetFrames(self):
        candidates = []
        for childID in xrange(0, self.getNumChildren()):
            child = self.getChild(childID)
            if child.isSelected():
                candidates.append(child)
        for widgetFrame in candidates:
            self.removeWidgetFrame(widgetFrame)
        self.__selectedWidget = None

    def reorderWidgets(self):
        #TODO: This is no layout management, yet
        count = 0
        height = 0
        for idx, widgetFrame in enumerate(self.__slots):
            if widgetFrame:
                widgetFrame.pos = (0, height)
                count += 1
                height += widgetFrame.height
        self.size = (self.maxSize[0], height)
