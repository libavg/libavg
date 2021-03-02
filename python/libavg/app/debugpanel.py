#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2021 Ulrich von Zadow
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
#   OXullo Intersecans <x at brainrapers dot org>
#   Richard Klemm <richy at coding-reality.de>

from collections import defaultdict, deque
import math

import libavg
from libavg import avg, player, graph
from .touchvisualization import DebugTouchVisualization
from .touchvisualization import TouchVisualizationOverlay as TouchVisOverlay
from . import keyboardmanager as kbmgr

g_fontsize = 10


class DebugWidgetFrame(avg.DivNode):
    BORDER = 7
    FRAME_HEIGHT_CHANGED = avg.Publisher.genMessageID()

    def __init__(self, size, widgetCls, **kwargs):
        super().__init__(size=size)
        self.registerInstance(self, None)
        self.setup(widgetCls, **kwargs)
        self.subscribe(self.SIZE_CHANGED, self._onSizeChanged)
        self.size = size
        self._onSizeChanged(size)

    def setup(self, widgetCls, **kwargs):
        self.__background = avg.RectNode(parent=self, opacity=0.8,
                fillcolor='000000', fillopacity=0.8)
        self.__widget = widgetCls(parent=self,
                size=(max(0, self.width - self.BORDER * 2), 0),
                pos=(self.BORDER, self.BORDER), **kwargs)
        self.__boundary = avg.RectNode(parent=self, sensitive=False)

        self.publish(DebugWidgetFrame.FRAME_HEIGHT_CHANGED)

        self.__widget.subscribe(self.__widget.WIDGET_HEIGHT_CHANGED,
                self.adjustWidgetHeight)
        self.__widget.update()
        self.sensitive = self.__widget.sensitive

    def _onSizeChanged(self, size):
        self.__boundary.size = size
        self.__background.size = size
        childSize = (max(0, size[0] - self.BORDER * 2), max(0, size[1] - self.BORDER * 2))
        self.__widget.size = childSize
        self.__widget.syncSize(childSize)

    def adjustWidgetHeight(self, height):
        self.size = (max(0, self.width), height + 2 * self.BORDER)
        self.notifySubscribers(DebugWidgetFrame.FRAME_HEIGHT_CHANGED, [])

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
    CAPTION = ''

    WIDGET_HEIGHT_CHANGED = avg.Publisher.genMessageID()

    def __init__(self, parent=None, sensitive=False, **kwargs):
        super().__init__(sensitive=sensitive, **kwargs)
        self.registerInstance(self, parent)
        self.publish(DebugWidget.WIDGET_HEIGHT_CHANGED)
        if self.CAPTION:
            self._caption = avg.WordsNode(text=self.CAPTION, pivot=(0, 0),
                    opacity=0.5, fontsize=14, parent=self)
            self._caption.angle = math.pi / 2
            self._caption.pos = (self.width, 0)

    def syncSize(self, size):
        self._caption.width = size[1]

    def update(self):
        pass

    def onShow(self):
        pass

    def onHide(self):
        pass

    def kill(self):
        pass


NUM_COLS = 10
COL_WIDTH = 60
ROW_HEIGHT = g_fontsize + 2


class TableRow(avg.DivNode):
    COL_POS_X = 0
    ROW_ID = 0

    def __init__(self, parent=None, **kwargs):
        super().__init__(**kwargs)
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
        self.liveColumn = avg.WordsNode(parent=self.columnContainer, fontsize=g_fontsize,
                text="N/A - SPECIAL", size=(COL_WIDTH, ROW_HEIGHT), variant="bold")
        for i in range(0, NUM_COLS):
            self.cols[i] = (avg.WordsNode(parent=self.columnContainer,
                                          fontsize=g_fontsize,
                                          text="0", size=(COL_WIDTH / 2.0, ROW_HEIGHT),
                                          pos=((i+1) * COL_WIDTH, 0)),
                            avg.WordsNode(parent=self.columnContainer,
                                          fontsize=g_fontsize,
                                          text="(0)", size=(COL_WIDTH / 2.0, ROW_HEIGHT),
                                          pos=((i+1) * COL_WIDTH + COL_WIDTH / 2, 0),
                                          color="000000"))

        self.rowData = deque([(0, 0)] * (NUM_COLS + 1), maxlen=NUM_COLS + 1)
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
        prevValue = self.rowData[0][0]
        self.rowData.appendleft([data, data-prevValue])
        for i in range(0, len(self.rowData)-1):
            val, diff = self.rowData[i]
            column = self.cols[i]
            column[0].text = str(val)
            column[1].text = "({diff})".format(diff=diff)
            column[1].pos = (column[0].x + column[0].getLineExtents(0)[0] + 2,
                             column[0].y)
            if diff == 0:
                column[1].color = "000000"
            elif diff < 0:
                column[1].color = "00FF00"
            else:
                column[1].color = "FF0000"

    def updateLiveColumn(self, value):
        self.liveColumn.text = str(value)


