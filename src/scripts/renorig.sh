#!/bin/sh
dfits *.fits | fitsort -d ORIGFILE | awk '{printf("mv %s %s\n", $1, $2);}' | sh 
