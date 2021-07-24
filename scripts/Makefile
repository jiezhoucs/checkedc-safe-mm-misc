# This Makefile is a debugging helper. It facilitates compiling and running
# Olden benchmarks.

OLDEN = ./olden.sh
PTRDIST = ./ptrdist.sh

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

# Ptrdist benchmarks
anagram:
	$(PTRDIST) $@

ks:
	$(PTRDIST) $@

clean:
	./olden.sh clean
	./olden-baseline.sh clean
	./ptrdist.sh clean
