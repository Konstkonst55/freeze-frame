DROP TABLE IF EXISTS assignments, assets, employees CASCADE;

CREATE TABLE employees (
    id        BIGSERIAL PRIMARY KEY,
    full_name TEXT NOT NULL,
    email     TEXT NOT NULL UNIQUE,
    active    BOOLEAN NOT NULL DEFAULT TRUE
);

CREATE TABLE assets (
    id               BIGSERIAL PRIMARY KEY,
    inventory_number TEXT NOT NULL UNIQUE,
    model            TEXT NOT NULL,
    category         TEXT NOT NULL,
    assigned         BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE assignments (
    id          BIGSERIAL PRIMARY KEY,
    asset_id    BIGINT NOT NULL REFERENCES assets(id),
    employee_id BIGINT NOT NULL REFERENCES employees(id),
    issued_at   BIGINT NOT NULL,
    returned_at BIGINT NULL
);

CREATE UNIQUE INDEX uq_active_asset
    ON assignments(asset_id) WHERE returned_at IS NULL;