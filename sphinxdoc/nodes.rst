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

            Read-only.

        .. py:attribute:: driver

            Read-only.

        .. py:attribute:: framenum

            The number of frames the camera has read since playback started. Read-only.

        .. py:attribute:: framerate

            Read-only.

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

        .. py:method:: getChild(i) -> Node

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

        Base class for vector nodes which have a filled area and a border.

        .. py:method:: __init__([filltexhref, fillopacity=0, fillcolor="FFFFFF", filltexcoord1=Point2D(0,0), filltexcoord2=Point2D(1,1)])

        .. py:attribute:: fillcolor

        .. py:attribute:: fillopacity

        .. py:attribute:: filltexcoord1

        .. py:attribute:: filltexcoord2

        .. py:attribute:: filltexhref

            An image file to use as a texture for the area of the node.

        .. py:method:: setFillBitmap(bitmap)

            Sets a bitmap to use as a fill texture. Sets filltexhref to an empty
            string.

    .. autoclass:: ImageNode

        A static raster image on the screen. The content of an ImageNode can be loaded
        from a file. It can also come from a :py:class:`Bitmap` object or from an 
        :py:class:`OffscreenCanvas`. Alpha channels of the image files are used as
        transparency information.

        .. py:method:: __init__([href, compression])

        .. py:attribute:: compression

            The texture compression used for this image. Currently, :py:const:`none`
            and :py:const:`B5G6R5` are supported. :py:const:`B5G6R5` causes the bitmap 
            to be compressed to 16 bit per pixel on load and is only valid if the source 
            is a filename. Read-only.

        .. py:attribute:: href

            In the standard case, this is the source filename of the image. To use a
            bitmap as source, call setBitmap().  To use an offscreen canvas as source, 
            use the :samp:`canvas:` protocol: :samp:`href="canvas:{id}"`.

        .. py:method:: setBitmap(bitmap)

            Sets a bitmap to use as content for the ImageNode. Sets href to an empty 
            string.

    .. autoclass:: LineNode

        A line. :py:attr:`pos1` and :py:attr:`pos2` are the two endpoints of the line.

        .. py:method:: __init__([pos1, pos2, texcoord1, texcoord2])

        .. py:attribute:: pos1

        .. py:attribute:: pos2

        .. py:attribute:: texcoord1

        .. py:attribute:: texcoord2

    .. autoclass:: MeshNode

        This is a generalized mesh of textured triangles. See 
        https://www.libavg.de/wiki/index.php/Mesh_Node for an example.

        .. py:method:: __init__([vertexcoords, texcoords, triangles])

        .. py:attribute:: texcoords

        .. py:attribute:: triangles

        .. py:attribute:: vertexcoords

    .. autoclass:: Node

        Base class for everything that can be put into an avg tree.

        .. py:method:: __init__([id: string])

        .. py:attribute:: id

            A unique identifier that can be used to reference the node, for instance using
            :py:meth:`Player.getElementByID`. Read-only.

    .. autoclass:: PanoImageNode

            A panorama image displayed in cylindrical projection.

            .. deprecated:: 1.5
                This is unsupported and probably buggy.

        .. py:method:: __init__([href, sensorwidth, sensorheight, focallength, rotation])

        .. py:attribute:: focallength

            The focal length of the lens in millimeters.

        .. py:attribute:: href

            The source filename of the image.

        .. py:attribute:: maxrotation

            The maximum angle the viewer can look at. Read-only.

        .. py:attribute:: rotation

            The current angle the viewer is looking at in radians.

        .. py:attribute:: sensorheight

            The height of the sensor used to make the image.

        .. py:attribute:: sensorwidth

            The width of the sensor used to make the image. This value
            is used together with sensorheight and focallength to
            determine the projection to use.

        .. py:method:: getScreenPosFromPanoPos(panoPos: Point2D) -> Point2D

            Converts a position in panorama image pixels to pixels in coordinates
            relative to the node, taking into account the current rotation angle.

        .. py:method:: getScreenPosFromAngle(angle) -> Point2D

            Converts panorama angle to pixels in coordinates
            relative to the node, taking into account the current rotation angle.

    .. autoclass:: PolygonNode

        A closed figure bounded by a number of line segments, optionally filled. Filled
        polygons may not be self-intersecting.

        .. py:method:: __init__([linejoin:"bevel", pos, texcoords])

        .. py:attribute:: linejoin

            The method by which line segments are joined together. Valid values are 
            :py:const:`bevel` and :py:const:`miter`

        .. py:attribute:: pos

            A sequence (:py:class:`list` or :py:class:`tuple`) of pixel positions.

        .. py:attribute:: texcoords

            A sequence of float texture coordinates corresponding to the border positions.
            
    .. autoclass:: PolyLineNode

        A figure similar to a polygon, but not closed and never filled. 

        .. py:method:: __init__([linejoin:"bevel", pos, texcoords])

        .. py:attribute:: linejoin

            The method by which line segments are joined together. Valid values are 
            :py:const:`bevel` and :py:const:`miter`

        .. py:attribute:: pos

            A sequence (:py:class:`list` or :py:class:`tuple`) of pixel positions.

        .. py:attribute:: texcoords

            A sequence of float texture coordinates corresponding to the border positions.
            
    .. autoclass:: RasterNode

        Base class for all nodes that have a direct 2d raster representation.
        This includes Image, Word, Camera, and Video nodes. The base class implements
        color controls (:py:attr:`contrast`, :py:attr:`intensity`, :py:attr:`gamma`),
        alpha masks (:py:attr:`maskhref`, :py:attr:`maskpos`, :py:attr:`masksize`), 
        several blend modes that define how compositing is done and mipmapping suport.

        Any Raster Node can have a GPU-based effect added to it by using 
        :py:meth:`setEffect`.

        In addition, RasterNodes can be warped. By default, a RasterNode is rectangular.
        However, it can be subdivided into a grid of reference points using 
        :py:attr:`maxtilewidth` and :py:attr:`maxtileheight`. The position of each of 
        these points can be changed with :py:meth:`getOrigVertexCoords`, 
        :py:meth:`getWarpedVertexCoords` and :py:meth:`setWarpedVertexCoords`, 
        yielding arbitrary shapes.
        
        .. py:method:: __init__([maxtilewidth, maxtileheight, blendmode, mipmap, maskhref, maskpos, masksize, gamma, contrast, intensity])

        .. py:attribute:: maxtilewidth

            The maximum width of the tiles used for warping. The effective tile size is 
            also dependent on hardware and driver limits. Read-only.

        .. py:attribute:: maxtileheight

            The maximum height of the tiles used for warping. The effective tile size is
            also dependent on hardware and driver limits. Read-only. 

        .. py:attribute:: blendmode

            The method of compositing the node with the nodes under
            it. Valid values are :py:const:`blend`, :py:const:`add`, :py:const:`min` 
            and :py:const:`max`. For :py:const:`min` and :py:const:`max`
            blend modes, opacity is ignored.

        .. py:attribute:: maskhref

            The source filename for a mask image to be used as alpha channel.
            Where this file is white, the node is shown. Where it is black, the
            node is transparent. If the node is an image with an alpha channel,
            the alpha channel is replaced by the mask.

        .. py:attribute:: maskpos

            An offset for the mask image. For images and videos, the offset is
            given in image or video pixels, respectively. For words nodes, the
            offset is given in screen pixels. If portions of the node extend
            outside the mask, the border pixels of the mask are taken.

        .. py:attribute:: masksize

            The size of the mask image. For images and videos, the size is
            given in image or video pixels, respectively. For words nodes, the
            size is given in screen pixels. If portions of the node extend
            outside the mask, the border pixels of the mask are taken.

        .. py:attribute:: mipmap

            Determines whether mipmaps (http://en.wikipedia.org/wiki/Mipmap) are 
            generated for this node. Setting this to True improves the quality of 
            minified nodes. Depending on the graphics card in use, turning on mipmaps
            may cause a performance hit for every image change. Read-only.

        .. py:attribute:: gamma

            Allows node-specific gamma correction. gamma is a triple that
            contains separate float values for red, green, and blue. A gamma value of
            1.0 in all channels leaves the image unchanged. Higher gamma values
            increase, lower values decrease the brightness. In all cases, black
            white pixels are not affected by gamma. See also 
            http://en.wikipedia.org/wiki/Gamma_correction.

        .. py:attribute:: intensity

            A control for the brightness of the node. intensity is a triple
            that contains separate float values for red, green, and blue. An intensity
            value of 1.0 in all channels leaves the image unchanged. This value
            corresponds to the photoshop brightness value.

        .. py:attribute:: contrast

            A control for the color contrast of the node. contrast is a triple
            that contains separate float values for red, green, and blue. A contrast
            value of 1.0 in all channels leaves the image unchanged.
        
        .. py:method:: getBitmap() -> Bitmap

            Returns a copy of the bitmap that the node contains.

        .. py:method:: getOrigVertexCoords() -> list

            Returns the unwarped coordinate of all vertices as a list of lists.

        .. py:method:: getWarpedVertexCoords() -> list

            Returnes the current coordinate of all vertices as a list of lists.

        .. py:method:: setWarpedVertexCoords(grid)

            Changes the current coordinates of all vertices. :py:attr:`grid` is a list of
            lists of coordinate tuples.

        .. py:method:: setEffect(FXNode)

            Sets an :py:class:`FXNode` that modifies the way the node looks.

    .. autoclass:: RectNode
    .. autoclass:: SoundNode
    .. autoclass:: VectorNode
    .. autoclass:: VideoNode
    .. autoclass:: VisibleNode
    .. autoclass:: WordsNode

