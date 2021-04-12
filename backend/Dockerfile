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

RUN cd SHEEP/backend/build; export CC=gcc-9; export CXX=g++-9; \
    HElib_ROOT=/SHEEP/backend/lib/HElib/src; \
    SEAL_INCLUDE_DIR=/usr/local/include/SEAL-3.6/seal; \
    PALISADE_INCLUDE_DIR=/PALISADE/src/pke/lib/; \
    PALISADE_LIB_DIR=/PALISADE/bin/lib/; \
    PALISADE_THIRD_PARTY_LIB_DIR=/PALISADE/third-party/lib/ \
    SEAL_INCLUDE_DIR=/usr/local/include/SEAL-3.6/seal; cmake ../

RUN cd SHEEP/backend/build; make VERBOSE=1 run-sheep-server

WORKDIR SHEEP/backend/build
EXPOSE 34568
CMD ["bin/run-sheep-server"]

FROM sheep as sheep-remote-cpp

RUN apt-get update \
  && apt-get install -y ssh \
      gdb \
      clang \
      rsync \
      tar \
      python \
  && apt-get clean

RUN ( \
    echo 'LogLevel DEBUG2'; \
    echo 'PermitRootLogin yes'; \
    echo 'PasswordAuthentication yes'; \
    echo 'Subsystem sftp /usr/lib/openssh/sftp-server'; \
  ) > /etc/ssh/sshd_config_test_clion \
  && mkdir /run/sshd

RUN useradd -m user \
  && yes password | passwd user

RUN usermod -s /bin/bash user
CMD ["/usr/sbin/sshd", "-D", "-e", "-f", "/etc/ssh/sshd_config_test_clion"]