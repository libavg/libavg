Misc. Classes
=============

.. automodule:: libavg.avg
    :no-members:

    .. autoclass:: Bitmap

        Class representing a rectangular set of pixels in CPU memory. Bitmaps can be 
        obtained from any :py:class:`RasterNode` or loaded from disk. For nodes of type 
        :py:class:`ImageNode`, the current bitmap can be set as well. In general, huge
        Bitmaps (e.g. width>65536) are supported as long as they fit into memory.

        The layout of the pixels in the bitmap is described by its pixel format.
        The names for pixel format constants are confusing. They try to follow logic,
        but it's a bit elusive: In many cases, each component is described by a single 
        letter indicating the component's role in the pixel and a number indicating the 
        number of bits used for this component.
        Components are named in the order they appear in memory. In the cases where
        the name doesn't follow this logic, reasons for the name are usually historical or
        by convention.
        You can receive a complete list of all supported pixel formats by calling
        :py:func:`avg.getSupportedPixelFormats()`.
        The pixel formats are:

            * :py:const:`B5G6R5`: 16 bits per pixel blue, green, red components.
            * :py:const:`B8G8R8`: 24 bits per pixel blue, green, red components.
            * :py:const:`B8G8R8A8`: 32 bits per pixel: blue, green, red and an 
              alpha (opacity) component.
            * :py:const:`B8G8R8X8`: 32 bits per pixel, with the last byte unused.
            * :py:const:`A8B8G8R8`
            * :py:const:`X8B8G8R8`
            * :py:const:`R5G6B5`
            * :py:const:`R8G8B8`
            * :py:const:`R8G8B8A8`
            * :py:const:`R8G8B8X8`
            * :py:const:`A8R8G8B8`
            * :py:const:`X8R8G8B8`
            * :py:const:`I8`: 8 bits of greyscale intensity.
            * :py:const:`I16`: 16 bits of greyscale intensity.
            * :py:const:`A8`: 8 bits of transparency information.
            * :py:const:`YCbCr411`: Interleaved YCbCr: Y Y Cb Y Y Cr. 
              Effectively 12 bits per pixel. Output format of some cameras.
            * :py:const:`YCbCr422`: Interleaved YCbCr: Cb Y Cr Y.
              Effectively 16 bits per pixel. Output format of some cameras.
            * :py:const:`YUYV422`: Like YCbCr422, but grey values come first, so the 
              order is Y Cb Y Cr.
            * :py:const:`YCbCr420p`: Not a valid pixel format for a single bitmap, but 
              still a description of planar bitmap coding. Signifies separate 
              bitmaps for Y, Cb and Cr components, with Cb and Cr half as big in both x
              and y dimensions. This is mpeg YCbCr, where the color components have 
              values from 16...235. Used by many video formats, including mpeg.
            * :py:const:`YCbCrJ420p`: Same as YCbCr420p, but this is the jpeg version 
              with component values in the range 0...255. Used in video as well, for 
              instance in motion jpeg encoding.
            * :py:const:`YCbCrA420p`: YCbCr420p with an additional alpha (transparency)
              bitmap at full resolution. Used in flash video with transparency.
            * :py:const:`BAYER8`: Bayer pattern. This is raw camera sensor data with
              an unspecified pixel order. The other :py:const:`BAYER_XXX` constants 
              specify differing camera sensor arrangements.
            * :py:const:`BAYER8_RGGB`
            * :py:const:`BAYER8_GBRG`
            * :py:const:`BAYER8_GRBG`
            * :py:const:`BAYER8_BGGR`
            * :py:const:`R32G32B32A32F`: 32 bits per channel float RGBA.
            * :py:const:`I32F`: 32 bits per channel greyscale intensity.

        .. py:method:: __init__(size, pixelFormat, name)

            Creates an uninitialized bitmap of the given size and pixel format.
            :py:attr:`name` is a name to be used in debug output.

        .. py:method:: __init__(bitmap)

            Creates a copy of an already existing bitmap.

        .. py:method:: __init__(bitmap, tlPos, brPos)

            Returns a rectangle inside an existing bitmap as a new bitmap. Note that the
            pixels are not copied and write operations will therefore effect the 
            original bitmap as well.

        .. py:method:: __init__(fileName)

            Loads an image file from disk and returns it as bitmap object.

        .. py:method:: blt(srcBmp, pos)

            Copies the pixels of srcBmp into the current bitmap at pos. 

        .. py:method:: getAvg() -> float

            Returns the average of all bitmap pixels.

        .. py:method:: getChannelAvg(channel) -> float

            Returns the average of one of the bitmap color channels (red, green or blue).
            Used for automatic tests.

        .. py:method:: getFormat()

            Returns the bitmap's pixel format.
            
        .. py:method:: getName() -> string

        .. py:method:: getPixel(pos) -> (r,g,b,a)

            Returns one image pixel as a color tuple. This should only be used
            for single pixels, as it is very slow.

        .. py:method:: getPixels() -> string

            Returns the raw pixel data in the bitmap as a python string. This
            method can be used to interface to the python imaging library PIL
            (http://www.pythonware.com/products/pil/).

        .. py:method:: getResized(newSize) -> Bitmap

            Returns a new bitmap that is a resized version of the original.

        .. py:method:: getSize() -> Point2D

            Returns the size of the image in pixels.

        .. py:method:: getStdDev() -> float

            Returns the standard deviation of all bitmap pixels.

        .. py:method:: save(filename)

            Writes the image to a file. File format is determined using the
            extension. Supported file types are those supported by gdk-pixbuf. This 
            includes at least png, jpeg, gif, tiff and xpixmaps. 

        .. py:method:: setPixels(pixels)

            Changes the raw pixel data in the bitmap. Doesn't change dimensions 
            or pixel format. Can be used to interface to the python imaging
            library PIL (http://www.pythonware.com/products/pil/).
            
            :param string pixels: Image data.

        .. py:method:: subtract(otherbitmap) -> bmp

            Subtracts two bitmaps and returns the result. Used mainly to compare
            test images with the intended results (along with :py:meth:`getAvg` and
            :py:meth:`getStdDev`).

    .. autoclass:: BitmapManager

        (EXPERIMENTAL) Singleton class that allow an asynchronous load of bitmaps.
        The instance is accessed by :py:meth:`get`.

        .. py:method:: loadBitmap(fileName, callback, pixelformat=NO_PIXELFORMAT)
        
            Asynchronously loads a file into a Bitmap. The provided callback is invoked
            with a Bitmap instance as argument in case of a successful load or with a
            RuntimeError exception instance in case of failure. The optional parameter
            :py:attr:`pixelformat` can be used to convert the bitmap to a specific format
            asynchronously as well.

        .. py:classmethod:: get() -> BitmapManager

            This method gives access to the BitmapManager instance.
        
        .. py:method:: setNumThreads(numThreads)

            Sets the number of threads used to load bitmaps. The default is a single
            thread. This should generally be less than the number of logical cores 
            available.

    .. autoclass:: CubicSpline(controlpoints)

        Class that generates a smooth curve between control points using cubic 
        spline-based interpolation. For an introduction on spline interpolation,
        see http://en.wikipedia.org/wiki/Spline_interpolation.

        :param controlpoints:

        A list of 2D coordinates. The x coordinates must be in increasing order.

        .. py:method:: interpolate(x) -> y

        Takes an x coordinate and delivers a corresponding y coordinate. 

    
    .. autoclass:: FontStyle(font="sans", variant="", color="FFFFFF", fontsize=15, indent=0, linespacing=-1, alignment="left", wrapmode="word", justify=False, letterspacing=0, aagamma=1, hint=True)

        A :py:class:`FontStyle` object encapsulates all configurable font attributes in a
        :py:class:`WordsNode`. It provides a way to set all relevant attributes 
        (:py:attr:`font`, :py:attr:`fontsize`, etc.) in one line of code. The attributes
        correspond to the :py:class:`WordsNode` attributes; refer to the 
        :py:class:`WordsNode` reference for descriptions.    


    .. autoclass:: Logger

        An python interface to libavg's logger.
        It can be accessed as follows:

        .. code-block:: python

            import libavg
            libavg.logger



        **Logging**

        .. py:method:: log(message, category, severity)

            Logs a message if category is active or severity is at least 
            :py:const:`ERROR`.

            :param category: 

                One of the categories listed or a custom category. Defaults to
                :py:const:`Logger.Category.APP`.

            :param severity:
                One of the severities listed. Defaults to :py:const:`Logger.Severity.INFO`.

            :param message: The log message string.

        .. py:method:: critical(msg, category)

            Shortcut to :py:meth:`log` with **severity=** :py:const:`Logger.Severity.CRIT`.

        .. py:method:: error(msg, category)

            Shortcut to :py:meth:`log` with **severity=** :py:const:`Logger.Severity.ERR`.

        .. py:method:: warning(msg, category)

            Shortcut to :py:meth:`log` with **severity=** :py:const:`Logger.Severity.WARN`.

        .. py:method:: info(msg, category)

            Shortcut to :py:meth:`log` with **severity=** :py:const:`Logger.Severity.INFO`.

        .. py:method:: debug(msg, category)

            Shortcut to :py:meth:`log` with **severity=** :py:const:`Logger.Severity.DBG`.

       **Configuration**

        .. py:method:: addSink(logger)

            Add a python logger object to libavg's logging handlers.
            The python logger gets the key `category` as an "extra" kwarg, useful for
            formatting the output.

        .. py:method:: removeSink(logger)

            Remove a previously added logger. It will not receive any messages dispatched
            by the logger annymore. It's safe to call the function even if the logger is
            not present.

        .. py:method:: removeStdLogSink()

            Remove libavg console printing sink.

            Setting :envvar:`AVG_LOG_OMIT_STDERR` as EnvironmentVar has the same effect.

        .. py:method:: configureCategory(category, severity)

            Assign a severity  to a given category.
            category is either a custom string or a :py:const:`Logger.Category`
            severity has to be one of :py:const:`Logger.Severity`, or it will default to
            :py:const:`Logger.Severity.None`

        .. py:method:: getCategories()

            Returns a dict with **category** as key and **severity** as value


        The Logger can also be configured using :envvar:`AVG_LOG_CATEGORIES` with
        the format:

        .. code-block:: bash

            CATEGORY[:SEVERITY] [CATEGORY[:SEVERITY]]...

        Thus it is possible to set a severity level for every category.
        If no severity is given, the default severity is used.
        (refer to: :py:meth:`setDefaultSeverity`)
        If the category does not exist, it will be created.

        .. code-block:: bash

            export AVG_LOG_CATEGORIES="CONFIG:INFO APP:DBG PROFILE:DBG MEMORY"

        Default categories are :py:const:`NONE`, :py:const:`APP` and
        :py:const:`DEPREC`. They are set to the defaultSeverity.



       **Categories:**

            :py:const:`APP`
                Reserved for application-level messages issued by python code.
            :py:const:`CONFIG`
                Outputs configuration data.
            :py:const:`DEPREC`
                Deprecation
                Messages that warn of functionality that will be removed from libavg
                in the future.
            :py:const:`EVENTS`
                Outputs basic event data.
            :py:const:`MEMORY`
                Outputs open/close information whenever a media file is accessed.
            :py:const:`NONE`
                Outputs everything that has not been categorized.
            :py:const:`PROFILE`
                Outputs performance statistics on player termination.
            :py:const:`PROFILE_V`
                Outputs performance statistics for video decoding.
            :py:const:`PLUGIN`
                Messages generated by loading plugins.
            :py:const:`PLAYER`
                General libavg playback messages.
            :py:const:`SHADER`
                Shader compiler messages.

       **Severities**

            :py:const:`CRIT` Critical

            :py:const:`ERR` Error

            :py:const:`WARN` Warning

            :py:const:`INFO` Info

            :py:const:`DBG` Debug

            :py:const:`NONE` None


    .. autoclass:: Point2D([x,y=(0,0)])

        A point in 2D space. Supports most arithmetic operations on vectors. The 
        operators :py:attr:`+`, :py:attr:`-`, :py:attr:`==` and :py:attr:`\!=` are 
        defined for two :py:class:`Point2D` parameters. Unary :py:attr:`-` (negation)
        is defined as well. :py:class:`Point2D` objects can also be multiplied and 
        divided by a scalar.
        
        :py:class:`Point2D` implicitly converts from and to 2-element float tuples and 
        lists, so in most cases you can use one of these types whenever a point is needed.

        .. py:attribute:: x

        .. py:attribute:: y

        .. py:classmethod:: angle(p1, p2) -> float

            Returns the angle between the two vectors :py:attr:`p1` and :py:attr:`p2`
            in the range 0 and 2*pi, with 0 being the positive x axis. Angles run
            clockwise.

        .. py:method:: getAngle() -> float

            Returns the direction of the vector as an angle between pi and -pi, with
            0 being the positive x axis. Angles run clockwise.

        .. py:method:: getNorm() -> float

            Returns the euclidian norm of the point, that is sqrt(x*x+y*y).

        .. py:method:: getNormalized() -> Point2D

            Returns a normalized version of the point with the same angle but a
            norm of one. Throws an exception if called on Point2D(0,0).

        .. py:method:: getRotated(angle) -> Point2D

            Return the position of point rotated around the origin.

        .. py:method:: getRotated(angle, pivot) -> Point2D

            Return the position of point rotated around :py:attr:`pivot`.

        .. py:classmethod:: fromPolar(angle, radius) -> Point2D

            Converts polar to cartesian coordinates. :py:attr:`angle` is in radians with 0
            being the positive x axis. Angle is clockwise (assuming that y points
            downward).

        .. py:method isInPolygon(poly) -> bool

            Checks if the point is inside a polygon.

            :param poly: List of points which constitute a polygon to check against.
            :returns: :py:const:`True` if point is inside, :py:const:`False` otherwise.


    .. autoclass:: SVG(filename, [unescapeIllustratorIDs=False])

        :py:class:`SVG` objects load and parse an svg file and render images from it.
        svg (*Scalable Vector Graphics*, see http://en.wikipedia.org/wiki/Svg) files are 
        xml-based and contain two-dimensional vector graphics. They can be created with 
        editors such as Adobe Illustrator and Inkscape. :py:class:`SVG` objects can render
        elements in the file to bitmaps and create image nodes from elements in the file. 
        Since the files contain vector graphics, the elements can be scaled to any size 
        when rendering without loss of resolution.

        :param filename: The name of the file to load.
        
        :param unescapeIllustratorIDs: 
        
            If this is :py:const:`True`, the file is assumed to be generated by Adobe
            Illustrator. Illustrator mangles element names to create IDs in the svg file. 
            Setting this parameter to :py:const:`True` allows these element names to be
            passed as IDs.

        .. py:method:: renderElement(elementID, [size | scale=1]) -> Bitmap

            Renders an element to a :py:class:`Bitmap`. Either :py:attr:`scale` or 
            :py:attr:`size` may be given. :py:attr:`size` is the size of the bitmap.
            :py:attr:`scale` is a factor to scale the native bitmap size with.

        .. py:method:: createImageNode(elementID, nodeAttrs, [size | scale=1]) -> Node

            Convenience method that calls :py:meth:`renderElement` to render a bitmap
            and then creates an image node that displays that bitmap. :py:attr:`nodeAttrs`
            is a dictionary containing constructor parameters for the node.

        .. py:method:: getElementPos(elementID) -> Point2D

            Returns the position of an element.

        .. py:method:: getElementSize(elementID) -> Point2D

            Returns the original size of an element.


    .. autoclass:: TestHelper

        Miscellaneous routines used by tests. Not intended for normal application usage.


    .. autoclass:: VersionInfo

        Exposes version data, including the specs of the builder.

        .. py:attribute:: full
        
        Full string containing a compact form of branch and revision number (if the
        build doesn't come from an exported tree)
        
        .. py:attribute:: release
        
        String representation in the form `major.minor.micro`
        
        .. py:attribute:: major
        
        Integer component of the release version (major)

        .. py:attribute:: minor
        
        Integer component of the release version (minor)

        .. py:attribute:: micro
        
        Integer component of the release version (micro)
        
        .. py:attribute:: revision
        
        Revision number, if applicable, or 0
        
        .. py:attribute:: branchurl
        
        Full URL path that represents the branch root, if applicable, or empty string
        
        .. py:attribute:: builder
        
        String representation in the form of `user@hostname machinespecs`
        
        .. py:attribute:: buildtime
        
        ISO timestamp representation of the build


    .. autoclass:: VideoWriter(canvas, filename, [framerate=30, qmin=3, qmax=5, synctoplayback=True])

        Class that writes the contents of a canvas to disk as a video file. The videos
        are written as motion jpeg-encoded mov files. Writing commences immediately upon 
        object construction and continues until :py:meth:`stop` is called. 
        :py:meth:`pause` and :py:meth:`play` can be used to pause and resume writing.
        
        The VideoWriter is built for high performance: Opening, writing and closing the
        video file is asynchronous to normal playback. Writing full HD videos of
        offscreen canvasses to disk should cost virtually no time on the main thread of 
        execution for an Intel Core-class processor with a graphics card that supports 
        shaders.

        :param canvas:

            A libavg canvas used as source of the video.

        .. py:attribute:: filename

            The name of the file to write to. Read-only.

        .. py:attribute:: framerate

            The speed of the encoded video in frames per second. This is used for two 
            purposes. First, it determines the nominal playback speed of the video that is
            encoded in the file. Second, if :py:attr:`synctoplayback` is 
            :py:const:`False`, the :py:class:`VideoWriter` will also use the 
            :py:attr:`framerate` value as the actual number of frames per second to 
            write. Read-only.

        .. py:attribute:: qmin

        .. py:attribute:: qmax

            :py:attr:`qmin` and :py:attr:`qmax` specify the minimum and maximum encoding 
            quality to use. :samp:`qmin = qmax = 1` give maximum quality at maximum file
            size. :samp:`qmin=3` and :samp:`qmax=5` (the default) give a good quality and
            a smaller file.  Read-only.

        .. py:attribute:: synctoplayback

            If :py:attr:`synctoplayback` is :py:const:`True` (the default), each frame
            played back in the canvas will be written to disk. This makes a lot of sense
            in combination with :py:meth:`Canvas.registerCameraNode()`. If not, 
            :py:attr:`framerate` is used to determine which frames to write to disk. For 
            instance, if :py:attr:`synctoplayback` is :py:const:`False`,
            :py:attr:`framerate` is 25 and the player is running at 60 fps, one movie
            frame will be written for each 2.5 frames of playback. The actual, not the
            nominal playback speed is used in this case. Read-only.

        .. py:method:: pause()

            Temporarily stops recording.

        .. py:method:: play()

            Resumes recording after a call to :py:meth:`pause`. :py:meth:`play` doesn't
            need to be called after construction of the :py:class:`VideoWriter` - writing
            commences immediately.

        .. py:method:: stop()

            Ends the recording and writes the rest of the file to disk. Note that this is
            asynchronous to normal playback. If you need to immediately re-open the
            video file (e.g. for playback in a video node), destroy the python object 
            first. This waits for sync.

    .. autofunction:: validateXml(xmlString, schemaString, xmlName, schemaName)

        Validates an xml string using a schema. Throws an exception if the xml doesn't
        conform to the schema. 


.. automodule:: libavg.statemachine
    :no-members:

    .. autoclass:: StateMachine(name, startState)

        A generic state machine, useful for user interface and other states. Consists of
        a set of states (represented by strings) and possible transitions between the 
        states. The :py:class:`StateMachine` can be configured to invoke callbacks at
        specific transitions and when entering or leaving a state. All callbacks are 
        optional. State changes can be logged for debugging purposes.

        State machines are initialized by calling :py:meth:`addState` for each
        possible state after constructing it.

        :param String name:
        
            A name for the state machine to be used in debugging output.

        :param String startState:

        .. py:attribute:: state

            The current state the :py:class:`StateMachine` is in. States are strings.

        .. py:method:: addState(state, transitions, [enterFunc=None, leaveFunc=None])

            Adds a state to the :py:class:`StateMachine`. Must be called before the first
            changeState.

            :param String state: The name of the state to add.
            :param transitions: 
            
                This parameter can be either a list of destination states or a dict of
                destinationState: callable pairs. The callables are invoked whenever the
                corresponding state change happens. If :py:meth:`transitions` is a list,
                no state change callbacks are registered.

            :param enterFunc: A callable to invoke whenever the state is entered.
            :param leaveFunc: A callable to invoke whenever the state is left.

        .. py:method:: changeState(newState)

            Changes the state. This includes calling the leave callback for the current
            state, actually changing the state, calling the transition callback and
            calling the enter callback for the new state.

            Raises a :py:class:`RuntimeError` if :py:attr:`newState` is not a valid state
            or if there is no transition defined from the current state to 
            :py:attr:`newState`.

        .. py:method:: dump()

            Prints all states and transitions to the console.

        .. py:method:: makeDiagram(imageFName, [showMethods=False])

            Dumps a graph of the state machine to an image file using dot. graphviz must
            be installed and in the path for this to work. Very useful for debugging. If
            :py:attr:`showMethods` is true, names of enter, leave and transition
            methods are included in the diagram.

        .. py:method:: traceChanges(trace)

            If :py:attr:`trace` is set to :py:const:`True`, all state changes are dumped
            to the console.


.. automodule:: libavg.persist
    :no-members:

    .. autoclass:: Persist(storeFile, initialData[, validator=lambda v: True, autoCommit=False])

        A general purpose persistent object.
        Its state is defined in the :py:attr:`data` attribute and pickled
        from/to a store file.
        

        :param string storeFile:
        
            Full path of the store file that is used to store and retrieve a
            serialized version of the data.

        :param initialData:

            A pickle-able object that is assigned to the :py:attr:`data` attribute
            when no file store exists or when the store file is corrupted.

        :param callable validator:

            An optional callable that receives the object state as soon it's
            de-pickled from the store file. If the validator call doesn't return True,
            the object state is restored to the provided `initialData`.

        :param bool autoCommit:

            If True, the :py:attr:`commit` method is registered as an `atexit` function.
            
        .. py:attribute:: data

            State of the persistent object.

        .. py:attribute:: storeFile

            Returns the full path of the store file.

        .. py:method:: commit()

            Dumps the contents of the :py:attr:`data` attribute to the store file.


    .. autoclass:: UserPersistentData(appName, fileName, initialData[, validator=lambda v: True, autoCommit=False])

        A :py:class:`Persist` subclass that sets up an OS-independent path for
        the store file.
        Under posix-compliant OSes is `$HOME/.avg/<appName>/<fileName>.pkl`
        Under Windows is `%APPDATA%\Avg\<appName>/<fileName>.pkl`

        :param string appName:
        
            Name of the application. This string is used to compose the full path to
            the file store and it creates a namespace (directory) for multiple files
            for the same application.

        :param string fileName:
        
            Name of the file store file. `.pkl` will be added as extension.

