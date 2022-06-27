#!/bin/sh

mkdir choreonoid-1.8.0
git archive v1.8.0 | tar -x -C choreonoid-1.8.0
rm choreonoid-1.8.0/.??*
zip -q choreonoid-1.8.0.zip -r choreonoid-1.8.0
rm -fr choreonoid-1.8.0
