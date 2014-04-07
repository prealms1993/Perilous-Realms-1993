#!/bin/sh
#2014 - Quick script to autogenerate all the object files
#Does NOT handle any errors.  Be careful
cd MOBS
../src/tran mob new.mobs ../lib/mob.out 
cd ..
cd OBJ
../src/tran obj new.objs ../lib/obj.out
cd ..
cd SHP
../src/tran shop new.shop ../lib/shop.out
cd ..
cd WORLD1
../src/tran room world ../lib/world.out
cd ..
