#!/usr/bin/env sh

# cmake .. -DUA_ENABLE_ENCRYPTION=ON -DUA_ENABLE_PUBSUB=ON -DUA_BUILD_EXAMPLES=ON
cmake .. -DUA_ENABLE_ENCRYPTION_OPENSSL=ON -DUA_ENABLE_PUBSUB=ON -DUA_BUILD_EXAMPLES=ON

