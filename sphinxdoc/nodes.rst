Nodes
=====

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: AreaNode CameraNode CanvasNode CircleNode CurveNode DivNode FilledVectorNode ImageNode LineNode MeshNode Node PanoImageNode PolygonNode PolyLineNode RasterNode RectNode SoundNode VectorNode VideoNode VisibleNode WordsNode
        :parts: 1

    .. autoclass:: AVGNode

        Root node of an onscreen avg tree. Defines the properties of the display
        and handles key press events. The AVGNode's width and height define the
        coordinate system for the display and are the default for the window
        size used (i.e. by default, the coordinate system is pixel-based).

        .. py:method:: __init__([onkeydown: string, onkeyup: string])
        
            :param string onkeyup:
        
                Name of python function to call when a key up
                event occurs.
        
                .. deprecated:: 1.5
                    Use :func:`VisibleNode.setEventHandler()` instead.
        
            :param string onkeydown:
        
                Name of python function to call when a key
                down event occurs.
        
                .. deprecated:: 1.5
                    Use :func:`VisibleNode.setEventHandler()` instead.

    .. autoclass:: AreaNode

        Base class for elements in the avg tree that define an area on the screen.
        Responsible for coordinate transformations and event handling. See 
        http://www.libavg.de/wiki/index.php/Coordinate_Systems
        for an explanation of coordinate systems and reference points.
        
        .. py:method:: __init__([x, y, pos, width, height, size, angle, pivot])
        
        .. py:method:: getMediaSize() -> avg.Point2D

            Returns the size in pixels of the media in the node. Image nodes
            return the bitmap size, Camera nodes
            the size of a camera frame and Words nodes the amount of space
            the text takes. Video nodes return the video size if decoding has
            started or (0,0) if not. Decoding starts after :py:func:`play` or 
            :py:func:`pause`
            is called and the node can be rendered.

        .. py:attribute:: x

            This is the horizontal position of the node's reference point 
            relative to it's parent node. 

        .. py:attribute:: y

            This is the vertical position of the node's reference point 
            relative to it's parent node. 

        .. py:attribute:: pos

            This is the position of the node's reference point 
            relative to it's parent node. 

        .. py:attribute:: width

        .. py:attribute:: height

        .. py:attribute:: angle
        
            The angle that the node is rotated to in radians. 0 is
            unchanged, 3.14 is upside-down.

        .. py:attribute:: size

            The size that the node takes on the canvas. Node types usually have sensible
            defaults for the size. For media nodes, this is generally the size of the
            media (so :samp:`size == getMediaSize()`). For DivNodes, the default size is
            infinite.

        .. py:attribute:: pivot

            The position of the point that the node is rotated around.
            Default is the center of the node.


    .. autoclass:: CameraNode

        A node that displays the image of a camera. The attributes correspond to the 
        camera properties in .avgtrackerrc and are explained under
        http://www.libavg.de/wiki/index.php/Tracker_Setup. An easy way to find the 
        appropriate parameters for your camera is to use :command:`avg_showcamera.py`.

        CameraNodes open the camera device on construction and set the chosen camera 
        parameters immediately.   

        .. py:method:: __init__([driver='firewire', device="", unit=-1, fw800=False, framerate=15, capturewidth=640, captureheight=480, pixelformat="RGB", brightness, exposure, sharpness, saturation, camgamma, shutter, gain, strobeduration])

        .. py:attribute:: brightness

        .. py:attribute:: camgamma

        .. py:attribute:: device

            read-only

        .. py:attribute:: driver

            read-only

        .. py:attribute:: framenum

            The number of frames the camera has read since playback started.

        .. py:attribute:: framerate

            read-only

        .. py:attribute:: gain

        .. py:attribute:: saturation

        .. py:attribute:: sharpness

        .. py:attribute:: shutter

        .. py:attribute:: strobeduration

        .. py:method:: doOneShotWhitebalance()

        .. py:method:: getWhitebalanceU() -> int
        
        .. py:method:: getWhitebalanceV() -> int

        .. py:method:: isAvailable() -> bool

            Returns True if there is a working device that can deliver images attached to
            the CameraNode.

        .. py:method:: play()

            Starts reading images from the camera device and displays them. Note that the
            camera device is opened on construction of the CameraNode.

        .. py:method:: setWhitebalance(u, v)

        .. py:method:: stop()

            Stops camera playback.

        .. py:classmethod:: dumpCameras()

            Dumps a list of available cameras to the console.

        .. py:classmethod:: resetFirewireBus()

            Frees all allocated bandwidth and devices on the firewire bus. Helpful
            if a program using a firewire device has crashed leaving resources
            allocated. Note that all firewire devices (including for instance
            external hard drives) are affected.

    .. autoclass:: CanvasNode

        Root node of a scene graph.

    .. autoclass:: CircleNode

        A circle. The reference point for a circle is it's center.

        .. py:method:: __init__([r=1, texcoord1=0, texcoord2=1])

        .. py:attribute:: r

            The radius of the circle in pixels.

        .. py:attribute:: texcoord1

        .. py:attribute:: texcoord2

    .. autoclass:: CurveNode

        A bezier curve (`<http://en.wikipedia.org/wiki/Bézier_curve>`_). :py:attr:`pos1`
        and :py:attr:`pos4` are the two endpoints of the curve. :py:attr:`pos2`
        and :py:attr:`pos3` are control points.

        .. py:method:: __init__([pos1, pos2, pos3, pos4, texcoord1, texcoord2])

        .. py:attribute:: pos1

        .. py:attribute:: pos2

        .. py:attribute:: pos3

        .. py:attribute:: pos4

        .. py:attribute:: texcoord1

        .. py:attribute:: texcoord2

    .. autoclass:: DivNode

        A div node is a node that groups other nodes logically and visually.
        Its positino is used as point of origin for the coordinates
        of its child nodes. Its extents can be used to clip the children if crop is set 
        to True. Its opacity is used as base opacity for the child nodes' opacities.
        The children of a div node are drawn in the order they are found
        in the avg file, so the first one is below all others in z-order.
        
        .. py:method:: __init__([crop=False, elementoutlinecolor, mediadir])

        .. py:attribute:: crop

            Boolean that turns clipping on or off.
        
        .. py:attribute:: elementoutlinecolor

            Allows debugging of div node nesting by rendering the outlines of
            this div and all its div children in the specified color (given in html hex
            format). Turn off by setting the color to the empty string.

        .. py:attribute:: mediadir
            
            The directory that the media files for the children of this node are
            in. Relative mediadirs are taken to mean subdirectories of the parent node's 
            mediadir.

        .. py:method:: getNumChildren() -> int

            Returns the number of immediate children that this div contains.

        .. py:method:: getChild(i) -> avg.Node
        
            Returns the child at index i.

        .. py:method:: appendChild(node)

            Adds a new child to the container behind the last existing child.

        .. py:method:: insertChildBefore(newNode, oldChild)

            Adds a new child to the container before the existing node
            oldChild. In z-order, the new child ist behind the old one.

        .. py:method:: insertChildAfter(newNode, oldChild)

            Adds a new child to the container after the existing node
            oldChild. In z-order, the new child ist in front of the old one.

        .. py:method:: insertChild(node, i)

            Adds a new child to the container at index i.

        .. py:method:: removeChild(node)

            Removes the child given by node from the div. Note that as long as other
            references to the node exist, the node is not deleted.

        .. py:method:: removeChild(i)

            Removes the child at index i from the div. Note that as long as other
            references to the node exist, the node is not deleted.

        .. py:method:: reorderChild(oldIndex, newIndex)

            Moves the child at oldIndex so it's at newIndex. This function
            can be used to change the order in which the children are drawn.

        .. py:method:: reorderChild(node, newPos)

            Moves the child node so it's at index newPos. This function
            can be used to change the order in which the children are drawn.

        .. py:method:: indexOf(node)

            Returns the index of the node given or -1 if node isn't a
            child of the container. 

        .. py:method:: getEffectiveMediaDir() -> string

            Returns the nodes' effective mediadir by traversing the node
            hierarchy up to the root node.

    .. autoclass:: FilledVectorNode
    .. autoclass:: ImageNode
    .. autoclass:: LineNode
    .. autoclass:: MeshNode
    .. autoclass:: Node
    .. autoclass:: PanoImageNode
    .. autoclass:: PolygonNode
    .. autoclass:: PolyLineNode
    .. autoclass:: RasterNode
    .. autoclass:: RectNode
    .. autoclass:: SoundNode
    .. autoclass:: VectorNode
    .. autoclass:: VideoNode
    .. autoclass:: VisibleNode
    .. autoclass:: WordsNode

