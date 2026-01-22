CREATE TABLE users (
    id INTEGER PRIMARY KEY,
    username TEXT UNIQUE
);

CREATE TABLE messages (
    sender TEXT,
    receiver TEXT,   -- אם NULL, הודעה ציבורית
    message TEXT,
    timestamp DATETIME

);
