#!/bin/bash


function _(){
    PORTS=("")
    PORTAMNT=0
    echo -n "How many $1s you wish to add"
    echo ""
    read portamnt
    if ! [[ "$portamnt" =~ ^[0-9]+$ ]] ; then
        "I needed a number..."
        quit 1
    fi
    for (( i=1; i <= $portamnt; i++ ))
    do
        let PORTAMNT=PORTAMNT+1
        echo "give $1$PORTAMNT"
        read newport
        PORTS[$i-1]="$newport"
    done
    if [ $PORTAMNT = 0 ]; then
        echo "skipping $1 config"
    else
        echo "configuring $1s: ${PORTS[@]}"

        for xi in ${!PORTS[*]} 
        do
            echo $2${PORTS[$xi]} >> $HOME/.nibbles/default.conf
        done
    fi
}

function :(){
    echo -n "$1 (y/n)"
    read yn
    if [ "y" == $yn ] || [ "Y" == $yn ]
    then
        $2
    else
        $3
    fi
}
#function ap(){
#    echo -n "give the  F$1 testport"
#    echo ""
#    read p
#    echo "f$2port=$p" >> $HOME/.nibbles/default.conf
#}
#function ip(){
#    echo -n "give the F$1 ip address in dotted decimal notation"
#    echo ""
#    read ip
#    echo "f$2ip=$ip" >> $HOME/.nibbles/default.conf
#}
#function add_fspp(){
#    ap SP sp
#}
#function add_fcmp(){
#    ap CM cm
#}
#function add_fspip(){
#    ip SP sp
#}

#function add_fcmip(){
#    ip CM cm
#}
function add_udplog(){
    echo -n "give name for default UDP log"
    echo ""
    read logname
    echo "udplog=$logname" >> $HOME/.nibbles/default.conf
}
function add_includes(){
    _ "include filter" "filter=+"
}

function add_excludes(){
    _ "exclude filter" "filter=-"
}
function add_hls(){
    _ "hl filter" "filter=!"
}

function do_defcfg_file(){
    echo "moving old config file to $HOME/.nibbles/default.conf.old"
    mv "$HOME/.nibbles/default.conf" "$HOME/.nibbles/default.conf.old"
    _ "listen port" "port="
    : "add default exclude filters" add_excludes ""
    : "add default highlight filters" add_hls ""
    : "add default include filters" add_includes ""
    : "set default udp log name" add_udplog ""
    echo "$HOME/.nibbles/default.conf written!"
}


function quit(){
    echo DONE
    if [ $1 ]
    then
        if [ $1 != "" ]
        then
            if [ $1 -ne 0 ]
            then
                echo "FAILURE!"
            fi
        else
            exit 0
        fi
    else
        exit 0
    fi
    exit $1
}

if [ `id -u` -ne 0 ] ; then
    echo "Non root account - doing make + configs but skipping install. You should type 'make install' or run this script as root to install binaries/manuals in system folders"
    echo "nibble some buttons to continue"
    imroot=0
else
    echo "root account - skipping make + configs but doing install. Assuming you have already built the exes"
    imroot=1
fi
read -s foo

if [ $imroot -eq 0 ]; then
    make
    if [ $? -ne 0 ]
    then
        echo "make failed! (compilation error? missinge ncurses development package?)"
    : "continue to configuration" "" "quit"
#    quit 1
    fi
    : "Do you wish to create folder for configs in $HOME/.nibbles" "mkdir $HOME/.nibbles" "quit"
    : "Copy shitemanager file containing known messages for nibbles to use" "cp ext/msgetemplates $HOME/.nibbles/msgtemplates" ""
    : "Do you wish to create default startup config file?" do_defcfg_file ""
    echo "Please copy I_Interface with all headers for definition parsing to ~/.nibbles/"

else

    make install
    if [ $? -ne 0 ]
    then
        echo "install fail (permission denied?) maybe root access required to write /usr/bin"
        echo "you should run make install as root manually, if you wish nibbles to be usable system wide"
    fi
fi

quit 0

