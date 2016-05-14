FROM gcc:5.3

RUN apt-get update && apt-get install -y \
	    cmake \
	    clang \
	    libboost-test1.55-dev \
	    libboost-system1.55-dev \
	    libboost-filesystem1.55-dev \
	    libglib2.0-dev \
	    uuid-dev \
	    libjson-c-dev \
	    libsystemd-journal-dev \
	    libsqlite3-dev \
	    libsystemd-daemon-dev \
	&& apt-get clean \
	&& rm -rf /var/lib/apt/lists/*

RUN rm -fr /tmp/build && mkdir -p /tmp/build
COPY utils/build.sh /tmp/build
RUN chmod a+x /tmp/build/build.sh

CMD ["/tmp/build/build.sh"]