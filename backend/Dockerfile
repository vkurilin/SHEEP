FROM sheep_base as sheep

ENV LD_LIBRARY_PATH "${LD_LIBRARY_PATH}:/PALISADE/bin/lib:/PALISADE/third-party/lib"
ENV LIBRARY_PATH "${LIBRARY_PATH}:/PALISADE/bin/lib"

ENV CMAKE_LIBRARY_PATH "/usr/local/lib:/PALISADE/bin/lib/:/PALISADE/third-party/lib"
ENV CMAKE_INCLUDE_PATH "/PALISADE/src/pke/lib/:/PALISADE/src/:/PALISADE/src/core/lib/:/PALISADE/third-party/include/"

ENV CPLUS_INCLUDE_PATH "/PALISADE/src/pke/lib/:/PALISADE/src/:/PALISADE/src/core/lib/:/PALISADE/third-party/include/"


ADD applications SHEEP/backend/applications
ADD cmake SHEEP/backend/cmake
ADD CMakeLists.txt SHEEP/backend/CMakeLists.txt
ADD include SHEEP/backend/include
ADD src SHEEP/backend/src
ADD tests SHEEP/backend/tests

RUN rm -fr SHEEP/backend/build; mkdir SHEEP/backend/build

RUN cd SHEEP/backend/build; export CC=gcc; export CXX=g++; \
    export HElib_ROOT=/SHEEP/backend/lib/HElib/src; \
    export SEAL_INCLUDE_DIR=/usr/local/include/SEAL-3.6/; \
    cmake ..

#RUN cd SHEEP/backend/build; make VERBOSE=1 run-sheep-server

WORKDIR SHEEP/backend/build
EXPOSE 34568
CMD ["bin/run-sheep-server"]

FROM sheep as sheep-remote-cpp

RUN dnf install -y \
    openssh-server \
    passwd \
    gdb \
    clang \
    libasan \
    libtsan \
    rsync \
    tar \
    python \
    glibc-static \
    libstdc++-static \
    && dnf clean all

RUN ssh-keygen -A

RUN ( \
    echo 'LogLevel DEBUG2'; \
    echo 'PermitRootLogin yes'; \
    echo 'PasswordAuthentication yes'; \
    echo 'Subsystem sftp /usr/libexec/openssh/sftp-server'; \
  ) > /etc/ssh/sshd_config_test_clion

RUN useradd -m user \
  && yes password | passwd user

CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config_test_clion"]