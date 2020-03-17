targetname="server"

rm build/* -rf
cd build
cmake ..
make
./${targetname}
