#!/bin/bash

g++ FlowVectors.cpp -o FlowVectors -L/usr/local/include -lzmq -Wno-nullability-completeness
