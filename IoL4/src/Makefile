MAKE=gmake
RM=rm

all: clean config
	$(MAKE) -C pistachio-0.2/kernel-build
	$(MAKE) -C pistachio-0.2/user
	$(MAKE) -C libCLight/
	$(MAKE) -C IoVM-2003-08-19/IoVM
	cp IoVM-2003-08-19/IoVM/io ../bin
	cp pistachio-0.2/user/serv/sigma0/sigma0 ../bin
	cp pistachio-0.2/user/util/kickstart/kickstart ../bin
	cp pistachio-0.2/kernel-build/ia32-kernel ../bin

config:
	$(MAKE) -C pistachio-0.2/kernel BUILDDIR=`pwd`/pistachio-0.2/kernel-build
	$(MAKE) -C pistachio-0.2/kernel-build menuconfig
	cd pistachio-0.2/user; ./configure

clean:
	$(MAKE) -C pistachio-0.2/user clean
	$(MAKE) -C libCLight/ clean
	$(MAKE) -C IoVM-2003-08-19/IoVM clean
	$(RM) -Rf pistachio-0.2/kernel-build
	$(RM) -Rf ../bin/*

