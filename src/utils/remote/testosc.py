#!/usr/bin/python

import OSC

client = OSC.Client("192.168.178.22",12000)

Source = "/track/"
Type = "up"

contour = [[1,2],[3,4],[5,6],[7,8]]

posMsg = OSC.Message()
posMsg.setAddress(Source+Type)
posMsg.append(1412)
posMsg.append(200)
posMsg.append(200)

contMsg = OSC.Message()

bundle = OSC.Bundle()
bundle.append(posMsg)

i = 0
for point in contour:
    contMsg = OSC.Message()
    contMsg.setAddress(Source+'cont/v/'+str(i))
    contMsg.append(point[0])
    contMsg.append(point[1])

    bundle.append(contMsg)
    i = i+1

client.sendRawMessage(bundle.getRawMessage())

