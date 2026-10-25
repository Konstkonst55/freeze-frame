# Стопкадр — учёт техники фотостудии

Настольное учебное приложение для учёта фототехники: фотографы, единицы техники
(камеры, объективы, вспышки, штативы, свет), выдача на съёмку и возврат в стойку.

**Стек:** C++17 · ODB (ORM) · PostgreSQL · CMake · FLTK 1.3

## 1. Установка зависимостей

**Debian / Ubuntu**

```bash
sudo apt update
sudo apt install postgresql postgresql-contrib \
    cmake g++ libfltk1.3-dev libpq-dev \
    libodb-dev libodb-pgsql-dev odb
```

**Fedora / RHEL**

```bash
sudo dnf install postgresql-server postgresql-contrib \
    cmake gcc-c++ fltk-devel libpq-devel \
    libodb-devel libodb-pgsql-devel odb
sudo postgresql-setup --initdb
```

**macOS (Homebrew)**

```bash
brew install postgresql@15 cmake fltk libpq odb
brew services start postgresql@15
```

Проверка, что ODB-компилятор на месте:

```bash
odb --version    # должно быть 2.4.0 или новее
```

---

## 2. База данных

Запустить PostgreSQL (если не запущен):

```bash
sudo systemctl start postgresql       # Linux
brew services start postgresql@15     # macOS
```

Cоздать базу:

```bash
sudo -u postgres createdb stopkadr
sudo -u postgres psql -d stopkadr -f sql/schema.sql
```

Дать права вашему пользователю на работу с базой.

---

## 3. Переменные окружения

> [!NOTE]
> Порядок параметров `odb::pgsql::database` в ODB 2.4.0 — `(user, password, dbname, host)`.
> <br>
> Поэтому имена переменных **не совпадают** с тем, что они содержат.

```bash
export EQDB_NAME=ВАШЕ           # → уходит в user
export EQDB_USER=ВАШЕ           # → уходит в password
export EQDB_PASS=stopkadr       # → уходит в dbname
export EQDB_HOST=localhost
```

Добавить в `~/.bashrc`, чтобы не повторять:

```bash
cat >> ~/.bashrc <<'EOF'
export EQDB_NAME=ВАШЕ
export EQDB_USER=ВАШЕ
export EQDB_PASS=stopkadr
export EQDB_HOST=localhost
EOF
source ~/.bashrc
```

---

## 4. Сборка

```bash
rm -rf build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

---

## 5. Запуск

```bash
./build/freezeframe
```
