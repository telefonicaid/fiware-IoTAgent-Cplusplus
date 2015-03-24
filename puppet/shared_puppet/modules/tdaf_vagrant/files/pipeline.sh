if ! echo ${PATH} |grep -q /opt/pdi/tdaf/dp ; then
PATH="$PATH:/opt/pdi/tdaf/dp"
fi
export DP_HOME=/opt/pdi/tdaf/dp