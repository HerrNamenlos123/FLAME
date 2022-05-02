@echo off

mkdir proto 2>NUL
protoc FLAME.proto --cpp_out=proto
Pause