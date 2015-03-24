# if running bash
if [ -n "$BASH_VERSION" ]; then
    # include .bashrc if it exists
    if [ -f "$HOME/.bashrc" ]; then
   . "$HOME/.bashrc"
    fi
fi
#echo "Welcome to:"
#cat .motd
. ./.git-prompt.sh
. ./.git-completion.bash
[[ -f ~/bin/setEnv.sh ]] && source ~/bin/setEnv.sh
[ "$distribution" == "Debian" ] && PS1='${debian_chroot:+($debian_chroot)}\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[01;31m\]$(__git_ps1 " (%s)")\[\033[00m\]\$ ' || PS1='\[\033[01;32m\]\u@\h\[\033[00m\]:\[\033[01;34m\]\w\[\033[01;31m\]$(__git_ps1 " (%s)")\[\033[00m\]\$ '


