FROM centos:6.6

MAINTAINER David

ENV GIT_URL_IOTACPP https://github.com/telefonicaid/fiware-IoTAgent-Cplusplus.git
ENV GIT_REV_IOTACPP develop
ENV GIT_PATH_IOTACPP /opt/iotacpp
ENV IOTACPP_BUILD_TYPE Release
ENV IOTACPP_USER iotagent

RUN yum -y install tar gzip unzip file \
                   cpp gcc gcc-c++ \
           automake autoconf libtool \
           git scons cmake \
           openssl-devel bzip2-devel zlib-devel \
           rpm-build && \
    git clone ${GIT_URL_IOTACPP} ${GIT_PATH_IOTACPP} && \
    cd ${GIT_PATH_IOTACPP} && \
    sed -i CMakeLists.txt \
        -e 's|^add_test|#add_test|g' \
    -e 's|^add_subdirectory(tests)|#add_subdirectory(tests)|g' \
    -e 's|^enable_testing|#enable_testing|g' \
        -e 's|git@github.com:mongodb/mongo-cxx-driver.git|https://github.com/mongodb/mongo-cxx-driver|g' && \
    source tools/get_version_string.sh && \
    mkdir -p  build/${IOTACPP_BUILD_TYPE} && \
    cd build/${IOTACPP_BUILD_TYPE} && \
    cmake -DGIT_VERSION=`get_rpm_version_string | cut -d ' ' -f 1` \
          -DGIT_COMMIT=`get_rpm_version_string | cut -d ' ' -f 2` \
      -DMQTT=ON \
      -DCMAKE_BUILD_TYPE=${IOTACPP_BUILD_TYPE} ../../ && \
    make install && \
    make package && \
    cd pack/Linux/RPM && \

    yum clean all && \

    yum -y remove autoconf automake bzip2-devel \
          cmake cpp file gcc gcc-c++ git \
         libtool openssl-devel rpm-build \
         scons zlib-devel bzip2 cloog-ppl \
         elfutils elfutils-libs gdb glibc-devel \
         glibc-headers kernel-headers \
         keyutils-libs-devel krb5-devel \
         libarchive libcom_err-devel libgomp \
         libselinux-devel libsepol-devel \
         libstdc++-devel m4 mpfr patch \
         perl-Error perl-Git perl-Module-Pluggable \
         perl-Pod-Escapes perl-Pod-Simple \
         perl-libs perl-version  \
         ppl rsync && \

    cd ${GIT_PATH_IOTACPP} && \
    cd build/${IOTACPP_BUILD_TYPE} && \
    cd pack/Linux/RPM && \
    curl -kOL http://download.opensuse.org/repositories/home:/oojah:/mqtt/CentOS_CentOS-6/home:oojah:mqtt.repo && \
    mv home:oojah:mqtt.repo /etc/yum.repos.d/ && \
    yum localinstall -y iot-agent-*.rpm && \
    yum clean all && \
    chkconfig iotagent on && \
    chkconfig mosquitto on

COPY config.json /etc/iot/
COPY iotagent_manager.conf /usr/local/iot/config/
COPY docker-entrypoint.sh /

RUN \
    chmod +x ${GIT_PATH_IOTACPP}/scripts/*.sh && \
    chown ${IOTACPP_USER}:${IOTACPP_USER} /etc/iot/config.json && \
    chown ${IOTACPP_USER}:${IOTACPP_USER} /usr/local/iot/config/iotagent_manager.conf && \
    rm -f /usr/local/iot/config/iotagent_protocol.conf && \
    chmod +x /docker-entrypoint.sh && \
    rm -rf ${GIT_PATH_IOTACPP}/.git

ENTRYPOINT ["/docker-entrypoint.sh"]

EXPOSE 8081
EXPOSE 1883




