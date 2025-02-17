FROM ubuntu:16.04

RUN mkdir -p /app/logs && \
    apt-get update && \
    apt-get install -y \
        git \
        g++ \
        python \
        qt5-default \
        python-pygraphviz \
        python-kiwi \
        python-pygoocanvas \
        libgoocanvas-dev \
        ipython \
        openmpi-bin \
        openmpi-common \
        openmpi-doc \
        libopenmpi-dev \
        mercurial \
        unzip \
        gdb \
        valgrind \
        clang-format \
        doxygen \
        graphviz \
        imagemagick \
        texlive \
        texlive-extra-utils \
        texlive-latex-extra \
        texlive-font-utils \
        dvipng \
        latexmk \
        python3-sphinx \
        dia \
        gsl-bin \
        libgsl-dev \
        tcpdump \
        sqlite \
        sqlite3 \
        libsqlite3-dev \
        libxml2 \
        libxml2-dev \
        cmake \
        libc6-dev \
        libc6-dev-i386 \
        libclang-dev \
        llvm-dev \
        automake \
        wget \
        vim \
        python3-pip

RUN python3 -m pip install --user cxxfilt

WORKDIR /usr/ns3

RUN wget http://www.nsnam.org/release/ns-allinone-3.26.tar.bz2 && \
    git clone https://github.com/a3794110/ns-3-NB-IoT.git && \
    tar xjf ns-allinone-3.26.tar.bz2

WORKDIR /usr/ns3/ns-allinone-3.26/ns-3.26

RUN rm -rf src/lte/model && \
    cp -r /usr/ns3/ns-3-NB-IoT/model src/lte/ && \
    ./waf configure --build-profile=debug --enable-examples --enable-tests && \
    ./waf build

COPY sim/* scratch/
COPY sumo_outputs/boa_vista/ns3.tcl scratch/ns3.tcl

RUN ./waf

ENTRYPOINT ["./waf"]
CMD ["--help"]
