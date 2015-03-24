#!/bin/bash

################################################################################
#                                                                              #
#  Copyright (C) 2012 Jack-Benny Persson <jack-benny@cyberinfo.se>             #
#                                                                              #
#   This program is free software; you can redistribute it and/or modify       #
#   it under the terms of the GNU General Public License as published by       #
#   the Free Software Foundation; either version 2 of the License, or          #
#   (at your option) any later version.                                        #
#                                                                              #
#   This program is distributed in the hope that it will be useful,            #
#   but WITHOUT ANY WARRANTY; without even the implied warranty of             #
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              #
#   GNU General Public License for more details.                               #
#                                                                              #
#   You should have received a copy of the GNU General Public License          #
#   along with this program; if not, write to the Free Software                #
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  #
#                                                                              #
################################################################################

###############################################################################
#                                                                             # 
# Nagios plugin to monitor a single files MD5 sum. In case of mismatch        #
# the plugin exit with a CRICITAL error code. This behavior can be changed    #
# with the --warning argument.                                                # 
# Written in Bash (and uses sed & awk).                                       #
#                                                                             #
###############################################################################


VERSION="1.1"
AUTHOR="(c) 2012 Jack-Benny Persson (jack-benny@cyberinfo.se)"

# Exit codes
STATE_OK=0
STATE_WARNING=1
STATE_CRITICAL=2
STATE_UNKNOWN=3

shopt -s extglob

#### Functions ####

# Print version information
print_version()
{
	printf "\n\n$0 - $VERSION\n"
}

#Print help information
print_help()
{
	print_version
	printf "$AUTHOR\n"
	printf "Monitor the MD5 checksum of a single file\n"
/bin/cat <<EOT

Options:
-h
   Print detailed help screen
-V
   Print version information

--warning
   Issue a warning state instead of a critical state in case of a MD5 failure
   Default is critical

--file /path/to/file
   Set which file to monitor
 
--md5 md5checksum
   Set the MD5 checksum for the file set by --file

EOT
}


# Parse command line options
while [[ -n "$1" ]]; do 
   case "$1" in

       -h | --help)
           print_help
           exit $STATE_OK
           ;;

       -V | --version)
           print_version
           exit $STATE_OK
           ;;

       -\?)
	   print_help
           exit $STATE_OK
           ;;

       --warning)
           warning="yes"
	   shift 1
	   ;;

       --file)
	   if [[ -z "$2" ]]; then
		printf "\nOption $1 requires an argument\n"
		print_help
		exit $STATE_UNKNOWN
	   fi
		file=$2
           shift 2
           ;;

       --md5)
           if [[ -z "$2" ]]; then
                printf "\nOption $1 requires an argument\n"
		print_help
                exit $STATE_UNKNOWN
           fi
                md5=$2
           shift 2
           ;;

	*)
           printf "\nInvalid option $1"
           print_help
           exit $STATE_UNKNOWN
           ;;


   esac
done

### Check if we provided a file and a MD5 sum ###

if [[ -z "$file" ]]; then
	# No file specified
	printf "\nNo file specified"
	print_help
	exit $STATE_UNKNOWN
fi

if [[ -z "$md5" ]]; then
	# No MD5 sum specified
	printf "\nNo MD5 sum specified"
	print_help
	exit $STATE_UNKNOWN
fi


### MAIN ###

#Get the current checksum of the file
filesum=`md5sum ${file} | awk '{print $1}'`

#Compare the MD5 on the file against the sum we provided
if [[ "$filesum" == "$md5" ]]; then
	printf "$file - MD5 OK\n" 
	exit $STATE_OK

#See if we wanted a warning instead of a critical
elif [[ "$warning" == "yes" ]]; then
		printf "$file - MD5 WARNING\n"
		exit $STATE_WARNING
#Critical
else	

  printf "$file - MD5 CRITICAL\n"
  exit $STATE_CRITICAL
fi

exit 3
