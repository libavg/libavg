//
// $Id$
// 

%module avg
%{
#include "../player/Words.h"
%}

%include "RasterNode.i"

%attribute(avg::Words, const std::string&, font, getFont, setFont);
%attribute(avg::Words, const std::string&, text, getText, setText);
%attribute(avg::Words, const std::string&, color, getColor, setColor);
%attribute(avg::Words, double, size, getSize, setSize);
%attribute(avg::Words, int, parawidth, getParaWidth, setParaWidth);
%attribute(avg::Words, int, indent, getIndent, setIndent);
%attribute(avg::Words, double, linespacing, getLineSpacing, setLineSpacing);
%attribute(avg::Words, const std::string&, alignment, getAlignment, setAlignment);
%attribute(avg::Words, const std::string&, weight, getWeight, setWeight);
%attribute(avg::Words, bool, italic, getItalic, setItalic);
%attribute(avg::Words, const std::string&, stretch, getStretch, setStretch);
%attribute(avg::Words, bool, smallcaps, getSmallCaps, setSmallCaps);

namespace avg {

class Words : public RasterNode
{
    public:
        Words ();
        virtual ~Words ();
};

}
