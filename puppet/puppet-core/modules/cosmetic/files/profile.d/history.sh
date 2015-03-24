##########################################################################
# This file is centrally managed, any manual changes will be OVERWRITTEN #
##########################################################################

shopt -s histappend
HISTFILE=~/.bash_history
HISTSIZE=55000
HISTFILESIZE=999999
HISTIGNORE="ignoredups"
HISTCONTROL=""
HISTTIMEFORMAT="%F %T "
readonly HISTFILE
readonly HISTSIZE
readonly HISTFILESIZE
readonly HISTIGNORE
readonly HISTCONTROL
readonly HISTTIMEFORMAT
export HISTFILE HISTSIZE HISTFILESIZE HISTIGNORE HISTCONTROL HISTTIMEFORMAT 
