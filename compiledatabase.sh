clang -fuse-ld=lld -MJ hello.json -o hello helloworld.c
sed -e '1s/^/[\n/' -e '$s/,$/\n]/' *.json
