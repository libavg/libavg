import math
from libavg import avg, Point2D, RasterNode

g_Player = avg.Player.get()

class TrackerImageFlipper:
    def __init__(self):
        self.readConfig()

    def readConfig(self):
        global g_tracker
        print "reading tracker config"
        g_tracker = g_Player.getTracker()
        trackerAngle = float(g_tracker.getParam('/transform/angle/@value'))
        self.angle = round(trackerAngle/math.pi) * math.pi
        self.flipX = 0 > float(g_tracker.getParam('/transform/displayscale/@x'))
        self.flipY = 0 > float(g_tracker.getParam('/transform/displayscale/@y'))

    def transformPos(self, (x, y)):
        if self.flipX:
            x = 1 - x
        if self.flipY:
            y = 1 - y
        return (x, y)

    def flipNode(self, node):
        node.angle = self.angle
        grid = node.getOrigVertexCoords()
        grid = [ [ self.transformPos(pos) for pos in line ] for line in grid]
        node.setWarpedVertexCoords(grid)

    def loadTrackerImage(self, node, imageID):
        fingerBitmap = g_tracker.getImage(imageID)
        node.setBitmap(fingerBitmap)
        self.flipNode(node)

