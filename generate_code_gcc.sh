WKDIR=$PWD

cd $WKDIR/libs/data_desk
echo "Building data_desk..."
./build_gcc.sh
DATA_DESK_EXEC=$PWD/build/data_desk
echo "Building code generation with data_desk..."
gcc -Wall -Wno-write-strings -g -fpic -shared -I source $WKDIR/src/generateSerialization.c -o generateSerialization.so
$DATA_DESK_EXEC --custom ./generateSerialization.so $WKDIR/src/globalTypes.ds
