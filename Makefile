
SRC=./src
MBI: $(SRC)/MBI.cpp $(SRC)/Liberty.cpp $(SRC)/Topology.cpp $(SRC)/Geometry.cpp $(SRC)/InverterTree.cpp
	g++ $(SRC)/MBI.cpp $(SRC)/Liberty.cpp $(SRC)/Topology.cpp $(SRC)/Geometry.cpp $(SRC)/InverterTree.cpp
clean:
	rm -rf MBI
