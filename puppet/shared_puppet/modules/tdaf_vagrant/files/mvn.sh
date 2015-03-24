if ! echo ${PATH} |grep -q /opt/apache-maven-3.0.5/bin/ ; then
PATH="$PATH:/opt/apache-maven-3.0.5/bin/"
fi
if ! echo ${PATH} |grep -q /usr/java/default/bin/ ; then
PATH="$PATH:/usr/java/default/bin/"
fi
export M2_HOME=/opt/apache-maven-3.0.5/
[ $(env | grep -i JAVA_HOME) ] || export JAVA_HOME=/usr/java/default