if ! echo ${PATH} |grep -q /opt/ec2-api-tools/bin ; then
PATH="$PATH:/opt/ec2-api-tools/bin"
fi
export EC2_HOME=/opt/ec2-api-tools
[ $(env | grep -i JAVA_HOME) ] || export JAVA_HOME=/usr/java/default
export AWS_ACCESS_KEY=AKIAI52ERUQP3UL5K2QA
export AWS_SECRET_KEY=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