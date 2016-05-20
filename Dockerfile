FROM ubuntu:xenial

RUN apt-get update && apt-get install -y \
	    cmake \
	    clang \
	    gcc \
	    libboost-test1.58-dev \
	    libboost-system1.58-dev \
	    libboost-filesystem1.58-dev \
	    libglib2.0-dev \
	    uuid-dev \
	    libsystemd-dev \
	    libjson-c-dev \
	    libsqlite3-dev \
	    gdb \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*
	    # libsystemd-journal-dev \
	    # libsystemd-daemon-dev \

COPY utils/build.sh .
RUN chmod a+x ./build.sh

CMD ["./build.sh"]