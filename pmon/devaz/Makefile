
all: bonito.h
	cd ../DEVAZ; $(MAKE)

REGDEFS=/homes/rick/bonito/src/regdefs.x

BONITO_SRC 	= /homes/rick/bonito/src
bonito.h: $(BONITO_SRC)/bonito.h
	@echo "bonito.h is out of date!"
	exit 1

copy:
	cp $(BONITO_SRC)/bonito.h .

#
# Local Variables:
# eval: (setq compile-command "sde3 gmake -k")
# End:
