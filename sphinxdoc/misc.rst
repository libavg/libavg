Misc. Classes
=============

.. automodule:: libavg.avg
    :no-members:

    .. autoclass:: Bitmap

        Class representing a rectangular set of pixels. Bitmaps can be obtained
        from any RasterNode. For nodes of type Image, the current bitmap can be
        set as well.

        The layout of the pixels in the bitmap is determined by it's pixel format.
        Possible return values are :py:const:`B5G6R5`, :py:const:`B8G8R8`, 
        :py:const:`B8G8R8A8`, :py:const:`B8G8R8X8`, :py:const:`A8B8G8R8`, 
        :py:const:`X8B8G8R8`, :py:const:`R5G6B5`, :py:const:`R8G8B8`, 
        :py:const:`R8G8B8A8`, :py:const:`R8G8B8X8`, :py:const:`A8R8G8B8`,
        :py:const:`X8R8G8B8`, :py:const:`I8` and :py:const:`YCbCr422`.

        .. py:method:: __init__(size, pixelFormat, name)

            Creates an uninitialized bitmap of the given size and pixel format.
            :py:attr:`name` is a name to be used in debug output.

        .. py:method:: __init__(bitmap)

            Creates a copy of an already existing bitmap.

        .. py:method:: __init__(fileName)

            Loads an image file from disk and returns it as bitmap object.

        .. py:method:: save(filename)

            Writes the image to a file. File format is determined using the
            extension. Any file format supported by ImageMagick 
            (http://www.imagemagick.org) can be used.

        .. py:method:: getSize() -> Point2D

            Returns the size of the image in pixels.

        .. py:method:: getFormat()

            Returns the bitmap's pixel format.
            
        .. py:method:: getPixels() -> string

            Returns the raw pixel data in the bitmap as a python string. This
            method can be used to interface to the python imaging library PIL
            (http://www.pythonware.com/products/pil/).

        .. py:method:: setPixels(pixels)

            Changes the raw pixel data in the bitmap. Doesn't change dimensions 
            or pixel format. Can be used to interface to the python imaging
            library PIL (http://www.pythonware.com/products/pil/).
            
            :param string pixels: Image data.

        .. py:method:: getPixel(pos) -> (r,g,b,a)

            Returns one image pixel als a color tuple. This should only be used
            for single pixels, as it is very slow.

        .. py:method:: subtract(otherbitmap) -> bmp

            Subtracts two bitmaps and returns the result. Used mainly to compare
            test images with the intended results (along with :py:meth:`getAvg` and
            :py:meth:`getStdDev`).

        .. py:method:: getAvg() -> float

            Returns the average of all bitmap pixels.

        .. py:method:: getStdDev() -> float

            Returns the standard deviation of all bitmap pixels.

        .. py:method:: getName() -> string

    .. autoclass:: ConradRelais

        Interface to one or more conrad relais cards connected to a serial port.
        Per card, up to eight 220V devices can be connected.

        .. deprecated:: 1.5

            This is unsupported. I don't think you can even buy the hardware anymore.

        .. py:method:: ConradRelais(AVGPlayer, port)

            Opens a connection to the relais card(s) connected to a serial port.

            :param port: 
                
                The port the device is connected to. The actual device file
                opened is :file:`/dev/ttyS<port>`.

        .. py:method:: getNumCards() -> int

            Returns the number of cards connected to the serial port.

        .. py:method:: set(card, index, value)

            Sets or resets one of the relais.

            :param card: Zero-based index of the card to address.
            :param index: Zero-based index of the relais on the card.
            :param value: 
            
                Whether to set (:keyword:`True`) or reset (:keyword:`False`)
                the relais.

        .. py:method:: get(card, index) -> value

            Returns the state of one of the relais.

            :param card: Zero-based index of the card to address.
            :param index: Zero-based index of the relais on the card.

    .. autoclass:: Logger



    .. autoclass:: ParPort

        Used for low-level control of the parallel port's data, status and control
        lines. Linux only.

        .. deprecated:: 1.5

            This is unsupported and probably buggy.

        .. py:method:: init(devicename)

            Opens a parallel port.

            :param devicename: 
            
                Device filename to use. If :py:attr:`devicename` is an empty
                string, :file:`/dev/parport0` is used as device name.

        .. py:method:: setControlLine(line, value) -> bool

            Sets or clears one of the control lines.

            :param line: 
            
                Which control line to modify. Possible values for line
                are :py:const:`CONTROL_STROBE`, :py:const:`CONTROL_AUTOFD`, 
                :py:const:`CONTROL_INITu` and :py:const:`CONTROL_SELECT`.

            :param value: 
            
                Whether to set (:keyword:`True`) or clear (:keyword:`False`) the line.

            :return: 
            
                :keyword:`True` if the value was set successfully, :keyword:`False`
                otherwise.

        .. py:method:: getStatusLine(line)

            Returns the value of one of the parallel port status lines.

            :param line: 
            
                Which status line to query. Possible values for line are
                :py:const:`STATUS_ERROR`, :py:const:`STATUS_SELECT`, 
                :py:const:`STATUS_PAPEROUT`, :py:const:`STATUS_ACK` and
                :py:const:`STATUS_BUSY`.

            :return: :keyword:`True` if the line is set.

        .. py:method:: setDataLines(lines)

            Sets data lines. 

            :param lines: 
            
                The lines to set. Constants to used for these lines are 
                :py:const:`PARPORTDATA0` - :py:const:`PARPORTDATA7`.
                Several of these constants can be or'ed together to set several lines. 
                The lines not mentioned in the parameter are left unchanged.

            :return: 
            
                :keyword:`True` if the lines were set, :keyword:`False` otherwise.

        .. py:method:: clearDataLines(lines)

            Clears data lines.

            :param lines: 
            
                The lines to clear. Constants to used for these lines are 
                :py:const:`PARPORTDATA0` - :py:const:`PARPORTDATA7`. Several of these 
                constants can be or'ed together to set several lines. The lines not 
                mentioned in the parameter are left unchanged.

            :return:
                
                :keyword:`True` if the lines were cleared, :keyword:`False` 
                otherwise.

        .. py:method:: setAllDataLines(lines)

            Changes the value of all data lines.
        
            :param lines: The lines to set. Constants to used for these
            lines are :py:const:`PARPORTDATA0` - :py:const:`PARPORTDATA7`. Several of 
            these constants can be or'ed together to set several lines. The lines not 
            mentioned in the parameter are cleared.

            :return: :keyword:`True` if the lines were set, :keyword:`False` otherwise.

        .. py:method:: isAvailable()

            Returns :keyword:`True` if the parallel port has been opened successfully, 
            :keyword:`False` otherwise.


    .. autoclass:: Point2D

        A point in 2D space. Supports most arithmetic operations on vectors. The 
        operators :py:attr:`+`, :py:attr:`-`, :py:attr:`==` and :py:attr:`\!=` are 
        defined for two :py:class:`Point2D` parameters. Unary :py:attr:`-` (negation)
        is defined as well. :py:class:`Point2D` objects can also be multiplied and 
        divided by a scalar.
        
        :py:class:`Point2D` implicitly converts from and to 2-element float tuples and 
        lists, so in most cases you can use one of these types whenever a point is needed.

        .. py:method:: __init__()

            Creates a point initialized to (0,0).

        .. py:method:: __init__(x,y)

        .. py:attribute:: x

        .. py:attribute:: y

        .. py:method:: getNormalized() -> Point2D

            Returns a normalized version of the point with the same angle but a
            norm of one. Throws an exception if called on Point2D(0,0).

        .. py:method:: getNorm() -> float

            Returns the euclidian norm of the point, that is sqrt(x*x+y*y).

        .. py:method:: getRotated(angle) -> Point2D

            Return the position of point rotated around the origin.
    
        .. py:method:: getRotated(angle, pivot) -> Point2D

            Return the position of point rotated around :py:attr:`pivot`.

        .. py:classmethod:: fromPolar(angle, radius) -> Point2D

            Converts polar to cartesian coordinates. angle is in radians with 0
            being the positive x axis. Angle is clockwise (assuming that y points
            downward).

    .. autoclass:: TestHelper

    .. autofunction:: pointInPolygon