class Table(avg.DivNode):
    def __init__(self, parent=None, **kwargs):
        super().__init__(**kwargs)
        self.registerInstance(self, parent)

    def labelColumnSizeChanged(self):
        for childID in range(0, self.getNumChildren()):
            child = self.getChild(childID)
            child.resizeLabelColumn()


class ObjectDumpWidget(DebugWidget):
    CAPTION = 'Objects count'

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.tableContainer = Table(parent=self, size=(self.width, self.SLOT_HEIGHT))
        self.tableDivs = defaultdict(lambda: TableRow(parent=self.tableContainer))

    def update(self):
        objDump = player.getTestHelper().getObjectCount()
        pos = (0, 0)
        for key in sorted(objDump.keys()):
            val = objDump[key]
            self.tableDivs[key].updateLiveColumn(val)
            self.tableDivs[key].setLabel(key)
            self.tableDivs[key].pos = pos
            pos = (0, pos[1] + self.tableDivs[key].height)
        height = len(objDump) * self.tableDivs[key].height
        if self.height != height:
            self.notifySubscribers(DebugWidget.WIDGET_HEIGHT_CHANGED, [height])

    def persistColumn(self):
        objDump = player.getTestHelper().getObjectCount()
        for key, val in objDump.items():
            self.tableDivs[key].insertValue(val)

    def syncSize(self, size):
        self.tableContainer.size = (size[0], size[1] - (g_fontsize + 2))

    def onShow(self):
        self.intervalID = player.setInterval(1000, self.update)
        kbmgr.bindKeyDown(
                keyname='I',
                handler=self.persistColumn,
                help="Take objects count snapshot",
                modifiers=avg.KEYMOD_CTRL)

    def onHide(self):
        if self.intervalID:
            player.clearInterval(self.intervalID)
            self.intervalID = None
        kbmgr.unbindKeyDown(keyname='I', modifiers=avg.KEYMOD_CTRL)

    def kill(self):
        self.onHide()
        self.tableDivs = None


class GraphWidget(DebugWidget):
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.__graph = None

    def onShow(self):
        if self.__graph:
            self.__graph.active = True
        else:
            self.__graph = self._createGraph()

    def onHide(self):
        if self.__graph:
            self.__graph.active = False

    def kill(self):
        self.__graph.unlink(True)

    def _createGraph(self):
        pass


class MemoryGraphWidget(GraphWidget):
    CAPTION = 'Memory usage'

    def _createGraph(self):
        return graph.AveragingGraph(parent=self, size=self.size,
                getValue=player.getMemoryUsage)


class FrametimeGraphWidget(GraphWidget):
    CAPTION = 'Time per frame'

    def _createGraph(self):
         return graph.SlidingBinnedGraph(parent=self,
                 getValue=player.getFrameTime,
                 binsThresholds=[0.0, 20.0, 40.0, 80.0, 160.0],
                 size=self.size)


class GPUMemoryGraphWidget(GraphWidget):
    CAPTION = 'GPU Memory usage'

    def _createGraph(self):
        try:
            player.getVideoMemUsed()
        except avg.Exception:
            return avg.WordsNode(parent=self,
                    text='GPU memory graph is not supported on this hardware',
                    color='ff5555')
        else:
            return graph.AveragingGraph(parent=self, size=self.size,
                    getValue=player.getVideoMemUsed)


