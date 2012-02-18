Effect Nodes
============

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: BlurFXNode ChromaKeyFXNode HueSatFXNode NullFXNode ShadowFXNode
        :parts: 1

    .. autoclass:: BlurFXNode(radius=1.0)

        Blurs the node it is applied to. Corresponds to the Gaussian Blur effect in 
        Photoshop.

        .. py:attribute:: radius

            The width of the blur. This corresponds to the radius parameter of
            photoshop.

    .. autoclass:: ChromaKeyFXNode

        A high-quality realtime chroma key (greenscreen or bluescreen) effect. Can be 
        configured by using the :program:`avg_chromakey.py` script. The effect is carried 
        out in the HSL colorspace (http://en.wikipedia.org/wiki/HSL_and_HSV).

        .. py:attribute:: color

            The color to key out. Pixels of this and similar colors are made transparent.

        .. py:attribute:: erosion

            Removes single non-keyed-out pixels in larger transparent areas. Values > 1
            remove larger areas. Useful for removing camera noise.

        .. py:attribute:: htolerance

            Hue tolerance for the key color. 

        .. py:attribute:: ltolerance

            Lightness tolerance for the key color. 

        .. py:attribute:: softness

            :py:attr:`softness` > 0 causes pixels with a color close to the keyed-out
            colors to become partially transparent. Greater values increase this effect.

        .. py:attribute:: spillthreshold

            Often, people in greenscreen studios aquire a greenish tint. Spill removal 
            works against this by desaturating pixels that are close to the key color.
            Larger values cause more desaturation.

        .. py:attribute:: stolerance

            Saturation tolerance for the key color. 

    .. autoclass:: FXNode

        Base class for GPU-based effects. These effects can be added to any 
        :py:class:`Node` by calling :py:meth:`Node:setEffect`.

    .. autoclass:: HueSatFXNode(hue=0.0, saturation=1.0, lightness=0.0, colorize=False)

        Color correction filter that works by converting pixels to the hsl color space 
        (http://en.wikipedia.org/wiki/HSL_and_HSV), changing the values there and 
        converting back to rgb. Can be used to change hue,
        saturation and lightness of a node. Corresponds to the Photoshop Hue/Saturation 
        dialog.

        .. py:attribute:: hue

           Used to get/set the color angle. Ranges depend on the current
           colorize mode and are the same as in Photoshop.

        .. py:attribute:: lightness
           
           Set :py:attr:`lightness` offset. Adds a per pixel offset in brightness.

        .. py:attribute:: saturation
           
           Set :py:attr:`saturation` of Node. Valid values are the same as in Photoshop.

        .. py:attribute:: colorize

           If :py:attr:`colorize` is :py:const:`True`, all colors will be tinted 
           according to the current :py:attr:`hue` value. Otherwise, :py:attr:`hue` is
           treated as a difference value.

    .. autoclass:: InvertFXNode

        Color Invert Effect. Inverts the brightness of nodes that it is attached to.

    .. autoclass:: NullFXNode

        Do-nothing effect. Exists primarily as aid in debugging libavg.

    .. autoclass:: ShadowFXNode(offset=(0,0), radius=1.0, opacity=1.0, color="FFFFFF")

        Adds a shadow behind the node.

        .. py:attribute:: offset

            An adjustment to the shadow's position behind the node in pixels.        

        .. py:attribute:: radius

            The blur radius of the shadow.

        .. py:attribute:: opacity

            The opacity of the shadow.

        .. py:attribute:: color

            The shadow color.


