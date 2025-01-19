CREATE TABLE temperature_logs (
    id SERIAL PRIMARY KEY,
    temperature DOUBLE PRECISION NOT NULL,
    timestamp TIMESTAMP NOT NULL
);

CREATE TABLE average_temperatures (
    id SERIAL PRIMARY KEY,
    type VARCHAR(10) NOT NULL, -- 'hourly' или 'daily'
    average DOUBLE PRECISION NOT NULL,
    timestamp TIMESTAMP NOT NULL
);


-- Для дебага:
-- INSERT INTO average_temperatures (type, average, timestamp) 
-- VALUES ('hourly', 22.5, to_timestamp(1633036800));

-- INSERT INTO average_temperatures (type, average, timestamp) 
-- VALUES ('daily', 21.3, to_timestamp(1633123200));