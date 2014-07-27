#!/bin/bash
#  Script to perform various operations on GPIO pins using sysfs
#  Run this as root in /sys/class/gpio

# Provided as-is for any purpose
#  Fred@Adapteva.com

# Usage:
#  gpio OP [N1 [N2]]
#    OP is one of:
#       ls     - list direction & value
#       exp    - export pin(s)
#       unex   - unexport pin(s)
#       in|out - set direction
#       0|1    - set value
#       info   - get kernel info about gpios
#
#    N1 is starting GPIO #
#    N2 is ending GPIO # (N1 and N2 are inclusive)
#    if N2 is omitted then only N1 is used
#    if N1 is omitted then all currently exported GPIOs are used
#    if no arguments are supplied then OP is taken as ls

# Note:
#   Attempting to set a value to an input pin results in a write error.

gpdir="/sys/class/gpio"

if [ -$2 != - -a -$3 != - ] ; then
   for n in `seq $2 $3` ; do
       list+="$gpdir/gpio$n "
   done
elif [ -$2 != - ] ; then
   list=$gpdir/gpio$2
else
   list=$gpdir/gpio*
fi

if [[ -$1 == -inf* ]] ; then
    cat /sys/kernel/debug/gpio
    exit
fi

#echo Using list: $list

for file in $list ; do

    if [[ $file == *gpiochip* ]] ; then
       continue
    fi

    if [[ -$1 != -ex* ]] ; then
       if [ ! -d $file ] ; then
          echo "$file: not present"
          continue
       fi
    fi

    num=${file##*gpio}

    case "-$1" in
       -|-ls)
          echo "$file: `cat $file/direction` `cat $file/value`"
          ;;
       -in|-out)
          echo $1 > $file/direction
          ;;
       -0|-1)
          echo $1 > $file/value
          ;;
       -ex*)
          echo "Exporting $num"
          echo $num > $gpdir/export
          ;;
       -unex*)
	  echo "Unexporting $num"
          echo $num > $gpdir/unexport
          ;;
       *)
          echo "Unrecognized operation $1"
	  exit

    esac

done
