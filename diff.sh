#!/bin/bash
rm /tmp/*.m
wolframscript -code nbToM.sh "$1" /tmp/source.m 2>/dev/null
wolframscript -code nbToM.sh "$(pwd)/$2" /tmp/target.m 2>/dev/null
kdiff3 /tmp/source.m /tmp/target.m
