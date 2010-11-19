Nodes
=====

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: AreaNode CameraNode CanvasNode CircleNode CurveNode DivNode FilledVectorNode ImageNode LineNode MeshNode Node PanoImageNode PolygonNode PolyLineNode RasterNode RectNode SoundNode VectorNode VideoNode VisibleNode WordsNode
        :parts: 1

    .. autoclass:: AVGNode

        .. py:method:: __init__(onkeydown: string, onkeyup: string)
        
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
    .. autoclass:: CameraNode
    .. autoclass:: CanvasNode
    .. autoclass:: CircleNode
    .. autoclass:: CurveNode
    .. autoclass:: DivNode
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

