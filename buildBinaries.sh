
BIN=./bin
SRC=./src
SRCs="$SRC/MBI.cpp $SRC/Liberty.cpp $SRC/Topology.cpp $SRC/Geometry.cpp $SRC/InverterTree.cpp"

for NET_ORDER in 0 1 2
do
for CRIT_ALG in 0 1 2
do
for CRIT_THRESHOLD in 0.9
do
for NON_CRIT_ALG in 0 1
do
for INV_POS in 0 1
do
g++ -DNET_ORDER=$NET_ORDER -DCRIT_ALG=$CRIT_ALG -DCRITICAL_THRESHOLD=$CRIT_THRESHOLD -DNON_CRIT_ALG=$NON_CRIT_ALG -DINV_POS=$INV_POS $SRCs -o ${BIN}/MBI_${NET_ORDER}_${CRIT_ALG}_${CRIT_THRESHOLD}_${NON_CRIT_ALG}_${INV_POS}
done
done
done
done
done