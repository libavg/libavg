#!/usr/bin/env python
# -*- coding: utf-8 -*-

# libavg - Media Playback Engine.
# Copyright (C) 2012-2013 Ulrich von Zadow
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

# TODO: some info foo here:
# * c't 09/2011 p.184, media sources/license
# * description, link to wiki tutorial page

from random import randint

from libavg import avg, player
from libavg import app
from libavg.utils import getMediaDir

player.loadPlugin('collisiondetector')

def playVideo(video):
    if not player.isUsingGLES():
        video.play()


### game elements ###

class Bullet(avg.VideoNode):
    __SPEED = 360 # px/s

    def __init__(self, parent=None, **kwargs):
        super(Bullet, self).__init__(href='bullet.mov', loop=True, active=False, **kwargs)
        self.registerInstance(self, parent)
        self.pause()

    def reset(self, pos):
        self.pos = pos
        self.active = True
        playVideo(self)

    def update(self, dt):
        y = self.y - Bullet.__SPEED * dt
        if y > -self.height:
            self.y = y
        else:
            self.destroy()

    def destroy(self):
        self.active = False
        self.pause()


class _Aircraft(avg.DivNode):
    _SPEED = 300 # px/s
    __SHADOW_SCALE = 0.5
    __SHADOW_OFFSET = avg.Point2D(170, 170)

    def __init__(self, mediabase, shadowdiv, parent=None, **kwargs):
        super(_Aircraft, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        self.__alive = False
        self.__aircraftVid = avg.VideoNode(href=mediabase+'.mov', loop=True, parent=self)
        self.__aircraftVid.pause()
        self.__destroyVid = avg.VideoNode(href='explosion.mov', active=False,
                threaded=False, parent=self)
        self.__destroyVid.pause()
        self.__destroyVid.subscribe(avg.VideoNode.END_OF_FILE, self._hide)
        self.__shadowImg = avg.ImageNode(href=mediabase+'.gif', opacity=0.5,
                pos=self.pos + _Aircraft.__SHADOW_OFFSET, parent=shadowdiv)
        self.__shadowImg.size *= _Aircraft.__SHADOW_SCALE
        if not player.isUsingGLES():
            self.__shadowImg.setEffect(avg.BlurFXNode(6.0))
        self.size = self.__aircraftVid.size
        #self._debug('created')

    @property
    def alive(self):
        return self.__alive

    def reset(self):
        #self._debug('reset')
        self.active = True
        self.__alive = True
        self.__aircraftVid.active = True
        playVideo(self.__aircraftVid)
        self.__destroyVid.active = False
        self.__destroyVid.pause()
        self.__shadowImg.active = True

    def destroy(self):
        #self._debug('destroy')
        self.__alive = False
        self.__aircraftVid.active = False
        self.__aircraftVid.pause()
        self.__destroyVid.active = True
        playVideo(self.__destroyVid)
        self.__destroyVid.seekToFrame(0)
        self.__shadowImg.active = False
        if player.isUsingGLES():
            self._hide()

    def _move(self, pos):
        self.pos = pos
        self.__shadowImg.pos = self.pos + _Aircraft.__SHADOW_OFFSET

    def _hide(self):
        #self._debug('hide')
        self.active = False
        self.__alive = False
        self.__aircraftVid.pause()
        self.__destroyVid.pause()
        self.__shadowImg.active = False

    @classmethod
    def _debug(cls, msg):
        print '%6d [%s] %s' %(player.getFrameTime(), cls.__name__, msg)


class PlayerAircraft(_Aircraft):
    ACTION_KEYS = ('left', 'right', 'up', 'down', 'space')
    __BULLET_OFFSET_L = avg.Point2D( 52, 16)
    __BULLET_OFFSET_R = avg.Point2D(140, 16)

    def __init__(self, shadowdiv, gunCtrl, parent=None, **kwargs):
        super(PlayerAircraft, self).__init__('spitfire', shadowdiv, parent, **kwargs)
        self.__gunCtrl = gunCtrl
        self.__bullets = [Bullet(parent=self.parent) for i in xrange(10)]
        self.__engineSnd = avg.SoundNode(href='flySound.mp3', loop=True, parent=self)
        self.__bulletSnd = avg.SoundNode(href='bulletSound.mp3', volume=0.75, parent=self)
        self.__maxX, self.__maxY = self.parent.size - self.size

    def reset(self):
        super(PlayerAircraft, self).reset()
        self.__gunCtrl.reset()
        self._move((self.__maxX / 2, self.__maxY))
        self.__engineSnd.play()

    def destroy(self):
        super(PlayerAircraft, self).destroy()
        self.__engineSnd.stop()

    def update(self, dt, keyStates):
        d = _Aircraft._SPEED * dt
        dx = 0
        if keyStates['left']:
            dx = -d
        if keyStates['right']:
            dx += d
        dy = 0
        if keyStates['up']:
            dy = -d
        if keyStates['down']:
            dy += d
        pos = (max(min(self.x + dx, self.__maxX), 0),
               max(min(self.y + dy, self.__maxY), 0))
        if pos != self.pos:
            self._move(pos)

        if keyStates['space'] and self.__gunCtrl.shoot():
            # fire bullets
            bulletLeft = None
            bulletRight = None
            for b in self.__bullets:
                if b.active:
                    b.update(dt)
                elif not bulletLeft:
                    bulletLeft = b
                elif not bulletRight:
                    bulletRight = b
            if not bulletLeft:
                bulletLeft = Bullet(parent=self.parent)
                self.__bullets.append(bulletLeft)
            if not bulletRight:
                bulletRight = Bullet(parent=self.parent)
                self.__bullets.append(bulletRight)
            bulletLeft.reset(self.pos + PlayerAircraft.__BULLET_OFFSET_L)
            bulletRight.reset(self.pos + PlayerAircraft.__BULLET_OFFSET_R)
            self.__bulletSnd.play()
            self.__bulletSnd.seekToTime(0)
        else:
            self.__gunCtrl.update(dt)
            self.updateBullets(dt)

    def updateBullets(self, dt):
        bulletsAlive = False
        for b in self.__bullets:
            if b.active:
                bulletsAlive = True
                b.update(dt)
        return bulletsAlive

    def getActiveBullets(self):
        return [b for b in self.__bullets if b.active]


class EnemyAircraft(_Aircraft):
    ESCAPED = avg.Publisher.genMessageID()

    def __init__(self, shadowdiv, parent=None, **kwargs):
        super(EnemyAircraft, self).__init__('enemy', shadowdiv, parent, **kwargs)
        self.publish(EnemyAircraft.ESCAPED)
        self.__destroySnd = avg.SoundNode(href='enemyDeath.mp3', volume=2.0, parent=self)
        self._hide()

    def reset(self):
        super(EnemyAircraft, self).reset()
        self._move((randint(0, self.parent.width - self.width), -self.height))

    def destroy(self):
        super(EnemyAircraft, self).destroy()
        self.__destroySnd.play()
        self.__destroySnd.seekToTime(0)

    def update(self, dt):
        y = self.y + _Aircraft._SPEED * dt
        if y < self.parent.height:
            self._move((self.x, y))
        else:
            self._hide()
            self.notifySubscribers(EnemyAircraft.ESCAPED, [])


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

    def __init__(self, parent=None, **kwargs):
        super(LiveCounter, self).__init__(**kwargs)
        self.registerInstance(self, parent)
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
    def __init__(self, parent=None, **kwargs):
        super(ScoreCounter, self).__init__(size=(3 * 34, 34), crop=True, **kwargs)
        self.registerInstance(self, parent)
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
        self.__updateImages()

    def dec(self):
        if self.__score > 0:
            self.__score -= 1
            self.__updateImages()

    def __updateImages(self):
        s = self.__score
        for img in self.__images:
            y = s % 10 * -34
            if img.y != y:
                avg.LinearAnim(img, 'y', 250, img.y, y).start()
            s /= 10


class GunControl(avg.DivNode):
    __SHOOT_INTERVAL = 1.0 / 7 # ms
    __TEMPERATURE_INC = 30 # px/shot
    __TEMPERATURE_DEC = 60 # px/s

    def __init__(self, parent=None, **kwargs):
        super(GunControl, self).__init__(**kwargs)
        self.registerInstance(self, parent)
        bg = avg.ImageNode(href='gui_heatbar_bg.png', parent=self)
        self.__heatbar = avg.DivNode(size=(1, bg.height), crop=True, parent=self)
        avg.ImageNode(href='gui_heatbar_fg.png', parent=self.__heatbar)
        self.__maxTemp = bg.width - GunControl.__TEMPERATURE_INC
        self.__shootTimeout = 0.0

    def reset(self):
        self.__heatbar.width = 1

    def update(self, dt):
        if self.__shootTimeout > 0.0:
            self.__shootTimeout = max(0.0, self.__shootTimeout - dt)
        dw = GunControl.__TEMPERATURE_DEC * dt
        if self.__heatbar.width > dw:
            self.__heatbar.width -= dw

    def shoot(self):
        if self.__shootTimeout == 0.0 and self.__heatbar.width < self.__maxTemp:
            self.__shootTimeout = GunControl.__SHOOT_INTERVAL
            self.__heatbar.width += GunControl.__TEMPERATURE_INC
            return True
        return False


### application ###

class FireBirds(app.MainDiv):
    ENEMY_SPAWN_TIMEOUT = 1000 # ms

    def onInit(self):
        self.mediadir = getMediaDir(__file__)

        self.__gameMusic = avg.SoundNode(href='Fire_Birds.mp3', loop=True,
                volume=0.75, parent=self)
        self.__scrollingBg = ScrollingBackground(self)
        self.__shadowDiv = avg.DivNode(parent=self)
        self.__gameDiv = avg.DivNode(size=self.size, parent=self)
        self.__guiDiv = avg.DivNode(parent=self)

        bg = avg.ImageNode(href='gui_frame.png', parent=self.__guiDiv)
        self.__guiDiv.pos = (0, self.height - bg.height)
        self.__liveCounter = LiveCounter(pos=(8, 12), parent=self.__guiDiv)
        gunCtrl = GunControl(pos=(300, 54), parent=self.__guiDiv)
        self.__scoreCounter = ScoreCounter(pos=(1142, 54), parent=self.__guiDiv)

        self.__enemies = []
        for i in xrange(2):
            self.__createEnemy()
        self.__player = PlayerAircraft(self.__shadowDiv, gunCtrl, parent=self.__gameDiv)

        enemyMask = avg.Bitmap(self.mediadir + '/enemy.gif')
        self.__playerCollisionDetector = collisiondetector.CollisionDetector(
                enemyMask, avg.Bitmap(self.mediadir + '/spitfire.gif'))
        self.__bulletCollisionDetector = collisiondetector.CollisionDetector(
                enemyMask, avg.Bitmap(self.mediadir + '/bullet.gif'))

        self.__keyStates = dict.fromkeys(PlayerAircraft.ACTION_KEYS, False)
        self.__frameHandlerId = None
        self.__spawnTimeoutId = None
        self.__gameMusic.play()
        self.__start()
        player.subscribe(player.KEY_DOWN, self.__onKeyDown)
        player.subscribe(player.KEY_UP, self.__onKeyUp)

    def __onKeyDown(self, event):
        if self.__player.alive:
            if event.keystring in PlayerAircraft.ACTION_KEYS:
                self.__keyStates[event.keystring] = True
                return True
            return False
        if not self.__frameHandlerId: # game stopped
            if event.keystring == 'space':
                self.__start()
                return True
        # else: wait for bullets and enemies to leave the screen
        return False

    def __onKeyUp(self, event):
        if event.keystring in PlayerAircraft.ACTION_KEYS:
            self.__keyStates[event.keystring] = False
            return True
        return False

    def __start(self):
        assert(not self.__frameHandlerId and not self.__spawnTimeoutId)
        self.__liveCounter.reset()
        self.__scoreCounter.reset()
        self.__player.reset()
        self.__frameHandlerId = player.subscribe(player.ON_FRAME, self.__onFrame)
        self.__spawnTimeoutId = player.setInterval(self.ENEMY_SPAWN_TIMEOUT,
                self.__spawnEnemy)

    def __stop(self):
        assert(self.__frameHandlerId and self.__spawnTimeoutId)
        player.clearInterval(self.__spawnTimeoutId)
        self.__spawnTimeoutId = None

    def __createEnemy(self):
        enemy = EnemyAircraft(self.__shadowDiv, parent=self.__gameDiv)
        enemy.subscribe(EnemyAircraft.ESCAPED, self.__scoreCounter.dec)
        self.__enemies.append(enemy)
        return enemy

    def __spawnEnemy(self):
        assert(self.__frameHandlerId)
        enemy = None
        for e in self.__enemies:
            if not e.active:
                enemy = e
                break
        if not enemy:
            enemy = self.__createEnemy()
        enemy.reset()

    def __onFrame(self):
        dt = player.getFrameDuration() * 0.001
        self.__scrollingBg.update(dt)

        bullets = self.__player.getActiveBullets()
        enemiesActive = False
        for e in self.__enemies:
            if e.active:
                enemiesActive = True
                if e.alive:
                    for b in bullets:
                        if self.__bulletCollisionDetector.detect(e.pos, b.pos):
                            self.__scoreCounter.inc()
                            e.destroy()
                            b.destroy()
                            break
                if e.alive: # no bullet hit
                    if self.__player.alive and \
                            self.__playerCollisionDetector.detect(e.pos, self.__player.pos):
                        e.destroy()
                        if self.__liveCounter.dec():
                            self.__stop()
                            self.__player.destroy()
                if e.alive: # no player collision
                    e.update(dt)

        if self.__player.alive:
            self.__player.update(dt, self.__keyStates)
        elif not self.__player.updateBullets(dt) and not enemiesActive:
            # player dead, all bullets and enemies left the screen, all destroy videos played
            player.unsubscribe(player.ON_FRAME, self.__frameHandlerId)
            self.__frameHandlerId = None


if __name__ == '__main__':
    app.App().run(FireBirds(), app_resolution='1280x720')

