targetname="main"

rm build/* -rf
cd build
cmake ..
make
./${targetname}
