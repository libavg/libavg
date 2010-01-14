import avg

def createWrapper(nodeName, nodeType):
    class NodeWrapper(nodeType):
        def __new__(cls, **attributes):
            node = avg.Player.get().createNode(nodeName, attributes)
            return node
    NodeWrapper.__name__ = nodeType.__name__
    NodeWrapper.__doc__ = nodeType.__doc__
    return NodeWrapper

for attrName in dir(avg):
    exportedClass = getattr(avg, attrName)
    if isinstance(exportedClass, type) and issubclass(exportedClass, avg.Node):
        if exportedClass != avg.Node:
            nodeName = exportedClass.__name__.replace("Node","").lower()
            wrapper = createWrapper(nodeName, exportedClass)
            setattr(avg, attrName, wrapper)

