# Thanks to Dru Nelson for help with this makefile

FOLDERS = IoVM IoServer IoDesktop

all:
	-for i in $(FOLDERS); do \
		if test -d $$i; then (cd $$i; $(MAKE)); fi \
		done

install:
	-for i in $(FOLDERS); do \
		if test -d $$i; then (cd $$i; $(MAKE) install); fi \
		done

clean:
	-for i in $(FOLDERS); do \
		if test -d $$i; then \
			(echo cleaning subdirectory $$i; cd $$i; \
			if test -f Makefile; \
			then $(MAKE) clean; \
			fi); \
		else true; fi; \
		done
	if test -e Projects/OSX/build; then rm -rf Projects/OSX/build; fi;
	if test -e Projects/OSXBundle/build; then rm -rf Projects/OSXBundle/build; fi;

configure:
	-for i in $(FOLDERS); do \
		if test -d $$i; then (cd $$i; $(MAKE) configure ); fi \
		done