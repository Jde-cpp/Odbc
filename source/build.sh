#!/bin/bash
debug=${1:-1}
clean=${2:-0}

cd "${0%/*}"
echo "Build odbc debug=$debug clean=$clean"
if [ $clean -eq 1 ]; then
	make clean DEBUG=$debug
	if [ $debug -eq 1 ]
	then
		ccache g++-8 -c -g -pthread -fPIC -std=c++17 -Wall -Wno-unknown-pragmas -DJDE_ODBC_EXPORTS -O0 -I.obj/debug pc.h -o.obj/debug/stdafx.h.gch -I/home/duffyj/code/libraries/spdlog/include -I/usr/include/iodbc  -I/home/duffyj/code/libraries/boostorg/boost_1_68_0
	else
		ccache g++-8 -c -g -pthread -fPIC -std=c++17 -Wall -Wno-unknown-pragmas -DJDE_ODBC_EXPORTS -march=native -DNDEBUG -O3 -I.obj/release pc.h -o.obj/release/stdafx.h.gch -I/home/duffyj/code/libraries/spdlog/include -I/home/duffyj/code/libraries/mysql/include/mysqlx -I/usr/include/iodbc  -I/home/duffyj/code/libraries/boostorg/boost_1_68_0
	fi
	if [ $? -eq 1 ]; then
		exit 1
	fi
	echo "built pc.h"
fi

make -j7 DEBUG=$debug
cd -
exit $?