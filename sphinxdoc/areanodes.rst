Area Nodes
==========

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: AVGNode AreaNode CameraNode CanvasNode DivNode ImageNode Node RasterNode SoundNode VideoNode WordsNode
        :parts: 1

    .. autoclass:: AreaNode([x, y, pos, width, height, size, angle, pivot])

        Base class for elements in the avg tree that define an area on the screen.
        Responsible for coordinate transformations and event handling. See 
        http://www.libavg.de/wiki/ProgrammersGuide/CoordinateSystems
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
            relative to its parent node. 

        .. py:attribute:: y

            This is the vertical position of the node's reference point 
            relative to its parent node. 

        .. py:attribute:: pos

            This is the position of the node's reference point 
            relative to its parent node. 

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


    .. autoclass:: AVGNode()

        Root node of an onscreen avg tree. Defines the properties of the display
        and handles key press events. The AVGNode's width and height define the
        coordinate system for the display and are the default for the window
        size used (i.e. by default, the coordinate system is pixel-based).


    .. autoclass:: CameraControl

        Camera controls are camera configuration parameters like brightness and white
        balance. A CameraControl object contains information about the supported maximum
        and minimum values as well as defaults for a specific control.

        .. py:attribute:: controlName

            String which tells which control is meant. Read-only.

        .. py:attribute:: default

        .. py:attribute:: max

        .. py:attribute:: min

    .. autoclass:: CameraImageFormat

        CameraImageFormat objects contain information about a supported
        image format of a camera.

        .. py:attribute:: framerates

            List of supported frame rates in images per second for that image format as
            floats. Read-only.

        .. py:attribute:: pixelFormat

            String which tells about the pixel format (see :py:class:`Bitmap`). Read-only.

        .. py:attribute:: size

            A point which represents the resolution in width and height. Read-only.

    .. autoclass:: CameraInfo

        CameraInfo objects contain data about camera capabilities. The data can be used
        to create create objects of class :py:class:`CameraNode`. The unique value to 
        identify the camera is stored in :py:attr:`device`, whereas :py:attr:`driver` 
        tells which driver is used to call the camera itself. Information about supported
        camera :py:attr:`controls` or :py:attr:`imageFormats` are stored
        in two separate lists.

        .. py:attribute:: controls

            List of :py:class:`CameraControl` objects with all possible controls for
            that camera. Read-only.

        .. py:attribute:: device

            String which contains the unique id of the camera. Read-only.

        .. py:attribute:: driver

            String which contains the name of the driver. Read-only.

        .. py:attribute:: imageFormats

            List of :py:class:`CameraImageFormat` objects with all possible image
            formats for that camera. Read-only.

    .. autoclass:: CameraNode([driver='firewire', device="", unit=-1, fw800=False, framerate=15, capturewidth=640, captureheight=480, pixelformat="RGB", brightness, exposure, sharpness, saturation, camgamma, shutter, gain, strobeduration])

        A node that displays the image of a camera. The attributes correspond to the 
        camera properties in .avgtrackerrc and are explained under
        http://www.libavg.de/wiki/ProgrammersGuide/Tracker. An easy way to find the 
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

        .. py:classmethod:: getCamerasInfos()

            Returns a list of :py:class:`CameraInfo` objects, one for for each connected 
            camera.

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

            .. deprecated:: 1.7
                Seldom used, error-prone and slow.

            The directory that the media files for the children of this node are
            in. Relative mediadirs are taken to mean subdirectories of the parent node's 
            mediadir.

        .. py:method:: getNumChildren() -> int

            Returns the number of immediate children that this div contains.

        .. py:method:: getChild(i) -> Node

            Returns the child at index :py:attr:`i`.

        .. py:method:: appendChild(node)

            Adds a new child to the container behind the last existing child.

        .. py:method:: insertChildBefore(newNode, oldChild)

            Adds a new child to the container before the existing node
            :py:attr:`oldChild`. In z-order, the new child ist behind the old one.

        .. py:method:: insertChildAfter(newNode, oldChild)

            Adds a new child to the container after the existing node
            :py:attr:`oldChild`. In z-order, the new child ist in front of the old one.

        .. py:method:: insertChild(node, i)

            Adds a new child to the container at index :py:attr:`i`.

        .. py:method:: removeChild(node)

            Removes the child given by :py:attr:`node` from the div. Note that as long as
            other references to the node exist, the node is not deleted.

        .. py:method:: removeChild(i)

            Removes the child at index :py:attr:`i` from the div. Note that as long a`
            other references to the node exist, the node is not deleted.

        .. py:method:: reorderChild(oldIndex, newIndex)

            Moves the child at :py:attr:`oldIndex` so it is at :py:attr:`newIndex`. This
            function can be used to change the order in which the children are drawn.

        .. py:method:: reorderChild(node, newPos)

            Moves the child :py:attr:`node` so it is at index :py:attr:`newPos`. This
            function can be used to change the order in which the children are drawn.

        .. py:method:: indexOf(node)

            Returns the index of the node given. Throws an exception if :py:attr:`node`
            isn't a child of the :py:class:`DivNode`.

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

    .. autoclass:: RasterNode([maxtilewidth, maxtileheight, blendmode, mipmap, maskhref, maskpos, masksize, gamma, contrast, intensity])

        Base class for all nodes that have a direct 2D raster representation.
        This includes Image, Word, Camera, and Video nodes. The base class implements
        color controls (:py:attr:`contrast`, :py:attr:`intensity`, :py:attr:`gamma`),
        alpha masks (:py:attr:`maskhref`, :py:attr:`maskpos`, :py:attr:`masksize`), 
        several blend modes that define how compositing is done and mipmapping support.

        Any Raster Node can have a GPU-based effect added to it by using 
        :py:meth:`setEffect`.

        In addition, RasterNodes can be warped. By default, a RasterNode is rectangular.
        However, it can be subdivided into a grid of reference points using 
        :py:attr:`maxtilewidth` and :py:attr:`maxtileheight`. The position of each of 
        these points can be changed with :py:meth:`getOrigVertexCoords`, 
        :py:meth:`getWarpedVertexCoords` and :py:meth:`setWarpedVertexCoords`, 
        yielding arbitrary shapes.
        
        .. py:attribute:: blendmode

            .. deprecated:: 1.7
                
                The min and max blend modes will be removed.

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
            increase, lower values decrease the brightness. In all cases, black and
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
            outside the mask, the border pixels of the mask are taken. Note that the
            maskpos is an offset from the top left of the node, even for 
            :py:class:`WordsNode` objects that have :py:attr:`alignment`
            :py:const:`Center` or :py:const:`Right`.


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
            may cause an extreme performance hit for every image change or have no
            performance cost at all. Read-only.

        .. py:method:: getOrigVertexCoords() -> list

            Returns the unwarped coordinate of all vertices as a list of lists.

        .. py:method:: getWarpedVertexCoords() -> list

            Returnes the current coordinate of all vertices as a list of lists.

        .. py:method:: setEffect(FXNode)

            Attaches an :py:class:`FXNode` to the node that modifies how it looks.

        .. py:method:: setWarpedVertexCoords(grid)

            Changes the current coordinates of all vertices. :py:attr:`grid` is a list of
            lists of coordinate tuples. :py:meth:`setWarpedVertexCoords` can only be called if
            the node is in a renderable state. This means that :py:meth:`Player.play()` must have
            been called and the node must be inserted in a Canvas. There must also be something to
            render (for instance, :py:meth:`play()` must be called before 
            :py:meth:`setWarpedVertexCoords` in the case of a :py:class:`CameraNode`). The grid
            submitted is lost if the node loses renderable status.

    .. autoclass:: SoundNode([href, loop=False, volume=1.0])

        A sound played from a file.

        **Messages:**

            To get this message, call :py:meth:`Publisher.subscribe`.

            .. py:method:: Node.END_OF_FILE()
            
                Emitted when the end of the audio stream has been reached.

        .. py:attribute:: duration

            The duration of the sound file in milliseconds. Some file formats don't store
            valid durations; in this case, 0 is returned. Read-only.

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

        .. py:method:: getCurTime() -> time

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

            .. deprecated:: 1.8
                Use the message interface instead.

            Sets a python callable to be invoked when the audio reaches end of file.

        .. py:method:: stop()

            Stops audio playback. Closes the object and 'rewinds' the playback cursor.

    .. autoclass:: VideoNode([href, loop=False, threaded=True, fps, queuelength=8, volume=1.0, accelerated=True, enablesound=True])

        Video nodes display a video file. Video formats and codecs supported
        are all formats that ffmpeg/libavcodec supports. Usage is described thoroughly
        in the libavg wiki: https://www.libavg.de/wiki/ProgrammersGuide/VideoNode.

        **Messages:**

            To get this message, call :py:meth:`Publisher.subscribe`.

            .. py:method:: Node.END_OF_FILE()
            
                Emitted when the end of the video stream has been reached.

        .. py:attribute:: accelerated

            On construction, set to :py:const:`True` if hardware acceleration should be 
            used to decode this video. Later queries of the attribute return 
            :py:const:`True` if acceleration is actually being used. Read-only.

        .. py:attribute:: enablesound

            On construction, set to :py:const:`True` if any audio present in the video
            file should be played back as well. A value of :py:const:`False` ignores 
            audio and just plays a silent video. 

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
            data source latency (i.e. hiccups during disk reads). Can only be set at node
            construction. Can't be set if :samp:`threaded=False`, since there is no queue
            in that case.

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

            Returns the audio codec used as a string such as :samp:`mp2`.

        .. py:method:: getAudioSampleRate() -> int

            Returns the sample rate in samples per second (for example, 44100).

        .. py:method:: getBitrate() -> int

            Returns the number of bits in the file per second.

        .. py:method:: getContainerFormat() -> string

            Returns the video file format. This is a string such as :samp:`avi` or 
            :samp:`mpeg`.

        .. py:method:: getCurFrame() -> int

            Returns the index of the video frame currently playing.

        .. py:method:: getCurTime() -> long

            Returns milliseconds of playback time since video start.

        .. py:method:: getDuration() -> int

            Returns the duration of the video in milliseconds. Some file formats don't 
            store valid durations; in this case, 0 is returned. Read-only.

        .. py:method:: getNumFrames() -> int

            Returns the number of frames in the video.

        .. py:method:: getNumAudioChannels() -> int 

            Returns the number of audio channels. 2 for stereo, etc.

        .. py:method:: getNumFramesQueued() -> int

            Returns the number of frames already decoded and waiting for playback.

        .. py:method:: getStreamPixelFormat() -> string

            Returns the pixel format of the video file as a string. Possible
            pixel formats are described in
            http://ffmpeg.mplayerhq.hu/doxygen/trunk/pixfmt_8h.html#60883d4958a60b91661e97027a85072a

        .. py:method:: getVideoCodec() -> string

            Returns the video codec used as a string such as :samp:`mpeg4`.

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

            .. deprecated:: 1.8
                Use the message interface instead.

            Sets a python callable to be invoked when the video reaches end of file.
        
        .. py:method:: stop()

            Stops video playback. Closes the file, 'rewinds' the playback
            cursor and clears the decoder queues.

        .. py:classmethod:: getVideoAccelConfig() -> enum

            Returns either :py:const:`NO_ACCELERATION` if the current configuration does
            not support hardware-accelerated video decoding or :py:const:`VDPAU` if VDPAU
            can be used to decode videos.

    .. autoclass:: WordsNode([fontstyle=None, font="sans", variant="", text="", color="FFFFFF", fontsize=15, indent=0, linespacing=-1, alignment="left", wrapmode="word", justify=False, rawtextmode=False, letterspacing=0, aagamma=1, hint=True])

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

        .. py:attribute:: aagamma

            Defines a gamma-correction value for the alpha (transparency) of the text
            rendered. Using this attibute, it is possible to fine-tune the text
            antialiasing and make sure rendering is smooth.

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

        .. py:attribute:: fontstyle

            A :py:class:`FontStyle` object that encapsulates all font attributes of the
            node. As a constructor parameter, this attribute sets the default attributes
            and other constructor arguments can override these. If set during
            :py:class:`WordsNode` use, all relevant attributes are set to the new values.

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

            The string to display. The string can be formatted using the pango text 
            attribute markup language described at
            http://developer.gnome.org/pango/unstable/PangoMarkupFormat.html. In addition,
            the :samp:`<br/>` tag is supported for paragraph breaks.

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
        
        .. py:method:: getNumLines() -> int

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

