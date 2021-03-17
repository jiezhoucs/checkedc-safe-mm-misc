# This Makefile is a debugging helper. It facilitates compiling and running
# Olden benchmarks.

OLDEN = ./olden.sh

olden:
	$(OLDEN)

bh:
	$(OLDEN) bh

bisort:
	$(OLDEN) bisort

em3d:
	$(OLDEN) em3d

health:
	$(OLDEN) health

mst:
	$(OLDEN) mst

perimeter:
	$(OLDEN) $@

power:
	$(OLDEN) power

treeadd:
	$(OLDEN) $@

tsp:
	$(OLDEN) tsp

clean:
	./olden.sh clean
	./olden-origin.sh clean
