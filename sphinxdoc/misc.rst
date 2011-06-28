Misc. Classes
=============

.. automodule:: libavg.avg
    :no-members:

    .. autoclass:: Bitmap

        Class representing a rectangular set of pixels in CPU memory. Bitmaps can be 
        obtained from any :py:class:`RasterNode` or loaded from disk. For nodes of type 
        :py:class:`ImageNode`, the current bitmap can be set as well.

        The layout of the pixels in the bitmap is described by it's pixel format.
        The names for pixel format constants are confusing. They try to follow logic,
        but it's a bit elusive: In many cases, each component is described by a single 
        letter indicating it's role in the pixel and a number indicating the number of 
        bits used for this component.
        Components are named in the order they appear in memory. In the cases where
        the name doesn't follow this logic, reasons for the name are historical or
        by convention or something, and anyway, most pixel formats are only used 
        internally and users usually won't come into contact with them. The pixel formats
        are:

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

        .. py:method:: __init__(fileName)

            Loads an image file from disk and returns it as bitmap object.

        .. py:method:: getAvg() -> float

            Returns the average of all bitmap pixels.

        .. py:method:: getChannelAvg(channel) -> float

            Returns the average of one of the bitmap color channels (red, green or blue). Used for
            automatic tests.

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

        .. py:method:: getSize() -> Point2D

            Returns the size of the image in pixels.

        .. py:method:: getStdDev() -> float

            Returns the standard deviation of all bitmap pixels.

        .. py:method:: save(filename)

            Writes the image to a file. File format is determined using the
            extension. Any file format supported by ImageMagick 
            (http://www.imagemagick.org/) can be used.

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

        .. py:method:: loadBitmap(fileName, callback)
        
            Asynchronously loads a file into a Bitmap. The provided callback is invoked
            with a Bitmap instance as argument in case of a successful load or with a
            RuntimeError exception instance in case of failure.

        .. py:classmethod:: get() -> BitmapManager

            This method gives access to the BitmapManager instance.

    .. autoclass:: ConradRelais(AVGPlayer, port)

        Interface to one or more conrad relais cards connected to a serial port.
        Per card, up to eight 220V devices can be connected.

        .. deprecated:: 1.5

            This is unsupported. I don't think you can even buy the hardware anymore.

        :param port: 
            
            The port the device is connected to. The actual device file
            opened is :file:`/dev/ttyS<port>`.

        .. py:method:: get(card, index) -> value

            Returns the state of one of the relais.

            :param card: Zero-based index of the card to address.
            :param index: Zero-based index of the relais on the card.

        .. py:method:: getNumCards() -> int

            Returns the number of cards connected to the serial port.

        .. py:method:: set(card, index, value)

            Sets or resets one of the relais.

            :param card: Zero-based index of the card to address.
            :param index: Zero-based index of the relais on the card.
            :param value: 
            
                Whether to set (:py:const:`True`) or reset (:py:const:`False`)
                the relais.

    .. autoclass:: Logger

        Interface to the logger used by the avg player. Enables the setting
        of different logging categories. Categories can be set either by calling
        Logger.setCategories or by setting the :envvar:`AVG_LOG_CATEGORIES` environment
        variable. When set through the environment, log categories are separated by
        colons. In bash syntax:

        .. code-block:: bash

            export AVG_LOG_CATEGORIES=ERROR:WARNING:CONFIG:PROFILE
        
        Log categories are:
       
        :py:const:`NONE`
            No logging except for errors.
        :py:const:`BLTS`
            Display subsystem logging. Useful for timing/performance measurements.
        :py:const:`PROFILE`
            Outputs performance statistics on player termination.
        :py:const:`PROFILE_LATEFRAMES`
            Outputs performance statistics whenever a frame is displayed late.
        :py:const:`PROFILE_VIDEO`
            Outputs performance statistics for video decoding.
        :py:const:`EVENTS`
            Outputs basic event data.
        :py:const:`EVENTS2`
            Outputs all event data available.
        :py:const:`CONFIG`
            Outputs configuration data.
        :py:const:`WARNING`
            Outputs warning messages.
        :py:const:`ERROR`
            Outputs error messages. Can't be turned off.
        :py:const:`MEMORY`
            Outputs open/close information whenever a media file is accessed.
        :py:const:`APP`
            Reserved for application-level messages issued by python code.
        :py:const:`PLUGIN`
            Messages generated by loading plugins.
        :py:const:`PLAYER`
            General libavg playback messages.

        Default categories are :py:const:`ERROR`, :py:const:`WARNING` and 
        :py:const:`APP`. 
        
        Log output is sent to the console (:file:`stderr`).  Each log entry contains 
        the time the message was written, the category of the entry and the message 
        itself.
        
        .. py:method:: popCategories

            Pops the current set of categories from the internal stack, restoring
            the state when the corresponding push was called.

        .. py:method:: pushCategories

            Pushes the current set of categories on an internal stack. Useful
            for saving and restoring the logging state so it can be changed
            for a short amount of time.

        .. py:method:: setCategories(categories)

            Sets the types of messages that should be logged. :py:attr:`categories` is
            an or'ed sequence of categories.

        .. py:method:: trace(category, message)

            Logs message to the log if category is active.

            :param category: 
            
                One of the categories listed above. Should in be APP for messages
                logged from python.

            :param message: The log message string.

        .. py:classmethod:: get

            This method gives access to the logger. There is only one instance.

    .. autoclass:: ParPort(devicename)

        Used for low-level control of the parallel port's data, status and control
        lines. Linux only. The parallel port device is opened on construction.

        .. deprecated:: 1.5

            This is unsupported and probably buggy.

        :param devicename: 
        
            Device filename to use. If :py:attr:`devicename` is an empty
            string, :file:`/dev/parport0` is used as device name.

        .. py:method:: clearDataLines(lines)

            Clears data lines.

            :param lines: 
            
                The lines to clear. Constants to used for these lines are 
                :py:const:`PARPORTDATA0` - :py:const:`PARPORTDATA7`. Several of these 
                constants can be or'ed together to set several lines. The lines not 
                mentioned in the parameter are left unchanged.

            :return:
                
                :py:const:`True` if the lines were cleared, :py:const:`False` 
                otherwise.

        .. py:method:: getStatusLine(line)

            Returns the value of one of the parallel port status lines.

            :param line: 
            
                Which status line to query. Possible values for line are
                :py:const:`STATUS_ERROR`, :py:const:`STATUS_SELECT`, 
                :py:const:`STATUS_PAPEROUT`, :py:const:`STATUS_ACK` and
                :py:const:`STATUS_BUSY`.

            :return: :py:const:`True` if the line is set.

        .. py:method:: isAvailable()

            Returns :py:const:`True` if the parallel port has been opened successfully, 
            :py:const:`False` otherwise.

        .. py:method:: setAllDataLines(lines)

            Changes the value of all data lines.
        
            :param lines: 
            
                The lines to set. Constants to used for these
                lines are :py:const:`PARPORTDATA0` - :py:const:`PARPORTDATA7`. Several of 
                these constants can be or'ed together to set several lines. The lines not 
                mentioned in the parameter are cleared.

            :return: :py:const:`True` if the lines were set, :py:const:`False` otherwise.

        .. py:method:: setControlLine(line, value) -> bool

            Sets or clears one of the control lines.

            :param line: 
            
                Which control line to modify. Possible values for line
                are :py:const:`CONTROL_STROBE`, :py:const:`CONTROL_AUTOFD`, 
                :py:const:`CONTROL_INITu` and :py:const:`CONTROL_SELECT`.

            :param value: 
            
                Whether to set (:py:const:`True`) or clear (:py:const:`False`) the line.

            :return: 
            
                :py:const:`True` if the value was set successfully, :py:const:`False`
                otherwise.

        .. py:method:: setDataLines(lines)

            Sets data lines. 

            :param lines: 
            
                The lines to set. Constants to used for these lines are 
                :py:const:`PARPORTDATA0` - :py:const:`PARPORTDATA7`.
                Several of these constants can be or'ed together to set several lines. 
                The lines not mentioned in the parameter are left unchanged.

            :return: 
            
                :py:const:`True` if the lines were set, :py:const:`False` otherwise.

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

        .. py:method:: isInf() -> bool

            Returns :py:const:`True` if one of the components is infinite.

        .. py:method:: isNaN() -> bool

            Returns :py:const:`True` if one of the components is Not a Number.

        .. py:classmethod:: fromPolar(angle, radius) -> Point2D

            Converts polar to cartesian coordinates. :py:attr:`angle` is in radians with 0
            being the positive x axis. Angle is clockwise (assuming that y points
            downward).

    .. autoclass:: TestHelper

        Miscellaneous routines used by tests. Not intended for normal application usage.

    .. autoclass:: VideoWriter(canvas, filename, [framerate=30, qmin=3, qmax=5, synctoplayback=True])

        Class that writes the contents of a canvas to disk as a video file. The videos
        are written as motion jpeg-encoded mov files. Opening, writing and closing the
        video file is asynchronous to normal playback.

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

        .. py:method:: stop()

            Ends the recording and writes the rest of the file to disk. Note that this is
            asynchronous to normal playback. If you need to immediately re-open the
            video file (e.g. for playback in a video node), destroy the python object 
            first. This waits for sync.

    .. autofunction:: getMemoryUsage() -> int

        Returns the amount of memory used by the application in bytes. More
        precisely, this function returns the resident set size of the process
        in bytes. This does not include shared libraries or memory paged out to
        disk.

    .. autofunction:: pointInPolygon(point, poly) -> bool

        Checks if a point is inside a polygon.

        :param Point2D point: Point to check.
        :param poly: List of points which constitute a polygon to check against.
        :returns: :py:const:`True` if point is inside, :py:const:`False` otherwise.

