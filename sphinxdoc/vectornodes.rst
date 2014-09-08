Vector Nodes
============

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: CircleNode CurveNode FilledVectorNode LineNode MeshNode Node PolygonNode PolyLineNode RectNode VectorNode libavg.geom.Arc libavg.geom.PieSlice libavg.geom.RoundedRect
        :parts: 1

    .. autoclass:: CircleNode([r=1, texcoord1=0, texcoord2=1])

        A circle. The reference point for a circle is its center.

        .. py:attribute:: r

            The radius of the circle in pixels.

        .. py:attribute:: texcoord1

        .. py:attribute:: texcoord2

            floats used as u texture coordinates for the circle border. The coordinates
            wrap around the circle once.

    .. autoclass:: CurveNode([pos1, pos2, pos3, pos4, texcoord1, texcoord2])

        A cubic bezier curve (`<http://en.wikipedia.org/wiki/Bezier_curve>`_). 
        :py:attr:`pos1` and :py:attr:`pos4` are the two endpoints of the curve. 
        :py:attr:`pos2` and :py:attr:`pos3` are control points.

        .. py:method:: getPtOnCurve(t) -> pos

            Returns a point on the curve. Which point is determined by the value of t.
            If :samp:`t=0`, returns :py:attr:`pos1`. :samp:`t=1` returns :py:attr:`pos4`,
            and values in between return the points in between.

        .. py:attribute:: length

            Returns an approximation of the length of the curve (ro).

        .. py:attribute:: pos1

        .. py:attribute:: pos2

        .. py:attribute:: pos3

        .. py:attribute:: pos4

        .. py:attribute:: texcoord1

        .. py:attribute:: texcoord2

            floats that signify the u axis texture coordinates.
    
    .. autoclass:: FilledVectorNode([filltexhref, fillopacity=0, fillcolor="FFFFFF", filltexcoord1=Point2D(0,0), filltexcoord2=Point2D(1,1)])

        Base class for vector nodes which have a filled area and a border. The area can
        be filled either with a solid color (:py:attr:`fillcolor`) or with a texture
        loaded from a file (:py:attr:`filltexhref`) or taken from a bitmap object 
        (:py:meth:`setFillBitmap`).

        .. py:attribute:: fillcolor

        .. py:attribute:: fillopacity

        .. py:attribute:: filltexcoord1

        .. py:attribute:: filltexcoord2

        .. py:attribute:: filltexhref

            An image file to use as a texture for the area of the node.

        .. py:method:: setFillBitmap(bitmap)

            Sets a bitmap to use as a fill texture. Sets :attr:`filltexhref` to an empty
            string.

    .. autoclass:: LineNode([pos1, pos2, texcoord1, texcoord2])

        A line. :py:attr:`pos1` and :py:attr:`pos2` are the two endpoints of the line.

        .. py:attribute:: pos1

        .. py:attribute:: pos2

        .. py:attribute:: texcoord1

        .. py:attribute:: texcoord2

            floats that signify the u axis texture coordinates.
    
    .. autoclass:: MeshNode([vertexcoords, texcoords, triangles])

        This is a generalized mesh of textured triangles. See 
        https://www.libavg.de/wiki/ProgrammersGuide/MeshNode for an example.

        .. py:attribute:: texcoords

        .. py:attribute:: triangles

        .. py:attribute:: vertexcoords

    .. autoclass:: PolygonNode([linejoin="bevel", pos, texcoords])

        A closed figure bounded by a number of line segments, optionally filled. Filled
        polygons may not be self-intersecting.

        .. py:attribute:: linejoin

            The method by which line segments are joined together. Valid values are 
            :py:const:`bevel` and :py:const:`miter`.

        .. py:attribute:: pos

            A sequence (:py:class:`list` or :py:class:`tuple`) of pixel positions.

        .. py:attribute:: texcoords

            A sequence of float texture coordinates along the u axis of the texture,
            used for the polygon border. It can contain either one coordinate per entry
            in pos (plus an additional one to close the polygon) or two coordinates that
            are interpolated to wrap around the :py:class:`PolygonNode` once.
            
    .. autoclass:: PolyLineNode([linejoin="bevel", pos, texcoords])

        A figure similar to a :py:class:`PolygonNode`, but not closed and never filled. 

        .. py:attribute:: linejoin

            The method by which line segments are joined together. Valid values are 
            :py:const:`bevel` and :py:const:`miter`.

        .. py:attribute:: pos

            A sequence (:py:class:`list` or :py:class:`tuple`) of pixel positions.

        .. py:attribute:: texcoords

            A sequence of float texture coordinates along the u axis of the texture.
            It can contain either one coordinate per entry in pos or two coordinates,
            one for either end of the :py:class:`PolyLine`.
            
    .. autoclass:: RectNode([pos, size, angle])

        A rectangle that can be filled.

        .. py:attribute:: angle

            The angle that the rectangle is rotated to in radians. 0 is
            unchanged, 3.14 is upside-down. The rectangle is rotated around its
            center.

        .. py:attribute:: pos

            The position of the top left corner of the rectangle.

        .. py:attribute:: size

        .. py:attribute:: texcoords

            A sequence of :py:const:`5` texture coordinates for the border of the
            rectangle, which wrap around the node.

    .. autoclass:: VectorNode([color="FFFFFF", strokewidth=1, texhref, blendmode="blend"])

        Base class for all nodes that draw geometrical primitives. All vector nodes 
        support configurable stroke width. Strokes can be filled either with a solid 
        color (:py:attr:`color`) or with a texture loaded from a file 
        (:py:attr:`texhref`) or taken from a bitmap object (:py:meth:`setBitmap`).

        When using textured :py:class:`VectorNodes`, texture coordinates can be set. 
        Generally, texture bitmaps are mapped to coordinates 
        :py:const:`(u,v) = (0,0)-(1,1)`, and the texture is repeated for coordinates 
        outside this range. The :py:const:`u` texture coordinate increases along the
        stroke, while the :py:const:`v` coordinate increases perpendicular to it.

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

.. automodule:: libavg.geom
    :no-members:

    .. autoclass:: Arc(radius, startangle, endangle[, pos=(0,0)])

        An unfilled arc (incomplete circle) from :py:attr:`startangle` to 
        :py:attr:`endangle`. :py:attr:`pos` is the center of the circle.

        .. py:attribute:: endangle

        .. py:attribute:: pos

        .. py:attribute:: radius

        .. py:attribute:: startangle


    .. autoclass:: PieSlice(radius, startangle, endangle[, pos=(0,0)])

        An arc (incomplete circle) from :py:attr:`startangle` to 
        :py:attr:`endangle` connected to the center of the circle. :py:attr:`pos` is the
        center of the circle. A :py:class:`PieSlice` can be filled.

        .. py:attribute:: endangle

        .. py:attribute:: pos

        .. py:attribute:: radius

        .. py:attribute:: startangle


    .. autoclass:: RoundedRect(size, radius, pos)

        A rectangle with rounded corners. :py:attr:`radius` is the corner radius.

        .. py:attribute:: pos

        .. py:attribute:: radius

        .. py:attribute:: size

