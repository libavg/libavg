#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2003-2012 Ulrich von Zadow
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
# Original author of this file: Thomas Schott <scotty@c-base.org>
#

# TODO: some info foo here:
# * c't 09/2011 p.184, media sources/license
# * description, link to wiki tutorial page

from libavg import avg
from libavg.gameapp import GameApp
from libavg.utils import getMediaDir

g_player = avg.Player.get()


### game elements ###

class Bullet(avg.VideoNode):
    def __init__(self, **kwargs):
        super(Bullet, self).__init__(href='bullet.mov', loop=True, **kwargs)
        self.play()


class Aircraft(avg.DivNode):
    __SHADOW_SCALE = 0.5
    __SHADOW_OFFSET = avg.Point2D(170, 170)

    def __init__(self, mediabase, shadowdiv, **kwargs):
        super(Aircraft, self).__init__(**kwargs)
        self.__aircraftVid = avg.VideoNode(href=mediabase+'.mov', loop=True, parent=self)
        self.__aircraftVid.pause()
        self.__destroyVid = avg.VideoNode(href='explosion.mov', active=False,
                threaded=False, parent=self)
        self.__destroyVid.pause()
        self.__shadowImg = avg.ImageNode(href=mediabase+'.gif', opacity=0.5,
                pos=self.pos + Aircraft.__SHADOW_OFFSET, parent=shadowdiv)
        self.__shadowImg.size *= Aircraft.__SHADOW_SCALE
        self.__shadowImg.setEffect(avg.BlurFXNode(6.0))

    def reset(self):
        self.__aircraftVid.active = True
        self.__aircraftVid.play()
        self.__destroyVid.active = False
        self.__destroyVid.pause()
        self.__shadowImg.active = True

    def destroy(self):
        self.__aircraftVid.active = False
        self.__aircraftVid.pause()
        self.__destroyVid.active = True
        self.__destroyVid.play()
        self.__destroyVid.seekToFrame(0)
        self.__shadowImg.active = False


class PlayerAircraft(Aircraft):
    def __init__(self, shadowdiv, **kwargs):
        super(PlayerAircraft, self).__init__('spitfire', shadowdiv, **kwargs)
        self.__engineSnd = avg.SoundNode(href='flySound.mp3', loop=True, parent=self)
        self.__bulletSnd = avg.SoundNode(href='bulletSound.mp3', volume=0.75, parent=self)

    def reset(self):
        super(PlayerAircraft, self).reset()
        self.__engineSnd.play()

    def destroy(self):
        super(PlayerAircraft, self).destroy()
        self.__engineSnd.stop()

    def shoot(self):
        self.__bulletSnd.play()
        self.__bulletSnd.seekToTime(0)


class EnemyAircraft(Aircraft):
    def __init__(self, shadowdiv, **kwargs):
        super(EnemyAircraft, self).__init__('enemy', shadowdiv, **kwargs)
        self.__destroySnd = avg.SoundNode(href='enemyDeath.mp3', volume=2.0, parent=self)

    def destroy(self):
        super(EnemyAircraft, self).destroy()
        self.__destroySnd.play()
        self.__destroySnd.seekToTime(0)


### gui elements ###

class ScrollingBackground(object):
    __SCROLL_SPEED = 120.0 # px/s

    def __init__(self, parent):
        self.__imgA = avg.ImageNode(href='ground.jpg', parent=parent)
        self.__imgB = avg.ImageNode(href='ground.jpg', pos=(0, -self.__imgA.height),
                parent=parent)

    def update(self, dt):
        dy = ScrollingBackground.__SCROLL_SPEED * dt
        self.__imgA.y += dy
        self.__imgB.y += dy
        if self.__imgA.y >= self.__imgA.height:
            self.__imgA.y = self.__imgB.y - self.__imgA.height
        elif self.__imgB.y >= self.__imgA.height:
            self.__imgB.y = self.__imgA.y - self.__imgA.height


class LiveCounter(avg.DivNode):
    __NUM_LIVES = 3

    def __init__(self, **kwargs):
        super(LiveCounter, self).__init__(**kwargs)
        self.__numLives = 0
        self.__images = []
        x = 0
        for i in xrange(LiveCounter.__NUM_LIVES):
            avg.ImageNode(href='gui_lives_bg.png', pos=(x, 0), parent=self)
            img = avg.ImageNode(href='gui_lives_fg.png', pos=(x, 0), parent=self)
            self.__images.append(img)
            x += img.width

    def reset(self):
        self.__numLives = 3
        for img in self.__images:
            avg.fadeIn(img, 250)

    def dec(self):
        assert(self.__numLives)
        self.__numLives -= 1
        avg.fadeOut(self.__images[self.__numLives], 250)
        return not self.__numLives


