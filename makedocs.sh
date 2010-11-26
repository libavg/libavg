#!/bin/bash

cd sphinxdoc
#sphinx-build -b linkcheck . _build/ 
sphinx-build -b coverage . _build/
sphinx-build -b html . _build/reference
