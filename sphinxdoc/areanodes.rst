Area Nodes
==========

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: AVGNode AreaNode CameraNode CanvasNode DivNode ImageNode Node PanoImageNode RasterNode SoundNode VideoNode VisibleNode WordsNode
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

        .. py:method:: getBitmap() -> Bitmap

            Returns a copy of the last camera frame.

        .. py:method:: getWhitebalanceU() -> int

        .. py:method:: getWhitebalanceV() -> int

        .. py:method:: isAvailable() -> bool

            Returns :py:const:`True` if there is a working device that can deliver images
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

    .. autoclass:: DivNode([crop=False, elementoutlinecolor, mediadir])

        A div node is a node that groups other nodes logically and visually.
        Its position is used as point of origin for the coordinates
        of its child nodes. Its extents can be used to clip the children if crop is set 
        to :py:const:`True`. Its opacity is used as base opacity for the child nodes' 
        opacities. The children of a div node are drawn in the order they are found
        in the avg file, so the first one is below all others in z-order.
       
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

            Returns the node's effective mediadir by traversing the node
            hierarchy up to the root node.

    .. autoclass:: ImageNode([href, compression])

        A static raster image on the screen. The content of an ImageNode can be loaded
        from a file. It can also come from a :py:class:`Bitmap` object or from an 
        :py:class:`OffscreenCanvas`. Alpha channels of the image files are used as
        transparency information.

        .. py:attribute:: compression

            The texture compression used for this image. Currently, :py:const:`none`
            and :py:const:`B5G6R5` are supported. :py:const:`B5G6R5` causes the bitmap 
            to be compressed to 16 bit per pixel on load and is only valid if the source 
            is a filename. Read-only.

        .. py:attribute:: href

            In the standard case, this is the source filename of the image. To use a
            bitmap as source, call setBitmap().  To use an offscreen canvas as source, 
            use the :samp:`canvas:` protocol: :samp:`href="canvas:{id}"`.

        .. py:method:: getBitmap() -> Bitmap

            Returns a copy of the bitmap that the node contains.

        .. py:method:: setBitmap(bitmap)

            Sets a bitmap to use as content for the ImageNode. Sets href to an empty 
            string.

    .. autoclass:: PanoImageNode([href, sensorwidth, sensorheight, focallength, rotation])

        A panorama image displayed in cylindrical projection.

        .. deprecated:: 1.5
            This is unsupported and probably buggy.

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

    .. autoclass:: RasterNode([maxtilewidth, maxtileheight, blendmode, mipmap, maskhref, maskpos, masksize, gamma, contrast, intensity])

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
            generated for this node. Setting this to :py:const:`True` improves the quality
            of minified nodes. Depending on the graphics card in use, turning on mipmaps
            may cause a performance hit for every image change. Read-only.

        .. py:method:: getOrigVertexCoords() -> list

            Returns the unwarped coordinate of all vertices as a list of lists.

        .. py:method:: getWarpedVertexCoords() -> list

            Returnes the current coordinate of all vertices as a list of lists.

        .. py:method:: setEffect(FXNode)

            Sets an :py:class:`FXNode` that modifies the way the node looks.

        .. py:method:: setWarpedVertexCoords(grid)

            Changes the current coordinates of all vertices. :py:attr:`grid` is a list of
            lists of coordinate tuples.

    .. autoclass:: SoundNode([href, loop=False, volume=1.0])

        A sound played from a file.

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

    .. autoclass:: VideoNode([href, loop=False, threaded=True, fps, queuelength=8, volume=1.0])

        Video nodes display a video file. Video formats and codecs supported
        are all formats that ffmpeg/libavcodec supports. Usage is described throughly
        in the libavg wiki: https://www.libavg.de/wiki/index.php/Videos.

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
            :py:const:`True`. Setting this attribute to :py:const:`False` makes seeking
            much quicker. On the other hand, it also disables audio and prevents libavg 
            from distributing the CPU load over several cores of a multi-core computer.

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

            Returns :py:const:`True` if the video contains an alpha (transparency) 
            channel. Throws an exception if the video has not been opened yet.

        .. py:method:: hasAudio() -> bool

            Returns :py:const:`True` if the video contains an audio stream. Throws an
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

    .. autoclass:: WordsNode([font="arial", variant="", text="", color="FFFFFF", fontsize=15, indent=0, linespacing=-1, alignment="left", wrapmode="word", justify=False, rawtextmode=False, letterspacing=0, hint=True])

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

            Sets whether the text should be parsed to apply markup (:py:const:`False`,
            default) or interpreted as raw string (:py:const:`True`).

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
            :py:const:`None` if there is no character at that position. :py:attr:`pos`
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