class ScoreCounter(avg.DivNode):
    def __init__(self, **kwargs):
        super(ScoreCounter, self).__init__(size=(3 * 34, 34), crop=True, **kwargs)
        self.__score = 0
        self.__images = [avg.ImageNode(href='gui_numbers.png', pos=((2 - i) * 34, 0),
                parent=self) for i in xrange(3)]

    def reset(self):
        self.__score = 0
        for img in self.__images:
            if img.y != 0:
                avg.LinearAnim(img, 'y', 250, img.y, 0).start()

    def inc(self):
        self.__score += 1
        s = self.__score
        for img in self.__images:
            y = s % 10 * -34
            if img.y != y:
                avg.LinearAnim(img, 'y', 250, img.y, y).start()
            s /= 10


class OverheatControl(avg.DivNode):
    __TEMPERATURE_INC = 30 # px/shot
    __TEMPERATURE_DEC = 60 # px/s

    def __init__(self, **kwargs):
        super(OverheatControl, self).__init__(**kwargs)
        bg = avg.ImageNode(href='gui_heatbar_bg.png', parent=self)
        self.__heatbar = avg.DivNode(size=(1, bg.height), crop=True, parent=self)
        avg.ImageNode(href='gui_heatbar_fg.png', parent=self.__heatbar)
        self.__maxTemp = bg.width - OverheatControl.__TEMPERATURE_INC

    def reset(self):
        self.__heatbar.width = 1

    def update(self, dt):
        dw = OverheatControl.__TEMPERATURE_DEC * dt
        if self.__heatbar.width > dw:
            self.__heatbar.width -= dw

    def shoot(self):
        if self.__heatbar.width < self.__maxTemp:
            self.__heatbar.width += OverheatControl.__TEMPERATURE_INC
            return True
        return False


### application ###

class TutorialApp(GameApp):
    multitouch = False

    def init(self):
        self._parentNode.mediadir = getMediaDir(__file__)

        self.__gameMusic = avg.SoundNode(href='Fire_Birds.mp3', loop=True,
                volume=0.75, parent=self._parentNode)
        self.__scrollingBg = ScrollingBackground(self._parentNode)
        shadowDiv = avg.DivNode(parent=self._parentNode)

        self.__guiDiv = avg.DivNode(parent=self._parentNode)
        bg = avg.ImageNode(href='gui_frame.png', parent=self.__guiDiv)
        self.__guiDiv.pos = (0, self._parentNode.height - bg.height)
        self.__liveCounter = LiveCounter(pos=(8, 12), parent=self.__guiDiv)
        self.__overheatCtrl = OverheatControl(pos=(300, 54), parent=self.__guiDiv)
        self.__scoreCounter = ScoreCounter(pos=(1142, 54), parent=self.__guiDiv)

        self.__player = PlayerAircraft(shadowDiv, parent=self._parentNode)
        self.__enemy0 = EnemyAircraft(shadowDiv, pos=(256, 0), parent=self._parentNode)
        Bullet(parent=self._parentNode)

        self.__frameHandlerId = None
        self.__gameMusic.play()
        self.__start()

    def onKeyDown(self, event):
        if self.__frameHandlerId is None:
            if event.keystring == 'space':
                self.__start()
                return True
            return False
        if event.keystring == 'space':
            if self.__overheatCtrl.shoot():
                self.__player.shoot()
            return True
        if event.keystring == '[-]':
            if self.__liveCounter.dec():
                self.__stop()
                self.__player.destroy()
                self.__enemy0.destroy()
            return True
        if event.keystring == '[+]':
            self.__scoreCounter.inc()
            return True
        return False

    def __start(self):
        assert(not self.__frameHandlerId)
        self.__liveCounter.reset()
        self.__overheatCtrl.reset()
        self.__scoreCounter.reset()
        self.__player.reset()
        self.__enemy0.reset()
        self.__frameHandlerId = g_player.setOnFrameHandler(self.__onFrame)

    def __stop(self):
        assert(self.__frameHandlerId)
        g_player.clearInterval(self.__frameHandlerId)
        self.__frameHandlerId = None

    def __onFrame(self):
        dt = g_player.getFrameDuration() / 1000.0
        self.__scrollingBg.update(dt)
        self.__overheatCtrl.update(dt)


if __name__ == '__main__':
    TutorialApp.start(resolution=(1280, 720))