class KeyboardManagerBindingsShower(DebugWidget):
    CAPTION = 'Keyboard bindings'

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.keybindingWordNodes = []
        kbmgr.publisher.subscribe(kbmgr.publisher.BINDINGS_UPDATED, self.update)

    def clear(self):
        for node in self.keybindingWordNodes:
            node.unlink(True)
        self.keybindingWordNodes = []

    def update(self):
        self.clear()
        for binding in kbmgr.getCurrentBindings():
            if binding.keyname:
                keystring = binding.keyname
            else:
                keystring = binding.text
            modifiersStr = self.__modifiersToString(binding.modifiers)

            if modifiersStr is not None:
                key = '%s-%s' % (modifiersStr, keystring)
            else:
                key = keystring

            if binding.type == avg.Event.KEY_DOWN:
                key = '%s %s' % (chr(8595), key)
            else:
                key = '%s %s' % (chr(8593), key)

            node = avg.WordsNode(
                    text='<span size="large"><b>%s</b></span>: %s' % (key, binding.help),
                    fontsize=g_fontsize, parent=self)
            self.keybindingWordNodes.append(node)

        self._placeNodes()

    def kill(self):
        kbmgr.publisher.unsubscribe(kbmgr.publisher.BINDINGS_UPDATED, self.update)

    def _placeNodes(self):
        if not self.keybindingWordNodes:
            return

        maxWidth = max([node.width for node in self.keybindingWordNodes])
        columns = int(self.parent.width / maxWidth)
        rows = len(self.keybindingWordNodes) / columns
        remainder = len(self.keybindingWordNodes) % columns

        if remainder != 0:
            rows += 1

        colSize = self.parent.width / columns

        currentColumn = 0
        currentRow = 0
        heights = [0] * columns
        for node in self.keybindingWordNodes:
            if currentRow == rows and currentColumn < columns - 1:
                currentRow = 0
                currentColumn += 1

            node.pos = (currentColumn * colSize, heights[currentColumn])
            heights[currentColumn] += node.height
            currentRow += 1

        finalHeight = max(heights)
        if self.height != finalHeight:
            self.notifySubscribers(self.WIDGET_HEIGHT_CHANGED, [finalHeight])

    def __modifiersToString(self, modifiers):
        def isSingleBit(number):
            bitsSet = 0
            for i in range(8):
                if (1 << i) & number:
                    bitsSet += 1

            return bitsSet == 1

        if modifiers in (0, kbmgr.KEYMOD_ANY):
            return None

        allModifiers = []
        for mod in dir(avg):
            if 'KEYMOD_' in mod:
                maskVal = int(getattr(avg, mod))
                if isSingleBit(maskVal):
                    allModifiers.append((maskVal, mod))

        modifiersStringsList = []
        for modval, modstr in allModifiers:
            if modifiers & modval:
                modifiersStringsList.append(modstr.replace('KEYMOD_', ''))

        for doubleMod in ['CTRL', 'META', 'SHIFT']:
            left = 'L' + doubleMod
            right = 'R' + doubleMod
            if left in modifiersStringsList and right in modifiersStringsList:
                modifiersStringsList.remove(left)
                modifiersStringsList.remove(right)
                modifiersStringsList.append(doubleMod)

        return '/'.join(modifiersStringsList).lower()


class DebugPanel(avg.DivNode):
    def __init__(self, parent=None, fontsize=10, **kwargs):
        self.__size = kwargs.pop('size')
        super().__init__(**kwargs)
        self.registerInstance(self, parent)

        avg.RectNode(size=self.__size, opacity=0, fillopacity=0.3, fillcolor='ff0000',
                sensitive=False, parent=self)
        avg.WordsNode(text='Debug panel', fontsize=fontsize,
                pos=(0, self.height - fontsize - fontsize / 3),
                sensitive=False, parent=self)

        self.active = False
        self.__panel = None
        self.__callables = []
        self.__fontsize = fontsize
        self.__touchVisOverlay = None

    def setupKeys(self):
        kbmgr.bindKeyDown(keyname='G',
                handler=lambda: self.toggleWidget(GPUMemoryGraphWidget),
                help='Show/hide GPU memory graph',
                modifiers=avg.KEYMOD_CTRL)

        kbmgr.bindKeyDown(keyname='M',
                handler=lambda: self.toggleWidget(MemoryGraphWidget),
                help='Show/hide memory graph',
                modifiers=avg.KEYMOD_CTRL)

        kbmgr.bindKeyDown(keyname='F',
                handler=lambda: self.toggleWidget(FrametimeGraphWidget),
                help='Show/hide frametime graph',
                modifiers=avg.KEYMOD_CTRL)

        kbmgr.bindKeyDown(text='?',
                handler=lambda: self.toggleWidget(KeyboardManagerBindingsShower),
                help='Show/hide keyboard bindings',
                modifiers=kbmgr.KEYMOD_ANY)

        kbmgr.bindKeyDown(keyname='O',
                handler=lambda: self.toggleWidget(ObjectDumpWidget),
                help='Show/hide objects count table',
                modifiers=avg.KEYMOD_CTRL)

        kbmgr.bindKeyDown(keyname='V',
                handler=self.toggleTouchVisualization,
                help='Toggle contact visualization',
                modifiers=avg.KEYMOD_CTRL)

    def addWidget(self, widgetClass, **kwargs):
        callable_ = lambda: self.__panel.addWidget(widgetClass, **kwargs)
        if self.__panel:
            callable_()
        else:
            self.__callables.append(callable_)

    def toggleWidget(self, widgetClass, **kwargs):
        if not self.active:
            self.show()
            self.__panel.ensureWidgetVisible(widgetClass, **kwargs)
        else:
            self.__panel.toggleWidget(widgetClass, **kwargs)

        if not self.__panel.activeWidgetClasses:
            self.hide()

    def hide(self):
        if self.__panel and self.active:
            self.__panel.hide()
            self.active = False

    def show(self):
        if self.__panel:
            if not self.active:
                self.__panel.show()
        else:
            self.forceLoadPanel()

        self.active = True

    def toggleVisibility(self):
        if self.active:
            self.hide()
        else:
            self.show()

    def toggleTouchVisualization(self):
        if self.__touchVisOverlay is None:
            self.__touchVisOverlay = TouchVisOverlay(
                    isDebug=True,
                    visClass=DebugTouchVisualization,
                    size=self.parent.size,
                    parent=self.parent)
        else:
            self.__touchVisOverlay.unlink(True)
            self.__touchVisOverlay = None

    def forceLoadPanel(self):
        if self.__panel is None:
            self.__panel = _DebugPanel(parent=self, size=self.__size,
                    fontsize=self.__fontsize)
            for callable_ in self.__callables:
                callable_()


