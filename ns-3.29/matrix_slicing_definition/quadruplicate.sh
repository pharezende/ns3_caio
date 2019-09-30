#!/bin/bash
awk '{print $0}1' $1 > uplinkMatrix20MHz_aux
awk '{print $0}1' uplinkMatrix20MHz_aux > uplinkMatrix20MHz_aux_aux
mv uplinkMatrix20MHz_aux_aux $2
rm uplinkMatrix20MHz_aux
