
PYPATH="`which python3`"
PYPATH="`dirname $PYPATH`"
PYPATH="`dirname $PYPATH`"

opt=
if [[ `uname` =~ Darwin ]]; then
	opt="-framework GLUT -framework OpenGL $opt"
fi

cd `dirname $0`/CPP && g++ -fPIC -std=c++11 -Wno-c++11-compat-deprecated-writable-strings -Wno-c++11-narrowing -Wno-writable-strings -Wno-write-strings -Wno-narrowing -masm=intel -mavx2 $opt \
	-g -shared -o ../tempovis.so Visualization.cpp TempoEst.cpp TempoVis.cpp vect_sse.cpp