class _DebugPanel(avg.DivNode):
    def __init__(self, parent=None, fontsize=10, **kwargs):
        self.__size = kwargs.pop('size')
        super().__init__(**kwargs)
        self.registerInstance(self, parent)

        self.__slots = []

        self.maxSize = self.__size
        self.__size = (self.__size[0], 0)
        self.activeWidgetClasses = []

        global g_fontsize
        g_fontsize = fontsize

        self.show()

    def show(self):
        for widgetFrame in self.__slots:
            if widgetFrame:
                widgetFrame.show()
        self.updateWidgets()

    def hide(self):
        for widget in self.__slots:
            if widget:
                widget.hide()

    def ensureWidgetVisible(self, widgetClass, **kwargs):
        if widgetClass not in self.activeWidgetClasses:
            self.toggleWidget(widgetClass, **kwargs)

    def toggleWidget(self, widgetClass, **kwargs):
        if widgetClass in self.activeWidgetClasses:
            self._removeWidgetByClass(widgetClass)
        else:
            self.addWidget(widgetClass, **kwargs)

    def addWidget(self, widgetClass, **kwargs):
        if widgetClass in self.activeWidgetClasses:
            libavg.logger.warning("You can't add the same widget twice")
            return

        widgetFrame = DebugWidgetFrame((max(0, self.__size[0]), DebugWidget.SLOT_HEIGHT),
                widgetClass, **kwargs)
        height = 0
        for frame in self.__slots:
            if frame:
                height += frame.height
        height += widgetFrame.height

        if height > self.maxSize[1]:
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
        widgetFrame.subscribe(widgetFrame.FRAME_HEIGHT_CHANGED, self._heightChanged)

        self.reorderWidgets()
        widgetFrame.show()
        self.updateWidgets()
        self.activeWidgetClasses.append(widgetClass)

    def _removeWidgetByClass(self, widgetClass):
        for frame in self.__slots:
            if frame and frame.widget.__class__ == widgetClass:
                self.removeWidgetFrame(frame)
                return

    def _heightChanged(self):
        height = 0
        for childID in range(0, self.getNumChildren()):
            child = self.getChild(childID)
            height += child.height
        self.__size = (self.__size[0], height)
        self.reorderWidgets()

    def updateWidgets(self):
        for childID in range(0, self.getNumChildren()):
            self.getChild(childID).widget.update()

    def removeWidgetFrame(self, widgetFrame):
        self.activeWidgetClasses.remove(widgetFrame.widget.__class__)
        for idx, slot in enumerate(self.__slots):
            if slot == widgetFrame:
                self.__slots[idx] = None
                break
        widgetFrame.widget.kill()
        widgetFrame.unlink(True)
        self.reorderWidgets()
        self.updateWidgets()

    def reorderWidgets(self):
        # TODO: This is no layout management, yet
        count = 0
        height = 0
        for idx, widgetFrame in enumerate(self.__slots):
            if widgetFrame:
                widgetFrame.pos = (0, height)
                count += 1
                height += widgetFrame.height
        self.__size = (self.maxSize[0], height)
