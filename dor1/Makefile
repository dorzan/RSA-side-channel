SUBDIRS=src demo

all: clean alla
alla:
	for f in ${SUBDIRS}; do   ${MAKE} -C $$f || break ;  done

clean: cleansubdirs localclean

cleansubdirs:
	for f in ${SUBDIRS}; do   ${MAKE} -C $$f clean;  done

localclean:

distclean: localclean
	for f in ${SUBDIRS}; do   ${MAKE} -C $$f distclean;  done
	rm Makefile
	rm config.status config.log

maintainerclean: versionclean
	rm configure

versionclean: distclean
	rm -rf autom4te.cache



