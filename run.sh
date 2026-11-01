#!/usr/bin/env bash
set -e
export EQDB_NAME=postgres   # user (пользователь)
export EQDB_USER=pass       # password (пароль, который вы установили)
export EQDB_PASS=stopkadr   # dbname (имя базы данных)
export EQDB_HOST=localhost
cd "$(dirname "$0")"
./build/freezeframe
