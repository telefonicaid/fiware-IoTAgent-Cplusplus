#!/bin/bash


#Print help information
print_help()
{
	print_version
	printf "$AUTHOR\n"
	printf "Monitoriza caducidad de los certificados\n"
/bin/cat <<EOT

Options:
-h
   Imprime ayuda

EOT
}
# Exit codes
STATE_OK=0
STATE_WARNING=1
STATE_CRITICAL=2
STATE_UNKNOWN=3

MESES="Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec"
DIR_CERT="/etc/pki/tls"
CERT="cert.pem"
CHECK_CMD="openssl x509 -noout -enddate -in $DIR_CERT/$CERT " 
#echo $CHECK_CMD
dates=`$CHECK_CMD` 
#echo "=>$dates"


while [[ -n "$1" ]]; do 
   case "$1" in

       -h | --help)
           print_help
           exit $STATE_OK
           ;;
	
	-c)
		if [[ -z "$2" ]]; then
                printf "\nOpcion $1 requiere un argumento\n"
                print_help
                exit $STATE_UNKNOWN
           fi
                critico=$2
           shift 2
           ;;
 
       -w)
           if [[ -z "$2" ]]; then
                printf "\nOpcion $1 requiere un argumento\n"
		print_help
                exit $STATE_UNKNOWN
           fi
                warning=$2
           shift 2
           ;;

	*)
           printf "\n opcion invalida: $1"
           print_help
           exit $STATE_UNKNOWN
           ;;


   esac
done

### MAIN ###
#notBefore=Nov 9 00:00:00 1994 GMT notAfter=Jan 7 23:59:59 2010 GMT
month=`echo $dates|awk -F'=' {'print $2'}|awk '{print $1}'`
day=`echo $dates|awk -F'=' {'print $2'}|awk '{print $2}'`
year=`echo $dates|awk -F'=' {'print $2'}|awk '{print $4}'`

string="${MESES%$month*}"
nummes="$((${#string}/4 + 1))"
actual_month=`date +%m`
actual_year=`date +%Y`

#########################################pruebas
actual_month=6
actual_year=2012
nummes=10
year=2012
#########################################finpruebas
#echo "mes actual: $actual_month"
#echo "mes tope:$nummes"
#echo "año actual:$actual_year"
#echo "año tope:$year"
#echo "#######"

#CRITICO
mescritico=$(($(($actual_month + $critico))))
#echo "mes critico:$mesicritico"
agnocritico=$actual_year
if [ $mescritico -gt 12 ]; then
#	echo "estamos en el año siguiente"
	agnocritico=$((agnocritico+1))
	mescritico=$(($mescritico%12))
#	echo "tope: año $agnocritico mes $mescritico"
fi
#WARNING
meswarning=$(($(($actual_month + $warning))))
#echo "mes warning:$meswarning"
agnowarning=$actual_year
if [ $meswarning -gt 12 ]; then
  #      echo "estamos en el año siguiente"
        agnowarning=$((agnowarning+1))
        meswarning=$(($meswarning%12))
 #       echo "tope: año $agnowarning mes $meswarning"
fi
#si años son distintods, devolvemos OK
diffyear=$(($year - $agnowarning))
diffmonth=$(($nummes - $meswarning ))
#echo "diferencia de mes: $diffmonth"
#echo "diferencia de años $diffyear"

diffyear_critico=$(($year - $agnocritico ))
diffmonth_critico=$(($nummes - $mescritico ))
#echo "diferencia de mes: $diffmonth_critico"
#echo "diferencia de años $diffyear_critico"
if [[ "$diffyear_critico" -ge 1 ]] || [[ "$diffmonth_critico" -ge 1 ]] ; then
        printf "VALIDEZ SSL OK\n"
        exit $STATE_OK
elif [[ $diffmonth_critico -le 0 ]]; then
        printf "VALIDEZ SSL CRITICAL\n"
        exit $STATE_CRITICAL
elif [[ $diffmonth -le 0 ]]; then
        printf "VALIDEZ SSL WARNING\n"
        exit $STATE_WARNING
else 
        printf "VALIDEZ SSL UNKNOWN\n"
        exit $STATE_UNKNOWN

fi
