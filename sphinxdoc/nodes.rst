Nodes
=====

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: AreaNode CameraNode CanvasNode CircleNode CurveNode DivNode FilledVectorNode ImageNode LineNode MeshNode Node PanoImageNode PolygonNode PolyLineNode RasterNode RectNode SoundNode VectorNode VideoNode VisibleNode WordsNode
        :parts: 1

    .. autoclass:: AVGNode([onkeydown: string, onkeyup: string])

        Root node of an onscreen avg tree. Defines the properties of the display
        and handles key press events. The AVGNode's width and height define the
        coordinate system for the display and are the default for the window
        size used (i.e. by default, the coordinate system is pixel-based).

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

    .. autoclass:: AreaNode([x, y, pos, width, height, size, angle, pivot])

        Base class for elements in the avg tree that define an area on the screen.
        Responsible for coordinate transformations and event handling. See 
        http://www.libavg.de/wiki/index.php/Coordinate_Systems
        for an explanation of coordinate systems and reference points.
        
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


    .. autoclass:: CameraNode([driver='firewire', device="", unit=-1, fw800=False, framerate=15, capturewidth=640, captureheight=480, pixelformat="RGB", brightness, exposure, sharpness, saturation, camgamma, shutter, gain, strobeduration])

        A node that displays the image of a camera. The attributes correspond to the 
        camera properties in .avgtrackerrc and are explained under
        http://www.libavg.de/wiki/index.php/Tracker_Setup. An easy way to find the 
        appropriate parameters for your camera is to use :command:`avg_showcamera.py`.

        CameraNodes open the camera device on construction and set the chosen camera 
        parameters immediately.   

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

            Returns :keyword:`True` if there is a working device that can deliver images
            attached to the CameraNode.

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

    .. autoclass:: CircleNode([r=1, texcoord1=0, texcoord2=1])

        A circle. The reference point for a circle is it's center.

        .. py:attribute:: r

            The radius of the circle in pixels.

        .. py:attribute:: texcoord1

        .. py:attribute:: texcoord2

    .. autoclass:: CurveNode([pos1, pos2, pos3, pos4, texcoord1, texcoord2])

        A cubic bezier curve (`<http://en.wikipedia.org/wiki/Bezier_curve>`_). 
        :py:attr:`pos1` and :py:attr:`pos4` are the two endpoints of the curve. 
        :py:attr:`pos2` and :py:attr:`pos3` are control points.

        .. py:method:: __init__([pos1, pos2, pos3, pos4, texcoord1, texcoord2])

        .. py:attribute:: pos1

        .. py:attribute:: pos2

        .. py:attribute:: pos3

        .. py:attribute:: pos4

        .. py:attribute:: texcoord1

        .. py:attribute:: texcoord2

    .. autoclass:: DivNode

        A div node is a node that groups other nodes logically and visually.
        Its position is used as point of origin for the coordinates
        of its child nodes. Its extents can be used to clip the children if crop is set 
        to :keyword:`True`. Its opacity is used as base opacity for the child nodes' 
        opacities. The children of a div node are drawn in the order they are found
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

        Base class for vector nodes which have a filled area and a border. The area can
        be filled either with a solid color (:py:attr:`fillcolor`) or with a texture
        loaded from a file (:py:attr:`filltexhref`) or taken from a bitmap object 
        (:py:meth:`setFillBitmap`).

        .. py:method:: __init__([filltexhref, fillopacity=0, fillcolor="FFFFFF", filltexcoord1=Point2D(0,0), filltexcoord2=Point2D(1,1)])

        .. py:attribute:: fillcolor

        .. py:attribute:: fillopacity

        .. py:attribute:: filltexcoord1

        .. py:attribute:: filltexcoord2

        .. py:attribute:: filltexhref

            An image file to use as a texture for the area of the node.

        .. py:method:: setFillBitmap(bitmap)

            Sets a bitmap to use as a fill texture. Sets :attr:`filltexhref` to an empty
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
            :py:const:`bevel` and :py:const:`miter`.

        .. py:attribute:: pos

            A sequence (:py:class:`list` or :py:class:`tuple`) of pixel positions.

        .. py:attribute:: texcoords

            A sequence of float texture coordinates corresponding to the border positions.
            
    .. autoclass:: PolyLineNode

        A figure similar to a :py:class:`PolygonNode`, but not closed and never filled. 

        .. py:method:: __init__([linejoin:"bevel", pos, texcoords])

        .. py:attribute:: linejoin

            The method by which line segments are joined together. Valid values are 
            :py:const:`bevel` and :py:const:`miter`.

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

        .. py:attribute:: blendmode

            The method of compositing the node with the nodes under
            it. Valid values are :py:const:`blend`, :py:const:`add`, :py:const:`min` 
            and :py:const:`max`. For :py:const:`min` and :py:const:`max`
            blend modes, opacity is ignored.

        .. py:attribute:: contrast

            A control for the color contrast of the node. contrast is a triple
            that contains separate float values for red, green, and blue. A contrast
            value of 1.0 in all channels leaves the image unchanged.
        
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

        .. py:attribute:: maxtileheight

            The maximum height of the tiles used for warping. The effective tile size is
            also dependent on hardware and driver limits. Read-only. 

        .. py:attribute:: maxtilewidth

            The maximum width of the tiles used for warping. The effective tile size is 
            also dependent on hardware and driver limits. Read-only.

        .. py:attribute:: mipmap

            Determines whether mipmaps (http://en.wikipedia.org/wiki/Mipmap) are 
            generated for this node. Setting this to :keyword:`True` improves the quality
            of minified nodes. Depending on the graphics card in use, turning on mipmaps
            may cause a performance hit for every image change. Read-only.

        .. py:method:: getBitmap() -> Bitmap

            Returns a copy of the bitmap that the node contains.

        .. py:method:: getOrigVertexCoords() -> list

            Returns the unwarped coordinate of all vertices as a list of lists.

        .. py:method:: getWarpedVertexCoords() -> list

            Returnes the current coordinate of all vertices as a list of lists.

        .. py:method:: setEffect(FXNode)

            Sets an :py:class:`FXNode` that modifies the way the node looks.

        .. py:method:: setWarpedVertexCoords(grid)

            Changes the current coordinates of all vertices. :py:attr:`grid` is a list of
            lists of coordinate tuples.

    .. autoclass:: RectNode

        A rectangle that can be filled.

        .. py:method:: __init__([pos, size, angle])

        .. py:attribute:: angle

            The angle that the rectangle is rotated to in radians. 0 is
            unchanged, 3.14 is upside-down. The rectangle is rotated around it's
            center.

        .. py:attribute:: pos

            The position of the top left corner of the rectangle.

        .. py:attribute:: size

        .. py:attribute:: texcoords

    .. autoclass:: SoundNode

        A sound played from a file.

        .. py:method:: __init__([href, loop=False, volume=1.0])

        .. py:attribute:: duration

            The duration of the sound file in milliseconds. Read-only.

        .. py:attribute:: href

            The source filename of the sound.

        .. py:attribute:: loop

            Whether to start the sound again when it has ended. Read-only.

        .. py:attribute:: volume

            Audio playback volume for this sound. 0 is silence, 1 passes media
            file volume through unchanged. Values higher than 1 can be used to
            amplify sound if the sound file doesn't use the complete dynamic
            range.

        .. py:method:: getAudioCodec() -> string

            Returns the codec used as a string such as :samp:`"mp2"`.

        .. py:method:: getAudioSampleRate() -> int

            Returns the sample rate in samples per second (for example, 44100).

        .. py:method:: getCurTime()

            Returns milliseconds of playback time since audio start.

        .. py:method:: getNumAudioChannels() -> int

            Returns the number of channels. 2 for stereo, etc.

        .. py:method:: pause()

            Stops audio playback but doesn't close the object. The playback
            cursor stays at the same position.

        .. py:method:: play()

            Starts audio playback.

        .. py:method:: seekToTime(time)

            Moves the playback cursor to the time given in milliseconds.

        .. py:method:: setEOFCallback(pyfunc)

            Sets a python callable to be invoked when the audio reaches end of file.

        .. py:method:: stop()

            Stops audio playback. Closes the object and 'rewinds' the playback cursor.

    .. autoclass:: VectorNode

        Base class for all nodes that draw geometrical primitives. All vector nodes 
        support configurable stroke width. Strokes can be filled either with a solid 
        color (:py:attr:`color`) or with a texture loaded from a file 
        (:py:attr:`texhref`) or taken from a bitmap object (:py:meth:`setBitmap`).

        .. py:method:: __init__([color="FFFFFF", strokewidth=1, texhref, blendmode="blend"])

        .. py:attribute:: blendmode

            The method of compositing the node with the nodes under
            it. Valid values are :py:const:`blend`, :py:const:`add`, :py:const:`min` 
            and :py:const:`max`. For :py:const:`min` and :py:const:`max`
            blend modes, opacity is ignored.

        .. py:attribute:: color

            The color of the strokes in standard html color notation:
            :samp:`"FF0000"` is red, :samp:`"00FF00"` green, etc.

        .. py:attribute:: strokewidth

            The width of the strokes in the vector. For lines, this is the line
            width. For rectangles, it is the width of the outline, etc.

        .. py:attribute:: texhref

            An image file to use as a texture for the node.

        .. py:method:: setBitmap(bitmap)

            Sets a bitmap to use as a texture. Sets :attr:`texhref` to an empty
            string.

    .. autoclass:: VideoNode

        Video nodes display a video file. Video formats and codecs supported
        are all formats that ffmpeg/libavcodec supports. Usage is described throughly
        in the libavg wiki: https://www.libavg.de/wiki/index.php/Videos.

        .. py:method:: __init__([href, loop=False, threaded=True, fps, queuelength=8, volume=1.0])

        .. py:attribute:: fps

            The nominal frames per second the object should display at. Read-only.

        .. py:attribute:: href

            The source filename of the video.

        .. py:attribute:: loop

            Whether to start the video again when it has ended. Read-only.

        .. py:attribute:: queuelength

            The length of the decoder queue in video frames. This is the number of
            frames that can be decoded before the first one is displayed. A higher
            number increases memory consumption but also resilience against
            data source latency. Can only be set at node construction. Can't be set
            if :samp:`threaded=False`, since there is no queue in that case.

        .. py:attribute:: threaded

            Whether to use separate threads to decode the video. The default is
            :keyword:`True`. Setting this attribute to :keyword:`False` makes seeking much quicker.
            On the other hand, it also disables audio and prevents libavg from 
            distributing the CPU load over several cores of a multi-core computer.

        .. py:attribute:: volume

            Audio playback volume for this video. 0 is silence, 1 passes media
            file volume through unchanged. Values higher than 1 can be used to
            amplify sound if the sound file doesn't use the complete dynamic
            range. If there is no audio track, volume is ignored.

        .. py:method:: getAudioCodec() -> string

            Returns the audio codec used as a string such as :samp:`"mp2"`.

        .. py:method:: getAudioSampleRate() -> int

            Returns the sample rate in samples per second (for example, 44100).

        .. py:method:: getBitrate() -> int

            Returns the number of bits in the file per second.

        .. py:method:: getCurFrame() -> int

            Returns the index of the video frame currently playing.

        .. py:method:: getCurTime()

            Returns milliseconds of playback time since video start.

        .. py:method:: getDuration() -> int

            Returns the duration of the video in milliseconds.

        .. py:method:: getNumFrames() -> int

            Returns the number of frames in the video.

        .. py:method:: getNumAudioChannels() -> int 

            Returns the number of audio channels. 2 for stereo, etc.

        .. py:method:: getNumFramesQueued() -> int

            Returns the number of frames already decoded and waiting for playback.

        .. py:method:: getStreamPixelFormat() -> string

            Returns the pixel format of the video file as a string. Possible
            pixel formats are described in
            http://cekirdek.pardus.org.tr/~ismail/ffmpeg-docs/ffmpeg-r_2libavutil_2avutil_8h.html

        .. py:method:: getVideoCodec() -> string

            Returns the video codec used as a string such as :samp:`"mpeg4"`.

        .. py:method:: hasAlpha() -> bool

            Returns :keyword:`True` if the video contains an alpha (transparency) channel.
            Throws an exception if the video has not been opened yet.

        .. py:method:: hasAudio() -> bool

            Returns :keyword:`True` if the video contains an audio stream. Throws an
            exception if the video has not been opened yet.

        .. py:method:: pause()

            Stops video playback but doesn't close the object. The playback
            cursor stays at the same position and the decoder queues remain full.

        .. py:method:: play()

            Starts video playback.

        .. py:method:: seekToFrame(num)

            Moves the playback cursor to the frame given.

        .. py:method:: seekToTime(millisecs)

            Moves the playback cursor to the time given.

        .. py:method:: setEOFCallback(pyfunc)

            Sets a python callable to be invoked when the video reaches end of file.
        
        .. py:method:: stop()

            Stops video playback. Closes the file, 'rewinds' the playback
            cursor and clears the decoder queues.

    .. autoclass:: VisibleNode

        Base class for all elements in the avg tree that have a visual representation.
        All nodes except those derived from :py:class:`FXNode` are VisibleNodes.

        .. py:method:: __init__([oncursormove, oncursorup, uncursordown, oncursorover, oncursorout, active=True, sensitive=True, opacity=1.0, parent])

            :param string oncursormove:

                Name of python function to call when a cursor moves.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param string oncursorup:

                Name of python function to call when an up event occurs.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param string oncursordown:

                Name of python function to call when a down event occurs.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param string oncursorover:

                Name of python function to call when a cursor enters the node.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param string oncursorout:

                Name of python function to call when a cursor leaves the node.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param DivNode parent:

                A :py:class:`DivNode` that the newly constructed Node should be appended
                to.

        .. py:attribute:: active

            If this attribute is true, the node behaves as usual. If not, it
            is neither drawn nor does it react to events.

        .. py:attribute:: opacity

            A measure of the node's transparency. 0.0 is completely
            transparent, 1.0 is completely opaque. Opacity is relative to
            the parent node's opacity.

        .. py:attribute:: sensitive

            A node only reacts to events if sensitive is true.

        .. py:method:: getAbsPos(relpos) -> Point2D

            Transforms a position in coordinates relative to the node to a
            position in window coordinates.

        .. py:method:: getElementByPos(pos) -> Node

            Returns the topmost child node that is at the position given. :py:attr:`pos`
            is in coordinates relative to the called node. The algorithm used
            is the same as the cursor hit test algorithm used for events.

        .. py:method:: getParent() -> Node

            Returns the container (:py:class:`AVGNode` or :py:class:`DivNode`) the node
            is in. For the root node, returns None.

        .. py:method:: getRelPos(abspos) -> Point2D

            Transforms a position in window coordinates to a position
            in coordinates relative to the node.

        .. py:method:: releaseEventCapture(cursorid=-1)

            Restores normal cursor event handling after a call to 
            :py:func:`setEventCapture()`. :py:attr:`cursorid` is the id of the
            cursor to release. If :py:attr:`cursorid` is not given, the mouse cursor is
            used.

        .. py:method:: setEventCapture(cursorid=-1)

            Sets up event capturing so that cursor events are sent to this node
            regardless of the cursor position. cursorid is optional; if left out,
            the mouse cursor is captured. If not, events from a specific tracker
            cursor are captured. The event propagates to the capturing node's
            parent normally. This function is useful for the
            implementation of user interface elements such as scroll bars. Only one
            node can capture a cursor at any one time. Normal operation can
            be restored by calling :py:func:`releaseEventCapture()`.
        
        .. py:method:: setEventHandler(type, source, pyfunc)

            Sets a callback function that is invoked whenever an event of the
            specified type from the specified source occurs. This function is
            similar to the event handler node attributes (e.g. oncursordown).
            It is more specific since it takes the event source as a parameter
            and allows the use of any python callable as callback function.
            
            :param type:
            
                One of the event types :py:const:`KEYUP`, :py:const:`KEYDOWN`, 
                :py:const:`CURSORMOTION`, :py:const:`CURSORUP`, :py:const:`CURSORDOWN`, 
                :py:const:`CURSOROVER`, :py:const:`CURSOROUT`, :py:const:`RESIZE` or 
                :py:const:`QUIT`.

            :param source:

                :py:const:`MOUSE` for mouse events, :py:const:`TOUCH` for multitouch touch
                events, :py:const:`TRACK` for multitouch track events or other tracking,
                :py:const:`NONE` for keyboard events. Sources can be or'ed together to 
                set a handler for several sources at once.

            :param pyfunc:

                The python callable to invoke.

        .. py:method:: unlink(kill)

            Removes a node from it's parent container. Equivalent to
            :samp:`node.getParent().removeChild(node.getParent().indexOf(node))`, 
            except that if the node has no parent, unlink does nothing. Normally, unlink
            moves the node's textures back to the CPU and preserves event handlers.
            If :samp:`kill=True`, this step is skipped. Event handlers are reset, all
            textures are deleted and the href is reset to empty in this case,
            saving some time and making sure there are no references to the node
            left on the libavg side. :py:attr:`kill` should always be set to 
            :keyword:`True` if the node will not be used after the unlink.

    .. autoclass:: WordsNode

        A words node displays formatted text. All
        properties are set in pixels. International and multi-byte character
        sets are fully supported. Words nodes use UTF-8 to encode international 
        characters (use python unicode strings for this).
        
        The pos attribute of a words node is the
        logical top left of the first character for left-aligned text. For
        centered and right-aligned text, it is the top center and right of the
        first line, respectively. For latin text, the logical top usually
        corresponds to the height of the ascender. There may be cases where
        portions of the text are rendered to the left of or above the logical position,
        for instance when italics are used.

        Words nodes are rendered using pango internally. 

        .. py:method:: __init__([font="arial", variant="", text="", color="FFFFFF", fontsize=15, indent=0, linespacing=-1, alignment="left", wrapmode="word", justify=False, rawtextmode=False, letterspacing=0, hint=True])

        .. py:attribute:: alignment

            The paragraph alignment. Possible values are :py:const:`left`,
            :py:const:`center` and :py:const:`right`.

        .. py:attribute:: color

            The color of the text in standard html color notation: FF0000 is red, 
            00FF00 green, etc.

        .. py:attribute:: font 

            The family name of the truetype font to use. Font files can either be 
            installed in the system, be in a :file:`fonts/` subdirectory of the current
            directory, or be in a directory specified using :py:meth:`addFontDir`. To
            figure out which fonts and variants are available, use the 
            :command:`avg_showfonts.py` utility.

        .. py:attribute:: fontsize

            The font size in pixels. Fractional sizes are supported.

        .. py:attribute:: hint

            Whether or not hinting (http://en.wikipedia.org/wiki/Font_hinting)
            should be used when rendering the text. Unfortunately, this setting
            does not override the fontconfig settings in
            :file:`/etc/fonts/conf.d/*-hinting.conf` or other fontconfig configuration
            files.

        .. py:attribute:: indent

            The indentation of the first line of the text.

        .. py:attribute:: justify

            Whether each complete line should be stretched to fill
            the entire width of the layout. Default is false.

        .. py:attribute:: letterspacing

            The amount of space between the idividual glyphs of the text in
            pixels, with 0 being standard spacing and negative values indicating
            packed text (less letter spacing than normal). Only active when text
            attribute markup is not being used.

        .. py:attribute:: linespacing

            The number of pixels between different lines of a paragraph. Setting this to
            :samp:`-1` results in default line spacing.

        .. py:attribute:: rawtextmode

            Sets whether the text should be parsed to apply markup (:keyword:`False`,
            default) or interpreted as raw string (:keyword:`True`).

        .. py:attribute:: text 

            The string to display. If the node is created using xml, this is either the
            text attribute of the words node or the content of the words
            node itself. In the second case, the string can be formatted
            using the pango text attribute markup language described at
            http://library.gnome.org/devel/pango/unstable/PangoMarkupFormat.html.
            Markup can also be used if the text is set using the python attribute.

            Markup parsing can be turned on or off with :py:attr:`rawtextmode` attribute.

        .. py:attribute:: variant

            The variant (:samp:`bold`, :samp:`italic`, etc.) of the font to use.

        .. py:attribute:: wrapmode

            Controls at which points text can wrap to the next line. Possible values are
            :py:const:`word` (split lines at the nearest whitespace, default), 
            :py:const:`char` (split at any position, ignoring word breaks) and 
            :py:const:`wordchar` (split at word boundaries but fall back
            to char mode if there is no free space for a full word).

        .. py:method:: getCharIndexFromPos(pos) -> int

            Returns the index of the character at the coordinates :py:attr:`pos`, or
            :keyword:`None` if there is no character at that position. :py:attr:`pos`
            is relative to the node position.
            Formatting markup such as <b> or <i> is treated as zero chars,
            <br/> is treated as one char. To get the text matched to this
            use :py:meth:`getTextAsDisplayed`.

        .. py:method:: getGlyphPos(i) -> Point2D

            Returns the position of the glyph at character index :py:attr:`i` in the 
            layout. The position is in pixels relative to the words
            node. Formatting markup such as <b> or <i> is treated as zero chars,
            <br/> is treated as one char.

        .. py:method:: getGlyphSize(i) -> Point2D

            Returns the size in pixels of the glyph at character index :py:attr:`i` in
            the layout. Formatting markup such 
            as <b> or <i> is treated as zero chars, <br/> is treated as one char.

        .. py:method:: getLineExtents(line) -> Point2D

            Returns the width and height of the specified line in pixels.
        
        .. py:method:: getNumLines()

            Returns the number of lines in the layout.

        .. py:method:: getTextAsDisplayed

            Returns the text without text attribute markup language. <br/>
            is replaced by \\n.

        .. py:classmethod:: addFontDir

            Adds a directory to be searched for fonts.
            May only be called before :py:meth:`Player.play`.

        .. py:classmethod:: getFontFamilies() -> list

            Returns a list of strings containing all font names available.

        .. py:classmethod:: getFontVariants(fontname) -> list

            Returns a list of available variants (:samp:`Regular`, :samp:`Bold`, etc.)
            of a font.

