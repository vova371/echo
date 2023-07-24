FROM centos:7 AS env

RUN yum update -y
RUN yum install -y epel-release
RUN yum install -y wget bzip2
RUN yum install -y cmake3
RUN yum install -y centos-release-scl
RUN yum install -y devtoolset-11
RUN yum install -y python3-devel

RUN echo "source /opt/rh/devtoolset-11/enable" >> /etc/bashrc
SHELL ["/bin/bash", "--login", "-c"]

RUN alternatives --install /usr/bin/python python /usr/bin/python2 20
RUN alternatives --install /usr/bin/python python /usr/bin/python3 30
RUN alternatives --install /usr/bin/cmake cmake /usr/bin/cmake3 30

# Download, build and install boost
ARG BOOST_VERSION=1.82.0
RUN BOOST_DIRNAME=boost_${BOOST_VERSION//./_} && \
    BOOST_URL=https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/${BOOST_DIRNAME}.tar.bz2 && \
    wget -O - ${BOOST_URL} | tar xjf - && \
    cd ${BOOST_DIRNAME} && \
    ./bootstrap.sh --prefix=/usr/local && \
    ./b2 install && \
    cd .. && \
    rm -fr ${BOOST_DIRNAME}
