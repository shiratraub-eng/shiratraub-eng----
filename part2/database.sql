CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE
);

CREATE TABLE messages (
    sender TEXT,
    receiver TEXT,
    message TEXT,
    timestamp DATETIME
);